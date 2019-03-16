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

#include <catch.hpp>
#include "discrete_cosine_transform.hpp"
#include "exceptions.hpp"

TEST_CASE("Encode/Decode using the DCT technique", "[DiscreteCosineTransform]")
{
    std::vector<unsigned char> correct_payload = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\n'};
    std::vector<unsigned char> decoded_payload;

    for (int swaps = 1; swaps<= 4; swaps++)
    {
        SECTION("Encode/Decode with a swaps value of " + std::to_string(swaps), "[Encode/Decode]")
        {
            DiscreteCosineTransform encode_dct = DiscreteCosineTransform("test/files/solid_white.png", swaps, 10);
            encode_dct.Encode("test/files/hello_world.txt");

            std::ifstream steg_image("steg-solid_white.jpg");
            REQUIRE(steg_image.good());
            steg_image.close();

            DiscreteCosineTransform decode_dct = DiscreteCosineTransform("steg-solid_white.jpg", swaps, 10);
            decode_dct.Decode();

            std::ifstream steg_file("steg-hello_world.txt");
            REQUIRE(steg_file.good());
            steg_file.close();
        }
    }

    boost::filesystem::ifstream steg_file("steg-hello_world.txt", std::ios::binary);
    steg_file.unsetf(std::ios::skipws);
    decoded_payload.reserve(boost::filesystem::file_size("steg-hello_world.txt"));
    decoded_payload.insert(decoded_payload.begin(), std::istream_iterator<unsigned char>(steg_file), std::istream_iterator<unsigned char>());
    steg_file.close();

    REQUIRE(correct_payload == decoded_payload);

    remove("steg-solid_white.jpg");
    remove("steg-hello_world.txt");
}

TEST_CASE("Encode failure using the DCT technique", "[DiscreteCosineTransform]")
{
    DiscreteCosineTransform encode_dct = DiscreteCosineTransform("test/files/solid_white.png", 1, 1);
    REQUIRE_THROWS_AS(encode_dct.Encode("test/files/lorem_ipsum.txt"), EncodeException);
}

TEST_CASE("Decode failure using the DCT technique", "[DiscreteCosineTransform]")
{
    DiscreteCosineTransform decode_dct = DiscreteCosineTransform("test/files/solid_white.png", 1, 1);
    REQUIRE_THROWS_AS(decode_dct.Decode(), DecodeException);
}
