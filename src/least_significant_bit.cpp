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

const int NUM_THREADS = std::thread::hardware_concurrency();

void LeastSignificantBit::Encode(const boost::filesystem::path &payload_path)
{
    // Ensure that the carrier has enough room for the payload
    if (boost::filesystem::file_size(payload_path) * 8 > this->image_capacity)
    {
        throw EncodeException("Error: Failed to encode payload, carrier too small");
    }

    // Convert the filename to a vector<unsigned char>
    std::string filename = payload_path.filename().string();
    std::vector<unsigned char> filename_bytes(filename.begin(), filename.end());

    // Encode the filename length into the carrier image
    this->EncodeChunkLength(0, filename_bytes.size());

    // Encode the filename into the carrier image
    this->EncodeChunk(32, filename_bytes.begin(), filename_bytes.end());

    // Read the payload into a vector<unsigned char>
    std::vector<unsigned char> payload_bytes = this->ReadPayload(payload_path);

    // Encode the payload length into the carrier image
    this->EncodeChunkLength(32 + (filename_bytes.size() * 8), payload_bytes.size());

    // Determine how many threads to use so that each thread encodes more than 3500KB
    int encode_threads = NUM_THREADS;

    while ((encode_threads > 1) && ((payload_bytes.size() / encode_threads)) < 3500)
    {
        encode_threads--;
    }

    // Encode the payload into the carrier image
    if (encode_threads == 1)
    {
        this->EncodeChunk(64 + (filename_bytes.size() * 8), payload_bytes.begin(), payload_bytes.end());
    }
    else
    {
        std::vector<std::thread> threads;
        threads.reserve(encode_threads);

        for (int i = 0; i < encode_threads; i++)
        {
            threads.push_back(
                    std::thread(&LeastSignificantBit::EncodeChunk,
                        this,
                        64 + (filename_bytes.size() * 8) + (((payload_bytes.size() / encode_threads) * 8) * i),
                        payload_bytes.begin() + ((payload_bytes.size() / encode_threads) * i),
                        payload_bytes.end() - ((payload_bytes.size() / encode_threads) * ((encode_threads - 1) - i))));
        }

        // Wait for all the threads to finish encoding
        for (std::thread &thr : threads)
        {
            thr.join();
        }
    }

    // Write the steganographic image
    cv::imwrite("steg-" + this->image_path.filename().replace_extension(".png").string(), this->image,
            std::vector<int>{cv::IMWRITE_PNG_STRATEGY_HUFFMAN_ONLY, 1});
}

void LeastSignificantBit::Decode()
{
    // Decode the filename from the steganographic image
    unsigned int filename_length = this->DecodeChunkLength(0);
    std::vector<unsigned char> filename_bytes(filename_length);
    this->DecodeChunk(32, filename_bytes.begin(), filename_bytes.end());

    // Convert the filename vector<unsigned char> to a string
    std::string payload_filename(filename_bytes.begin(), filename_bytes.end());

    // Decode the payload length from the steganographic image
    unsigned int payload_length = this->DecodeChunkLength(32 + (filename_length * 8));

    // Determine how many threads to use so that each thread encodes more than 3500KB
    int decode_threads = NUM_THREADS;

    while ((decode_threads > 1) && (payload_length / decode_threads) < 3500)
    {
        decode_threads--;
    }

    // Decode the payload from the steganographic image
    std::vector<unsigned char> payload_bytes(payload_length);

    if (decode_threads == 1)
    {
        this->DecodeChunk(64 + (filename_length * 8), payload_bytes.begin(), payload_bytes.end());
    }
    else
    {
        std::vector<std::thread> threads;
        threads.reserve(decode_threads);

        for (int i = 0; i < decode_threads; i++)
        {
            threads.push_back(
                std::thread(&LeastSignificantBit::DecodeChunk,
                    this,
                    64 + (filename_length * 8) + (((payload_length / decode_threads) * 8) * i),
                    payload_bytes.begin() + ((payload_length / decode_threads) * i),
                    payload_bytes.end() - ((payload_length / decode_threads) * ((decode_threads - 1) - i))));
        }

        // Wait for all the threads to finish encoding
        for (std::thread &thr : threads)
        {
            thr.join();
        }
    }

    // Write the decoded payload
    this->WritePayload("steg-" + payload_filename, payload_bytes);
}

