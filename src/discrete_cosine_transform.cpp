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

/**
 * The default JPEG luminance quantisation table with all it's values divided
 * by 10.
 */
const int quant[8][8] = {{1, 1, 1, 1, 2, 4, 5, 6},
                         {1, 1, 1, 1, 2, 5, 6, 5},
                         {1, 1, 1, 2, 4, 5, 6, 5},
                         {1, 1, 2, 2, 5, 8, 8, 6},
                         {1, 2, 3, 5, 6, 10, 10, 7},
                         {2, 3, 5, 6, 8, 10, 11, 9},
                         {4, 6, 7, 8, 10, 12, 12, 10},
                         {7, 9, 9, 9, 11, 10, 10, 9}};

/**
 * Encode the payload into the carrier image using DCT coefficient swapping.
 *
 * @param payload_path The path to the file you are encoding.
 */
void DiscreteCosineTransform::Encode(const boost::filesystem::path &payload_path)
{
    // Convert the payload filename to byte vector.
    std::string payload_filename = payload_path.filename().string();
    std::vector<unsigned char> payload_filename_bytes(payload_filename.begin(), payload_filename.end());

    // Encode the length/content of the filename in the carrier image.
    this->EncodeChunkLength(0, payload_filename_bytes.size());
    this->EncodeChunk(32, payload_filename_bytes);

    // Read the payload into a byte vector.
    std::vector<unsigned char> payload_bytes = this->ReadPayload(payload_path);

    // Encode the length/content of the payload in the carrier image.
    this->EncodeChunkLength(32 + payload_filename_bytes.size() * 8, payload_bytes.size());
    this->EncodeChunk(64 + payload_filename_bytes.size() * 8, payload_bytes);

    // Save the modified image with the "steg-" prefix.
    boost::filesystem::path steg_image_filename = this->image_path.filename();
    steg_image_filename.replace_extension(".jpg");

    // TODO(James Lee) - Expose JPEG quality to the user.
    cv::imwrite("steg-" + steg_image_filename.string(), this->image, std::vector<int>{CV_IMWRITE_JPEG_QUALITY, 100});
}

/**
 * Decode the payload from the carrier image.
 */
void DiscreteCosineTransform::Decode()
{
    // Decode the filename byte vector from the carrier image.
    unsigned int payload_filename_length = this->DecodeChunkLength(0);
    std::vector<unsigned char> payload_filename_bytes = this->DecodeChunk(32, 32 + payload_filename_length * 8);

    // Convert the payload filename to a string.
    std::string payload_filename(payload_filename_bytes.begin(), payload_filename_bytes.end());

    // Decode the payload byte vector from the carrier image.
    unsigned int payload_length = this->DecodeChunkLength(32 + payload_filename_length * 8);
    std::vector<unsigned char> payload_bytes = this->DecodeChunk(64 + payload_filename_length * 8, 64 + payload_filename_length * 8 + payload_length * 8);

    // Save the payload to it's filename with the "steg-" prefix.
    this->WritePayload("steg-" + payload_filename, payload_bytes);
}

/**
 * Encode a chunk of information into the carrier image.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk The chunk of information that will be encoded into the carrier image.
 */
