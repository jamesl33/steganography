/* This file is a part of "Steganography" a C++ steganography tool.

Copyright (C) 2019 James Lee <jamesl33info@gmail.com>.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "discrete_cosine_transform.hpp"

const std::vector<std::vector<int>> luminance = {{16, 11, 10, 16, 24, 40, 51, 61},
                                                 {12, 12, 14, 19, 26, 58, 60, 55},
                                                 {14, 13, 16, 24, 40, 57, 69, 56},
                                                 {14, 17, 22, 29, 51, 87, 80, 62},
                                                 {18, 22, 37, 56, 68, 109, 103, 77},
                                                 {24, 35, 55, 64, 81, 104, 113, 92},
                                                 {49, 64, 78, 87, 103, 121, 120, 101},
                                                 {72, 92, 95, 98, 112, 100, 103, 99}};

/**
 * Encode the payload into the carrier image using DCT coefficient swapping.
 *
 * @param payload_path The path to the file you are encoding.
 */
void DiscreteCosineTransform::Encode(const boost::filesystem::path& payload_path) {
    // Convert the payload filename to byte vector.
    std::string payload_filename = payload_path.filename().string();
    std::vector<unsigned char> payload_filename_bytes(payload_filename.begin(), payload_filename.end());

    // Encode the length/content of the filename in the carrier image.
    this -> EncodeChunkLength(0, payload_filename_bytes.size());
    this -> EncodeChunk(32, payload_filename_bytes);

    // Read the payload into a byte vector.
    std::vector<unsigned char> payload_bytes = this -> ReadPayload(payload_path);

    // Encode the length/content of the payload in the carrier image.
    this -> EncodeChunkLength(32 + payload_filename_bytes.size() * 8, payload_bytes.size());
    this -> EncodeChunk(64 + payload_filename_bytes.size() * 8, payload_bytes);

    // Save the modified image with the "steg-" prefix.
    boost::filesystem::path steg_image_filename = this -> image_path.filename();
    steg_image_filename.replace_extension(".jpg");

    // TODO(James Lee) - Expose JPEG quality to the user.
    cv::imwrite("steg-" + steg_image_filename.string(), this -> image, std::vector<int>{CV_IMWRITE_JPEG_QUALITY, 100});
}

/**
 * Decode the payload from the carrier image.
 */
void DiscreteCosineTransform::Decode() {
    // Decode the filename byte vector from the carrier image.
    unsigned int payload_filename_length = this -> DecodeChunkLength(0);
    std::vector<unsigned char> payload_filename_bytes = this -> DecodeChunk(32, 32 + payload_filename_length * 8);

    // Convert the payload filename to a string.
    std::string payload_filename(payload_filename_bytes.begin(), payload_filename_bytes.end());

    // Decode the payload byte vector from the carrier image.
    unsigned int payload_length = this -> DecodeChunkLength(32 + payload_filename_length * 8);
    std::vector<unsigned char> payload_bytes = this -> DecodeChunk(64 + payload_filename_length * 8, 64 + payload_filename_length * 8 + payload_length * 8);

    // Save the payload to it's filename with the "steg-" prefix.
    this -> WritePayload("steg-" + payload_filename, payload_bytes);
}

/**
 * Encode a chunk of information into the carrier image.
 *
 * Encode a chunk of information into the carrier image using DCT. The
 * information is hidden in the luminance channel of the YCbCr colorspace by
 * swapping DCT coefficients.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk The chunk of information that will be encoded into the carrier image.
 */