void LeastSignificantBit::EncodeChunk(const int &start, std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator en)
{
    int bit = 0;
    bool loops_initialised = false;

    for (int row = 0; row < this->image.rows; row++)
    {
        for (int col = 0; col < this->image.cols; col++)
        {
            for (int cha = 0; cha < this->image.channels(); cha++)
            {
                if (!loops_initialised)
                {
                    row = (start / this->image.channels()) / this->image.cols;
                    col = (start / this->image.channels()) % this->image.cols;
                    cha = start % this->image.channels();
                    loops_initialised = true;
                }

                this->SetBit(&this->image.at<cv::Vec3b>(row, col)[cha], 0, this->GetBit(*it, bit % 8));

                if (++bit % 8 == 0 && ++it == en)
                {
                    return;
                }
            }
        }
    }
}

void LeastSignificantBit::EncodeChunkLength(const int &start, const unsigned int &chunk_length)
{
    int bit = 0;
    bool loops_initialised = false;

    for (int row = 0; row < this->image.rows; row++)
    {
        for (int col = 0; col < this->image.cols; col++)
        {
            for (int cha = 0; cha < this->image.channels(); cha++)
            {
                if (!loops_initialised)
                {
                    row = (start / this->image.channels()) / this->image.cols;
                    col = (start / this->image.channels()) % this->image.cols;
                    cha = start % this->image.channels();
                    loops_initialised = true;
                }

                this->SetBit(&this->image.at<cv::Vec3b>(row, col)[cha], 0, this->GetBit(chunk_length, bit));

                if (++bit == 32)
                {
                    return;
                }
            }
        }
    }
}

void LeastSignificantBit::DecodeChunk(const int start, std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator en)
{
    int bit = 0;
    bool loops_initialised = false;

    for (int row = 0; row < this->image.rows; row++)
    {
        for (int col = 0; col < this->image.cols; col++)
        {
            for (int cha = 0; cha < this->image.channels(); cha++)
            {
                if (!loops_initialised)
                {
                    row = (start / this->image.channels()) / this->image.cols;
                    col = (start / this->image.channels()) % this->image.cols;
                    cha = start % this->image.channels();
                    loops_initialised = true;
                }

                this->SetBit(&(*it), bit % 8, this->GetBit(this->image.at<cv::Vec3b>(row, col)[cha], 0));

                if (++bit % 8 == 0 && ++it == en)
                {
                    return;
                }
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload");
}

unsigned int LeastSignificantBit::DecodeChunkLength(const int &start)
{
    unsigned int chunk_length = 0;

    int bit = 0;
    bool loops_initialised = false;

    for (int row = 0; row < this->image.rows; row++)
    {
        for (int col = 0; col < this->image.cols; col++)
        {
            for (int cha = 0; cha < this->image.channels(); cha++)
            {
                if (!loops_initialised)
                {
                    row = (start / this->image.channels()) / this->image.cols;
                    col = (start / this->image.channels()) % this->image.cols;
                    cha = start % this->image.channels();
                    loops_initialised = true;
                }

                this->SetBit(&chunk_length, bit, this->GetBit(this->image.at<cv::Vec3b>(row, col)[cha], 0));

                if (++bit == 32)
                {
                    if (chunk_length == 0 || chunk_length > this->image_capacity)
                    {
                        throw DecodeException("Error: Failed to decode payload length");
                    }

                    return chunk_length;
                }
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload length");
}
