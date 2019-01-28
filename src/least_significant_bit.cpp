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
 * Encode the payload into the carrier image.
 *
 * @param payload_path The path to the file you are encoding.
 */
void LeastSignificantBit::Encode(const boost::filesystem::path& payload_path) {

}

/**
 * Decode the payload from the carrier image.
 */
void LeastSignificantBit::Decode() {

}

/**
 * Encode a chunk of information into the carrier image.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk The chunk of information that will be encoded into the carrier image.
 */
void LeastSignificantBit::EncodeChunk(const int& start, const std::vector<unsigned char>& chunk) {

}

/**
 * Encode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void LeastSignificantBit::EncodeChunkLength(const int& start, const unsigned int& chunk_length) {

}

/**
 * Decode a chunk of information from the carrier image.
 *
 * @param start The pixel index to start decoding at.
 * @param end The pixel index to stop encoding at.
 * @return The chunk of information read from the carrier image.
 */
std::vector<unsigned char> LeastSignificantBit::DecodeChunk(const int& start, const int& end) {

}

/**
 * Decode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start decoding at.
 * @return The length of the next chunk in bytes.
 */
unsigned int LeastSignificantBit::DecodeChunkLength(const int& start) {

}

/**
 * Set the n'th significant bit in an unsigned char.
 *
 * This function assumes that it is being used correctly; there is no incorrect
 * input detection.
 *
 * @param byte The unsigned char you are modifying.
 * @param bit The bit you are setting.
 * @param value The value the bit will be set too.
 */
void LeastSignificantBit::SetBit(unsigned char* byte, const int& bit, const int& value) {
    *byte ^= (-(unsigned int)value ^ *byte) & (1UL << bit);
}

/**
 * Get the n'th bit in an unsigned char.
 *
 * This function assumes that it is being used correctly; there is no incorrect
 * input detection.
 *
 * @param byte The unsigned char you are checking.
 * @param bit The bit you are checking.
 * @return The status of the n'th bit.
 */
int LeastSignificantBit::GetBit(unsigned char& byte, const int& bit) {
    return (byte >> bit) & 1U;
}