void DiscreteCosineTransform::EncodeChunk(const int& start, const std::vector<unsigned char>& chunk) {
    int bits_written = 0;
    std::queue<unsigned char> chunk_bytes;

    // Copy the payload into a queue.
    for (unsigned char byte : chunk) {
        chunk_bytes.emplace(byte);
    }

    // Convert the image to the YCrCb color space.
    cv::Mat image_ycrcb;
    cv::cvtColor(this -> image, image_ycrcb, cv::COLOR_BGR2YCrCb);

    // Convert the image to floating point.
    cv::Mat image_ycrcb_fp;
    image_ycrcb.convertTo(image_ycrcb_fp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(image_ycrcb_fp, channels);

    for (int row = 0; row < image_ycrcb_fp.rows - 8; row += 8) {
        for (int col = 0; col < image_ycrcb_fp.cols - 8; col += 8) {
            if (row == 0 && col == 0) {
                row = start / (image_ycrcb_fp.cols / 8) * 8;
                col = start % (image_ycrcb_fp.cols / 8) * 8;
            }

            // Stop once we have embedded the whole message.
            if (chunk_bytes.empty()) {
                // Merge the image channels.
                cv::Mat merged_ycrcb_fp;
                cv::merge(channels, merged_ycrcb_fp);

                // Convert the image to unsigned char.
                cv::Mat merged_ycrcb;
                merged_ycrcb_fp.convertTo(merged_ycrcb, CV_8U);

                // Convert the image to the BGR colorspace.
                cv::cvtColor(merged_ycrcb, this -> image, cv::COLOR_YCrCb2BGR);
                return;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform forward dct.
            cv::dct(block, trans);

            // Quantize the 8x8 block while rounding the values.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) = float(int(trans.at<float>(x, y) / (luminance[x][y] / 10)));
                }
            }

            int bit = this -> GetBit(chunk_bytes.front(), bits_written % 8);

            // TODO(James Lee) - Swap multiple coefficients.
            float low = trans.at<float>(2, 0);
            float high = trans.at<float>(0, 2);

            // Swap the dct coefficients to match the data bit.
            if (bit) {
                if (low == high) {
                    low -= 0.5;
                    high += 0.5;
                } else if (low > high) {
                    std::swap(low, high);
                }
            } else {
                if (low == high) {
                    low += 0.5;
                    high -= 0.5;
                } else if (low < high) {
                    std::swap(low, high);
                }
            }

            // TODO(James Lee) - Expose this option to the user.
            // Increase the difference between the "important" coefficients.
            // A larger value will distort the image more, however, it will
            // make the data more robust.
            if (low < high) {
                low -= 5;
                high += 5;
            } else {
                low += 5;
                high -= 5;
            }

            trans.at<float>(2, 0) = low;
            trans.at<float>(0, 2) = high;

            // Dequantize the 8x8 block.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) *= (luminance[x][y] / 10);
                }
            }

            // Perform the inverse dct.
            cv::idct(trans, block);

            // Centre the coefficients around 128.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    block.at<float>(x, y) += 128;
                }
            }

            bits_written++;

            if (bits_written % 8 == 0) {
                // We have encoded a full byte now, pop it from the queue.
                chunk_bytes.pop();
            }
        }
    }
}

/**
 * Encode a 32bit integer stating the length of the following chunk
 *
 * @param start The pixel index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void DiscreteCosineTransform::EncodeChunkLength(const int& start, const unsigned int& chunk_length) {
    int bits_written = 0;

    // Convert the image to the YCrCb color space.
    cv::Mat image_ycrcb;
    cv::cvtColor(this -> image, image_ycrcb, cv::COLOR_BGR2YCrCb);

    // Convert the image to floating point.
    cv::Mat image_ycrcb_fp;
    image_ycrcb.convertTo(image_ycrcb_fp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(image_ycrcb_fp, channels);

    for (int row = 0; row < image_ycrcb_fp.rows - 8; row += 8) {
        for (int col = 0; col < image_ycrcb_fp.cols - 8; col += 8) {
            if (row == 0 && col == 0) {
                row = start / (image_ycrcb_fp.cols / 8) * 8;
                col = start % (image_ycrcb_fp.cols / 8) * 8;
            }

            // We only need to encode a 32bit integer, stop once complete.
            if (bits_written == 32) {
                // Merge the image channels.
                cv::Mat merged_ycrcb_fp;
                cv::merge(channels, merged_ycrcb_fp);

                // Convert the image to unsigned char.
                cv::Mat merged_ycrcb;
                merged_ycrcb_fp.convertTo(merged_ycrcb, CV_8U);

                // Convert the image to the BGR colorspace.
                cv::cvtColor(merged_ycrcb, this -> image, cv::COLOR_YCrCb2BGR);
                return;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform forward dct.
            cv::dct(block, trans);

            // Quantize the 8x8 block while rounding the values.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) = float(int(trans.at<float>(x, y) / (luminance[x][y] / 10)));
                }
            }

            int bit = this -> GetBit(chunk_length, bits_written);

            // TODO(James Lee) - Swap multiple coefficients.
            float low = trans.at<float>(2, 0);
            float high = trans.at<float>(0, 2);

            // Swap the dct coefficients to match the data bit.
            if (bit) {
                if (low == high) {
                    low -= 0.5;
                    high += 0.5;
                } else if (low > high) {
                    std::swap(low, high);
                }
            } else {
                if (low == high) {
                    low += 0.5;
                    high -= 0.5;
                } else if (low < high) {
                    std::swap(low, high);
                }
            }

            // TODO(James Lee) - Expose this option to the user.
            // Increase the difference between the "important" coefficients.
            // A larger value will distort the image more, however, it will
            // make the data more robust.
            if (low < high) {
                low -= 5;
                high += 5;
            } else {
                low += 5;
                high -= 5;
            }

            trans.at<float>(2, 0) = low;
            trans.at<float>(0, 2) = high;

            // Dequantize the 8x8 block.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) *= (luminance[x][y] / 10);
                }
            }

            // Performce the inverse dct.
            cv::idct(trans, block);

            // Center the coefficients around 128.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    block.at<float>(x, y) += 128;
                }
            }

            bits_written++;
        }
    }
}

/**
 * Decode a chunk of information from the carrier image.
 *
 * @param start The pixel index to start decoding at.
 * @param end The pixel index to stop decoding at.
 * @return The chunk of information read from the carrier image.
 */
