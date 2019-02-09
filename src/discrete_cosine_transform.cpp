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
 * Encode the payload into the carrier image using DCT coefficient swapping.
 *
 * @param payload_path The path to the file you are encoding.
 */
void DiscreteCosineTransform::Encode(const boost::filesystem::path& payload_path) {

}

/**
 * Decode the payload from the carrier image.
 */
void DiscreteCosineTransform::Decode() {

}

/**
 * Encode a chunk of information into the carrier image.
 *
 * @param start The pixel index to start encoding at.
 * @param chunk The chunk of information that will be encoded into the carrier image.
 */
void DiscreteCosineTransform::EncodeChunk(const int& start, const std::vector<unsigned char>& chunk) {

}

/**
 * Encode a 32bit integer stating the length of the following chunk
 *
 * @param start The pixel index to start encoding at.
 * @param chunk_length The length of the next chunk in bytes.
 */
void DiscreteCosineTransform::EncodeChunkLength(const int& start, const unsigned int& chunk_length) {

}

/**
 * Decode a chunk of information from the carrier image.
 *
 * @param start The pixel index to start decoding at.
 * @param end The pixel index to stop decoding at.
 * @return The chunk of information read from the carrier image.
 */
std::vector<unsigned char> DiscreteCosineTransform::DecodeChunk(const int& start, const int& end) {
    return std::vector<unsigned char> {}; // This "should" not be reached
}

/**
 * Decode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start decoding at.
 * @return The length of the next chunk in bytes.
 */
unsigned int DiscreteCosineTransform::DecodeChunkLength(const int& start) {
    return 0; // This "should" not be reached
}
