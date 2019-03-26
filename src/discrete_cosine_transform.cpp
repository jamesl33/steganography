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

void DiscreteCosineTransform::Encode(const boost::filesystem::path &payload_path)
{
    // Ensure that the carrier has enough room for the payload
    if (boost::filesystem::file_size(payload_path) * 8 > this->image_capacity)
    {
        throw EncodeException("Error: Failed to encode payload, carrier too small");
    }

    // Convert the filename to a vector<unsigned char>
    std::string payload_filename = payload_path.filename().string();
    std::vector<unsigned char> filename_bytes(payload_filename.begin(), payload_filename.end());

    // Encode the filename into the carrier image
    this->EncodeChunkLength(0, filename_bytes.size());
    this->EncodeChunk(32, filename_bytes);

    // Read the payload into a vector<unsigned char>
    std::vector<unsigned char> payload_bytes = this->ReadPayload(payload_path);

    this->EncodeChunkLength(32 + filename_bytes.size() * 8, payload_bytes.size());
    this->EncodeChunk(64 + filename_bytes.size() * 8, payload_bytes);

    // Encode the payload into the carrier image
    boost::filesystem::path steg_image_filename = this->image_path.filename();
    steg_image_filename.replace_extension(".jpg");

    // Merge the image channels and convert back to unsigned char
    cv::merge(this->channels, this->image);
    this->image.convertTo(this->image, CV_8U);

    // Write the steganographic image
    cv::imwrite("steg-" + steg_image_filename.string(), this->image, std::vector<int>{CV_IMWRITE_JPEG_QUALITY, 100});
}

void DiscreteCosineTransform::Decode()
{
    // Decode the filename from the steganographic image
    unsigned int filename_length = this->DecodeChunkLength(0);
    std::vector<unsigned char> filename_bytes = this->DecodeChunk(32, 32 + filename_length * 8);

    // Convert the filename vector<unsigned char> to a string
    std::string payload_filename(filename_bytes.begin(), filename_bytes.end());

    // Decode the payload from the steganographic image
    unsigned int payload_length = this->DecodeChunkLength(32 + filename_length * 8);
    std::vector<unsigned char> payload_bytes = this->DecodeChunk(64 + filename_length * 8, 64 + filename_length * 8 + payload_length * 8);

    // Write the decoded payload
    this->WritePayload("steg-" + payload_filename, payload_bytes);
}

void DiscreteCosineTransform::EncodeChunk(const int &start, const std::vector<unsigned char> &chunk)
{
    int bits_written = 0;

    for (int row = 0; row < this->image.rows - 8; row += 8)
    {
        for (int col = 0; col < this->image.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((this->image.cols - 8) / 8) * 8;
                col = start % ((this->image.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform the forward dct
            cv::dct(block, trans);

            // Embed the current chunk bit in the carrier
            this->SwapCoefficients(&trans, this->GetBit(chunk[bits_written / 8], bits_written % 8));

            // Perform the inverse dct
            cv::idct(trans, block);

            // Recentre the coefficients around 128
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) += 128;
                }
            }

            // We have finished embedding, clean up
            if (++bits_written == chunk.size() * 8)
            {
                return;
            }
        }
    }
}

void DiscreteCosineTransform::EncodeChunkLength(const int &start, const unsigned int &chunk_length)
{
    int bits_written = 0;

    for (int row = 0; row < this->image.rows - 8; row += 8)
    {
        for (int col = 0; col < this->image.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((this->image.cols - 8) / 8) * 8;
                col = start % ((this->image.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Centre the coefficients around 0
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) -= 128;
                }
            }

            // Perform the forward dct
            cv::dct(block, trans);

            // Swap N DCT coefficients
            this->SwapCoefficients(&trans, this->GetBit(chunk_length, bits_written));

            // Perform the inverse dct
            cv::idct(trans, block);

            // Recentre the coefficients around 128
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    block.at<float>(x, y) += 128;
                }
            }

            // We have finished embedding, clean up
            if (++bits_written == 32)
            {
                return;
            }
        }
    }
}

std::vector<unsigned char> DiscreteCosineTransform::DecodeChunk(const int &start, const int &end)
{
    std::vector<unsigned char> chunk_bytes((end - start) / 8);

    int bits_read = 0;

    for (int row = 0; row < this->image.rows - 8; row += 8)
    {
        for (int col = 0; col < this->image.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((this->image.cols - 8) / 8) * 8;
                col = start % ((this->image.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct
            cv::dct(block, trans);

            // Read from N swapped DCT coefficients
            this->SetBit(&chunk_bytes[bits_read / 8], bits_read % 8, (trans.at<float>(0, 2) < trans.at<float>(2, 0)));

            if (++bits_read == end - start)
            {
                // We have finished decoding, return the chunk
                return chunk_bytes;
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload");
}

unsigned int DiscreteCosineTransform::DecodeChunkLength(const int &start)
{
    unsigned int chunk_length = 0;

    int bits_read = 0;

    for (int row = 0; row < this->image.rows - 8; row += 8)
    {
        for (int col = 0; col < this->image.cols - 8; col += 8)
        {
            if (row == 0 && col == 0)
            {
                row = start / ((this->image.cols - 8) / 8) * 8;
                col = start % ((this->image.cols - 8) / 8) * 8;
            }

            // The current 8x8 block we are working on
            cv::Mat block(channels[0], cv::Rect(col, row, 8, 8));

            // A "working" copy of the block
            cv::Mat trans(cv::Size(8, 8), block.type());

            // Perform the forward dct
            cv::dct(block, trans);

            // Read from N swapped DCT coefficients
            this->SetBit(&chunk_length, bits_read, (trans.at<float>(0, 2) < trans.at<float>(2, 0)));

            if (++bits_read == 32)
            {
                // We have decoded the integer, check if it's valid
                if (chunk_length >= this->image_capacity || chunk_length == 0)
                {
                    throw DecodeException("Error: Failed to decode payload length");
                }

                return chunk_length;
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload length");
}

void DiscreteCosineTransform::SwapCoefficients(cv::Mat *block, const int &value)
{
    // Read two coefficients from the image block
    float low = block->at<float>(0, 2);
    float high = block->at<float>(2, 0);

    // Swap the coefficients so that low is low and high is high
    if (value && (low > high))
    {
        std::swap(low, high);
    }
    else if (!value && (low < high))
    {
        std::swap(low, high);
    }

    // Apply the persistence value
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

    // Write the coefficients back to the block
    block->at<float>(0, 2) = low;
    block->at<float>(2, 0) = high;
}