std::vector<unsigned char> DiscreteCosineTransform::DecodeChunk(const int& start, const int& end) {
    int bits_read = 0;
    std::vector<unsigned char> chunk_bytes = {0};

    // Convert the image to the YCrCb color space.
    cv::Mat image_ycrcb;
    cv::cvtColor(this -> image, image_ycrcb, cv::COLOR_BGR2YCrCb);

    // Convert the image to floating point.
    cv::Mat image_ycrcb_fp;
    image_ycrcb.convertTo(image_ycrcb_fp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(image_ycrcb_fp, channels);

    for (int row = 0; row < image_ycrcb_fp.rows - 8; row += 8) {
        for (int col = 0; col < image_ycrcb_fp.cols - 8; col += 8) {
            if (row == 0 && col == 0) {
                row = start / (image_ycrcb_fp.cols / 8) * 8;
                col = start % (image_ycrcb_fp.cols / 8) * 8;
            }

            if (bits_read == end - start) {
                // We only need to decode a 32bit integer, stop once complete.
                return chunk_bytes;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct.
            cv::dct(block, trans);

            // Round the values in the 8x8 block.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) = float(int(trans.at<float>(x, y)));
                }
            }

            // TODO(James Lee) - Read from multiple swapped coefficients.
            // Decode the data from the swapped coefficients.
            this -> SetBit(&chunk_bytes.back(), bits_read % 8, (trans.at<float>(0, 2) > trans.at<float>(2, 0)));

            bits_read++;

            if (!(bits_read == end - start) && (bits_read % 8 == 0)) {
                // We have decoded a full byte, place an empty once at the back of the chunk_bytes vector.
                chunk_bytes.emplace_back(0);
            }
        }
    }

    return std::vector<unsigned char> {}; // This "should" not be reached
}

/**
 * Decode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start decoding at.
 * @return The length of the next chunk in bytes.
 */
unsigned int DiscreteCosineTransform::DecodeChunkLength(const int& start) {
    int bits_read = 0;
    unsigned int chunk_length = 0;

    // Convert the image to the YCrCb color space.
    cv::Mat image_ycrcb;
    cv::cvtColor(this -> image, image_ycrcb, cv::COLOR_BGR2YCrCb);

    // Convert the image to floating point.
    cv::Mat image_ycrcb_fp;
    image_ycrcb.convertTo(image_ycrcb_fp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(image_ycrcb_fp, channels);

    for (int row = 0; row < image_ycrcb_fp.rows - 8; row += 8) {
        for (int col = 0; col < image_ycrcb_fp.cols - 8; col += 8) {
            if (row == 0 && col == 0) {
                row = start / (image_ycrcb_fp.cols / 8) * 8;
                col = start % (image_ycrcb_fp.cols / 8) * 8;
            }

            if (bits_read == 32) {
                // We only need to decode a 32bit integer, stop once complete.
                return chunk_length;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct.
            cv::dct(block, trans);

            // Round the values in the 8x8 block.
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    trans.at<float>(x, y) = float(int(trans.at<float>(x, y)));
                }
            }

            // TODO(James Lee) - Read from multiple swapped coefficients.
            // Decode the data from the swapped coefficients.
            this -> SetBit(&chunk_length, bits_read, (trans.at<float>(0, 2) > trans.at<float>(2, 0)));

            bits_read++;
        }
    }

    return 0; // This "should" not be reached
}