void DiscreteCosineTransform::EncodeChunk(const int &start, const std::vector<unsigned char> &chunk)
{
    int bits_written = 0;
    std::queue<unsigned char> chunk_bytes;

    // Copy the payload into a queue.
    for (unsigned char byte : chunk)
    {
        chunk_bytes.emplace(byte);
    }

    // Convert the image to floating point.
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // Stop once we have embedded the whole message.
            if (chunk_bytes.empty())
            {
                // Merge the image channels.
                cv::Mat mergedfp;
                cv::merge(channels, mergedfp);

                mergedfp.convertTo(this->image, CV_8U);
                return;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform forward dct.
            cv::dct(block, trans);

            // Quantize the 8x8 block.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    trans.at<float>(x, y) = trans.at<float>(x, y) / quant[x][y];
                }
            }

            this->SwapCoefficients(&trans, this->GetBit(chunk_bytes.front(), bits_written % 8));

            // Dequantize the 8x8 block.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    trans.at<float>(x, y) *= quant[x][y];
                }
            }

            // Perform the inverse dct.
            cv::idct(trans, block);

            // Centre the coefficients around 128.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) += 128;
                }
            }

            bits_written++;

            if (bits_written % 8 == 0)
            {
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
void DiscreteCosineTransform::EncodeChunkLength(const int &start, const unsigned int &chunk_length)
{
    int bits_written = 0;

    // Convert the image to floating point.
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            // We only need to encode a 32bit integer, stop once complete.
            if (bits_written == 32)
            {
                // Merge the image channels.
                cv::Mat mergedfp;
                cv::merge(channels, mergedfp);

                // Convert the image to unsigned char.
                mergedfp.convertTo(this->image, CV_8U);
                return;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform forward dct.
            cv::dct(block, trans);

            // Quantize the 8x8 block.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    trans.at<float>(x, y) = trans.at<float>(x, y) / quant[x][y];
                }
            }

            this->SwapCoefficients(&trans, this->GetBit(chunk_length, bits_written));

            // Dequantize the 8x8 block.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    trans.at<float>(x, y) *= quant[x][y];
                }
            }

            // Performce the inverse dct.
            cv::idct(trans, block);

            // Center the coefficients around 128.
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
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
std::vector<unsigned char> DiscreteCosineTransform::DecodeChunk(const int &start, const int &end)
{
    int bits_read = 0;
    std::vector<unsigned char> chunk_bytes = {0};

    // Convert the image to floating point.
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            if (bits_read == end - start)
            {
                // We only need to decode a 32bit integer, stop once complete.
                return chunk_bytes;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct.
            cv::dct(block, trans);

            // TODO(James Lee) - Read from multiple swapped coefficients.
            // Decode the data from the swapped coefficients.
            this->SetBit(&chunk_bytes.back(), bits_read % 8, (trans.at<float>(2, 0) > trans.at<float>(0, 2)));

            bits_read++;

            if (!(bits_read == end - start) && (bits_read % 8 == 0))
            {
                // We have decoded a full byte, place an empty once at the back of the chunk_bytes vector.
                chunk_bytes.emplace_back(0);
            }
        }
    }

    return std::vector<unsigned char>{}; // This "should" not be reached
}

/**
 * Decode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start decoding at.
 * @return The length of the next chunk in bytes.
 */
unsigned int DiscreteCosineTransform::DecodeChunkLength(const int &start)
{
    int bits_read = 0;
    unsigned int chunk_length = 0;

    // Convert the image to floating point.
    cv::Mat imagefp;
    this->image.convertTo(imagefp, CV_32F);

    // Split the image into channels.
    std::vector<cv::Mat> channels;
    cv::split(imagefp, channels);

    for (int row = 0; row < imagefp.rows - 8; row += 8)
    {
        for (int col = 0; col < imagefp.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((imagefp.cols - 8) / 8) * 8;
                col = start % ((imagefp.cols - 8) / 8) * 8;
            }

            if (bits_read == 32)
            {
                // We only need to decode a 32bit integer, stop once complete.
                return chunk_length;
            }

            // The current 8x8 block we are working on.
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block.
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct.
            cv::dct(block, trans);

            // TODO(James Lee) - Read from multiple swapped coefficients.
            // Decode the data from the swapped coefficients.
            this->SetBit(&chunk_length, bits_read, (trans.at<float>(2, 0) > trans.at<float>(0, 2)));

            bits_read++;
        }
    }

    return 0; // This "should" not be reached
}

/**
 * Swap two DCT coefficients.
 *
 * Swap two DCT coefficients and apply a persistence value to ensure that the
 * data survives the compression process.
 *
 * @param block Pointer to the 8x8 block we are currently operating on.
 * @param value The value which is being stored, should be 0 or 1.
 */
void DiscreteCosineTransform::SwapCoefficients(cv::Mat *block, const int &value)
{
    float low = block->at<float>(0, 2);
    float high = block->at<float>(2, 0);

    // Swap the dct coefficients to match the value bit.
    if (value && (low > high))
    {
        std::swap(low, high);
    }
    else if (!value && (low < high))
    {
        std::swap(low, high);
    }

    if (value && (low == high || low < high))
    {
        low -= this->persistence;
        high += this->persistence;
    }
    else if (!value && (low == high || low > high))
    {
        low += this->persistence;
        high -= this->persistence;
    }

    block->at<float>(0, 2) = low;
    block->at<float>(2, 0) = high;
}
