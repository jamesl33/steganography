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
    int bit_index = 0;
    int bits_written = 0;
    std::queue<unsigned char> chunk_bytes;

    for (unsigned char byte : chunk) {
        chunk_bytes.emplace(byte);
    }

    for (int col = 0; col < this -> image.cols; col++) {
        for (int row = 0; row < this -> image.rows; row++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (chunk_bytes.empty()) {
                    return;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
                        switch (this -> image.channels()) {
                            case 3: {
                                this -> SetBit(&this -> image.at<cv::Vec3b>(col, row)[cha], bit, this -> GetBit(chunk_bytes.front(), bits_written % 8));
                                break;
                            }
                            case 4: {
                                this -> SetBit(&this -> image.at<cv::Vec4b>(col, row)[cha], bit, this -> GetBit(chunk_bytes.front(), bits_written % 8));
                                break;
                            }
                        }

                        bits_written++;

                        if (chunk_bytes.empty()) {
                            return;
                        } else if (bits_written % 8 == 0) {
                            chunk_bytes.pop();
                        }
                    }
                }

                bit_index++;
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
    int bit_index = 0;
    int bits_written = 0;

    for (int col = 0; col < this -> image.cols; col++) {
        for (int row = 0; row < this -> image.rows; row++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bits_written == 32) {
                    return;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
                        switch (this -> image.channels()) {
                            case 3: {
                                this -> SetBit(&this -> image.at<cv::Vec3b>(col, row)[cha], bit, this -> GetBit(chunk_length, bits_written));
                                break;
                            }
                            case 4: {
                                this -> SetBit(&this -> image.at<cv::Vec4b>(col, row)[cha], bit, this -> GetBit(chunk_length, bits_written));
                                break;
                            }
                        }

                        bits_written++;
                    }
                }

                bit_index++;
            }
        }
    }
}

/**
 * Decode a chunk of information from the carrier image.
 *
 * @param start The pixel index to start decoding at.
 * @param end The pixel index to stop encoding at.
 * @return The chunk of information read from the carrier image.
 */
std::vector<unsigned char> LeastSignificantBit::DecodeChunk(const int& start, const int& end) {
    int bit_index = 0;
    int bits_read = 0;
    std::vector<unsigned char> chunk_bytes = {0};

    for (int col = 0; col < this -> image.cols; col++) {
        for (int row = 0; row < this -> image.rows; row++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bit_index == end) {
                    return chunk_bytes;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
                        switch (this -> image.channels()) {
                            case 3: {
                                this -> SetBit(&chunk_bytes.back(), bits_read % 8, this -> GetBit(this -> image.at<cv::Vec3b>(col, row)[cha], bit));
                                break;
                            }
                            case 4: {
                                this -> SetBit(&chunk_bytes.back(), bits_read % 8, this -> GetBit(this -> image.at<cv::Vec4b>(col, row)[cha], bit));
                                break;
                            }
                        }

                        bits_read++;

                        if (bits_read == end - start) {
                            return chunk_bytes;
                        } else if (bits_read % 8 == 0) {
                            chunk_bytes.emplace_back(0);
                        }
                    }
                }

                bit_index++;
            }
        }
    }
}

/**
 * Decode a 32bit integer stating the length of the following chunk.
 *
 * @param start The pixel index to start decoding at.
 * @return The length of the next chunk in bytes.
 */
unsigned int LeastSignificantBit::DecodeChunkLength(const int& start) {
    int bits_read = 0;
    int bit_index = 0;
    unsigned int chunk_length = 0;

    for (int col = 0; col < this -> image.cols; col++) {
        for (int row = 0; row < this -> image.rows; row++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bits_read == 32) {
                    return chunk_length;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
                        switch (this -> image.channels()) {
                            case 3: {
                                this -> SetBit(&chunk_length, bits_read, this -> GetBit(this -> image.at<cv::Vec3b>(col, row)[cha], bit));
                                break;
                            }
                            case 4: {
                                this -> SetBit(&chunk_length, bits_read, this -> GetBit(this -> image.at<cv::Vec4b>(col, row)[cha], bit));
                                break;
                            }
                        }

                        bits_read++;
                    }
                }

                bit_index++;
            }
        }
    }
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
 * Set the n'th significant bit in an unsinged integer.
 *
 * This function assumes that it is being used correctly; there is no incorrect
 * input detection.
 *
 * @param integer The unsigned int you are modifying.
 * @param bit The bit you are setting.
 * @param value The value the bit will be set too.
 */
void LeastSignificantBit::SetBit(unsigned int* integer, const int& bit, const int& value) {
    *integer ^= (-(unsigned int)value ^ *integer) & (1UL << bit);
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
int LeastSignificantBit::GetBit(const unsigned char& byte, const int& bit) {
    return (byte >> bit) & 1UL;
}

/**
 * Get the n'th bit in an unsigned int.
 *
 * This function assumes that it is being used correctly; there is no incorrect
 * input detection.
 *
 * @param byte The unsigned int you are checking.
 * @param bit The bit you are checking.
 * @return The status of the n'th bit.
 */
int LeastSignificantBit::GetBit(const unsigned int& integer, const int& bit) {
    return (integer >> bit) & 1UL;
}
