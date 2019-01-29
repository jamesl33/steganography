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
    int bit_index = 0;
    int bits_written = 0;
    std::queue<unsigned char> chunk_bytes;

    // Copy the chunk into a queue
    for (unsigned char byte : chunk) {
        chunk_bytes.emplace(byte);
    }

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (chunk_bytes.empty()) {
                    return;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
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

                        if (chunk_bytes.empty()) {
                            // There is nothing left to encode.
                            return;
                        } else if (bits_written % 8 == 0) {
                            // We have encoded a full byte now, pop it from the queue.
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

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bits_written == 32) {
                    // We only need to encode a 32bit integer, stop once complete.
                    return;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
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

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bit_index == end) {
                    return chunk_bytes;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
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

                        if (bits_read == end - start) {
                            // We have decoded all of the bytes.
                            return chunk_bytes;
                        } else if (bits_read % 8 == 0) {
                            // We have decoded a full byte, place an empty once at the back of the chunk_bytes vector.
                            chunk_bytes.emplace_back(0);
                        }
                    }
                }

                bit_index++;
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
    int bit_index = 0;
    unsigned int chunk_length = 0;

    for (int row = 0; row < this -> image.rows; row++) {
        for (int col = 0; col < this -> image.cols; col++) {
            for (int cha = 0; cha < this -> image.channels(); cha++) {
                if (bits_read == 32) {
                    // We only need to encode a 32bit integer, stop once complete.
                    return chunk_length;
                }

                if (bit_index >= start) {
                    // TODO(James Lee) - Expose this option to the user.
                    for (int bit = 0; bit < 1; bit++) {
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

                bit_index++;
            }
        }
    }

    return 0; // This "should" not be reached
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
