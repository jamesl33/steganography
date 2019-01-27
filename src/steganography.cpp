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

#include "steganography.hpp"

/**
 * Read all the bytes from a payload file into a vector.
 *
 * @param payload_path The path to the file to read as the payload.
 * @return A vector containing all the bytes from the payload file.
 */
std::vector<unsigned char> Steganography::ReadPayload(const boost::filesystem::path& payload_path) {
    std::ifstream file(payload_path.string(), std::ios::binary);
    std::vector<unsigned char> payload;

    if (file.good()) {
        for (char byte = file.get(); byte != -1; byte = file.get()) {
            payload.emplace_back(byte);
        }
    } else {
        std::cerr << "Error: Failed to open input payload file" << std::endl;
    }

    file.close();
    return payload;
}

/**
 * Write all the bytes decoded from the carrier image to a file.
 *
 * @param payload_path The path to the file that will be created.
 * @param payload The payload decoded from the carrier image.
 */
void Steganography::WritePayload(const boost::filesystem::path& payload_path, const std::vector<unsigned char>& payload) {
    std::ofstream file(payload_path.string(), std::ios::binary);

    if (file.good()) {
        for (unsigned char byte : payload) {
            file.put(byte);
        }
    } else {
        std::cerr << "Error: Failed to open output payload file" << std::endl;
    }

    file.close();
}
