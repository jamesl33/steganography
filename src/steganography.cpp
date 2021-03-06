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

std::vector<unsigned char> Steganography::ReadPayload(const boost::filesystem::path &payload_path)
{
    boost::filesystem::ifstream file(payload_path, std::ios::binary);
    file.unsetf(std::ios::skipws); // do not skip whitespace

    std::vector<unsigned char> payload;
    payload.reserve(boost::filesystem::file_size(payload_path));
    payload.insert(payload.begin(), std::istream_iterator<unsigned char>(file), std::istream_iterator<unsigned char>());

    file.close();
    return payload;
}

void Steganography::WritePayload(const boost::filesystem::path &payload_path, const std::vector<unsigned char> &payload)
{
    boost::filesystem::ofstream file(payload_path, std::ios::binary);

    std::copy(payload.cbegin(), payload.cend(), std::ostream_iterator<unsigned char>(file));

    file.close();
}
