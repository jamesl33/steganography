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
 * Encode the payload file into the carrier image by embedding into one or more
 * least significant bits.
 *
 * @param payload_path Path to the file we are encoding.
 * @exception EncodeException thrown when encoding fails.
 */
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

    // Encode the filename into the carrier image
    this->EncodeChunkLength(0, filename_bytes.size());
    this->EncodeChunk(32, filename_bytes);

    // Read the payload into a vector<unsigned char>
    std::vector<unsigned char> payload_bytes = this->ReadPayload(payload_path);

    // Encode the payload into the carrier image
    this->EncodeChunkLength(32 + filename_bytes.size() * 8, payload_bytes.size());
    this->EncodeChunk(64 + filename_bytes.size() * 8, payload_bytes);

    // Write the steganographic image
    cv::imwrite("steg-" + this->image_path.filename().replace_extension(".png").string(), this->image,
            std::vector<int>{cv::IMWRITE_PNG_STRATEGY_HUFFMAN_ONLY, 1});
}

/**
 * Decode the payload from the steganographic image by reading from one or more
 * least significant bits.
 */
void LeastSignificantBit::Decode()
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

/**
 * Encode a chunk of information into the carrier image.
 *
 * Before encoding a chunk of information you "should" first encode its length
 * using the EncodeChunkLength function.
 *
 * @param start The bit index to start encoding at.
 * @param chunk The chunk of information which will be encoded.
 */
void LeastSignificantBit::EncodeChunk(const int &start, const std::vector<unsigned char> &chunk)
{
    int bit = 0;
    cv::MatIterator_<cv::Vec3b> it = this->image.begin<cv::Vec3b>() + start;
    cv::MatIterator_<cv::Vec3b> en = this->image.end<cv::Vec3b>();

    for (; it != en; it++)
    {
        for (int cha = 0; cha < this->image.channels(); cha++)
        {
            for (int depth = 0; depth < this->bit_depth; depth++)
            {
                // Embed the current chunk bit in the carrier
                this->SetBit(&(*it)[cha], depth, this->GetBit(chunk[bit / 8], bit % 8));

                if (++bit == chunk.size() * 8)
                {
                    // We have finished decoding
                    return;
                }
            }
        }
    }
}

/**
 * Encode a 32bit integer stating the length of the following chunk into the
 * carrier image.
 *
 * @param start The bit index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void LeastSignificantBit::EncodeChunkLength(const int &start, const unsigned int &chunk_length)
{
    int bit = 0;
    cv::MatIterator_<cv::Vec3b> it = this->image.begin<cv::Vec3b>() + start;
    cv::MatIterator_<cv::Vec3b> en = this->image.end<cv::Vec3b>();

    for (; it != en; it++)
    {
        for (int cha = 0; cha < this->image.channels(); cha++)
        {
            for (int depth = 0; depth < this->bit_depth; depth++)
            {
                // Embed the current integer bit in the carrier
                this->SetBit(&(*it)[cha], depth, this->GetBit(chunk_length, bit));

                if (++bit == 32)
                {
                    // We have finished decoding
                    return;
                }
            }
        }
    }
}

/**
 * Attempt to decode a chunk of information from the steganographic image.
 *
 * @param start The bit index to start decoding at.
 * @param end The bit index to stop decoding at.
 * @return The chunk of information read from the steganographic image.
 * @exception DecodeException Thrown when decoding fails.
 */
std::vector<unsigned char> LeastSignificantBit::DecodeChunk(const int &start, const int &end)
{
    std::vector<unsigned char> chunk((end - start) / 8);

    int bit = 0;
    cv::MatIterator_<cv::Vec3b> it = this->image.begin<cv::Vec3b>() + start;
    cv::MatIterator_<cv::Vec3b> en = this->image.end<cv::Vec3b>();

    for (; it != en; it++)
    {
        for (int cha = 0; cha < this->image.channels(); cha++)
        {
            for (int depth = 0; depth < this->bit_depth; depth++)
            {
                // Read the current bit from the steganographic image
                this->SetBit(&chunk[bit / 8], bit % 8, this->GetBit((*it)[cha], depth));

                if (++bit == end - start)
                {
                    // We have decoded the chunk, return it
                    return chunk;
                }
            }
        }
    }

    throw DecodeException("Error: Failed to decode payload");
}

/**
 * Attempt to decode the 32bit integer stating the length of the following
 * chunk.
 *
 * @param start The bit index to start decoding at.
 * @return The length of the following chunk.
 * @exception DecodeException Thrown when decoding fails.
 */
unsigned int LeastSignificantBit::DecodeChunkLength(const int &start)
{
    unsigned int chunk_length = 0;

    int bit = 0;
    cv::MatIterator_<cv::Vec3b> it = this->image.begin<cv::Vec3b>() + start;
    cv::MatIterator_<cv::Vec3b> en = this->image.end<cv::Vec3b>();

    for (; it != en; it++)
    {
        for (int cha = 0; cha < this->image.channels(); cha++)
        {
            for (int depth = 0; depth < this->bit_depth; depth++)
            {
                // Read the current bit from the steganographic image
                this->SetBit(&chunk_length, bit, this->GetBit((*it)[cha], depth));

                if (++bit == 32)
                {
                    // We have decoded the integer, check if it's valid
                    if (chunk_length > this->image_capacity)
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
