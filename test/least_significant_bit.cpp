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

#include <vector>
#include <fstream>

#include <catch.hpp>
#include "least_significant_bit.hpp"
#include "exceptions.hpp"

TEST_CASE("Encode/Decode using the LSB technique", "[LeastSignificantBit]")
{
    std::vector<unsigned char> correct_payload = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\n'};
    std::vector<unsigned char> decoded_payload;

    LeastSignificantBit encode_lsb = LeastSignificantBit("test/files/solid_white.png");
    encode_lsb.Encode("test/files/hello_world.txt");

    std::ifstream steg_image("steg-solid_white.png");
    REQUIRE(steg_image.good());
    steg_image.close();

    LeastSignificantBit decode_lsb = LeastSignificantBit("steg-solid_white.png");
    decode_lsb.Decode();

    std::ifstream steg_file("steg-hello_world.txt");
    REQUIRE(steg_file.good());
    steg_file.close();

    boost::filesystem::ifstream check_file("steg-hello_world.txt", std::ios::binary);
    check_file.unsetf(std::ios::skipws); // do not skip whitespace
    decoded_payload.reserve(boost::filesystem::file_size("steg-hello_world.txt"));
    decoded_payload.insert(decoded_payload.begin(), std::istream_iterator<unsigned char>(check_file), std::istream_iterator<unsigned char>());
    check_file.close();

    REQUIRE(correct_payload == decoded_payload);

    remove("steg-solid_white.png");
    remove("steg-hello_world.txt");
}

TEST_CASE("Encode failure using the LSB technique", "[Encode]")
{
    LeastSignificantBit encode_lsb = LeastSignificantBit("test/files/solid_white.png");
    REQUIRE_THROWS_AS(encode_lsb.Encode("test/files/lorem_ipsum.txt"), EncodeException);
}

TEST_CASE("Decode failure using the LSB technique", "[Decode]")
{
    LeastSignificantBit decode_lsb = LeastSignificantBit("test/files/solid_white.png");
    REQUIRE_THROWS_AS(decode_lsb.Decode(), DecodeException);
}
