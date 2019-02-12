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

#include "least_significant_bit.hpp"

/**
 * Encode the payload into the carrier image using LSB.
 *
 * @param payload_path The path to the file you are encoding.
 */
void LeastSignificantBit::Encode(const boost::filesystem::path& payload_path) {
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
    steg_image_filename.replace_extension(".png");

    // TODO(James Lee) - Expose png compression to the user.
    cv::imwrite("steg-" + steg_image_filename.string(), this -> image, std::vector<int>{CV_IMWRITE_PNG_COMPRESSION, 9});
}

/**
 * Decode the payload from the carrier image.
 */
void LeastSignificantBit::Decode() {
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
 * @param start The pixel index to start encoding at.
 * @param chunk The chunk of information that will be encoded into the carrier image.
 */
void LeastSignificantBit::EncodeChunk(const int& start, const std::vector<unsigned char>& chunk) {
    int bits_written = 0;
    std::queue<unsigned char> chunk_bytes;

    // Copy the chunk into a queue
    for (unsigned char byte : chunk) {
        chunk_bytes.emplace(byte);
    }

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (row  == 0 && col == 0 && cha == 0) {
                    row = (start / this -> image.channels()) / this -> image.cols;
                    col = (start / this -> image.channels() % this -> image.cols);
                    cha = (start % this -> image.channels());
                }

                if (chunk_bytes.empty()) {
                    return;
                }

                for (int bit = 0; bit < this -> bit_depth; bit++) {
                    switch (this -> image.channels()) {
                        case 3: {
                            this -> SetBit(&this -> image.at<cv::Vec3b>(row, col)[cha], bit, this -> GetBit(chunk_bytes.front(), bits_written % 8));
                            break;
                        }
                        case 4: {
                            this -> SetBit(&this -> image.at<cv::Vec4b>(row, col)[cha], bit, this -> GetBit(chunk_bytes.front(), bits_written % 8));
                            break;
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
    }
}

/**
 * Encode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void LeastSignificantBit::EncodeChunkLength(const int& start, const unsigned int& chunk_length) {
    int bits_written = 0;

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (row  == 0 && col == 0 && cha == 0) {
                    row = (start / this -> image.channels()) / this -> image.rows;
                    col = (start / this -> image.channels() % this -> image.cols);
                    cha = (start % this -> image.channels());
                }

                if (bits_written == 32) {
                    // We only need to encode a 32bit integer, stop once complete.
                    return;
                }

                for (int bit = 0; bit < this -> bit_depth; bit++) {
                    switch (this -> image.channels()) {
                        case 3: {
                            this -> SetBit(&this -> image.at<cv::Vec3b>(row, col)[cha], bit, this -> GetBit(chunk_length, bits_written));
                            break;
                        }
                        case 4: {
                            this -> SetBit(&this -> image.at<cv::Vec4b>(row, col)[cha], bit, this -> GetBit(chunk_length, bits_written));
                            break;
                        }
                    }

                    bits_written++;
                }
            }
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
std::vector<unsigned char> LeastSignificantBit::DecodeChunk(const int& start, const int& end) {
    int bits_read = 0;
    std::vector<unsigned char> chunk_bytes = {0};

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (row  == 0 && col == 0 && cha == 0) {
                    row = (start / this -> image.channels()) / this -> image.rows;
                    col = (start / this -> image.channels() % this -> image.cols);
                    cha = (start % this -> image.channels());
                }

                if (bits_read == end - start) {
                    return chunk_bytes;
                }

                for (int bit = 0; bit < this -> bit_depth; bit++) {
                    switch (this -> image.channels()) {
                        case 3: {
                            this -> SetBit(&chunk_bytes.back(), bits_read % 8, this -> GetBit(this -> image.at<cv::Vec3b>(row, col)[cha], bit));
                            break;
                        }
                        case 4: {
                            this -> SetBit(&chunk_bytes.back(), bits_read % 8, this -> GetBit(this -> image.at<cv::Vec4b>(row, col)[cha], bit));
                            break;
                        }
                    }

                    bits_read++;

                    if (!(bits_read == end - start) && (bits_read % 8 == 0)) {
                        // We have decoded a full byte, place an empty once at the back of the chunk_bytes vector.
                        chunk_bytes.emplace_back(0);
                    }
                }
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
unsigned int LeastSignificantBit::DecodeChunkLength(const int& start) {
    int bits_read = 0;
    unsigned int chunk_length = 0;

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (row  == 0 && col == 0 && cha == 0) {
                    row = (start / this -> image.channels()) / this -> image.rows;
                    col = (start / this -> image.channels() % this -> image.cols);
                    cha = (start % this -> image.channels());
                }

                if (bits_read == 32) {
                    // We only need to decode a 32bit integer, stop once complete.
                    return chunk_length;
                }

                for (int bit = 0; bit < this -> bit_depth; bit++) {
                    switch (this -> image.channels()) {
                        case 3: {
                            this -> SetBit(&chunk_length, bits_read, this -> GetBit(this -> image.at<cv::Vec3b>(row, col)[cha], bit));
                            break;
                        }
                        case 4: {
                            this -> SetBit(&chunk_length, bits_read, this -> GetBit(this -> image.at<cv::Vec4b>(row, col)[cha], bit));
                            break;
                        }
                    }

                    bits_read++;
                }
            }
        }
    }

    return 0; // This "should" not be reached
}
