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

    DiscreteCosineTransform encode_dct = DiscreteCosineTransform("test/files/solid_white.png", 5);
    encode_dct.Encode("test/files/hello_world.txt");

    // check to see if the steganographic image was saved with the correct filename
    std::ifstream steg_carrier("steg-solid_white.jpg");
    REQUIRE(steg_carrier.good());

    DiscreteCosineTransform decode_dct = DiscreteCosineTransform("steg-solid_white.jpg", 5);
    decode_dct.Decode();

    // check to see if the payload was decoded with the correct filename
    std::ifstream steg_payload("steg-hello_world.txt");
    REQUIRE(steg_payload.good());

    // check to if the payload was decoded correctly
    char byte;
    std::vector<unsigned char> decoded_payload;

    while (steg_payload.get(byte))
    {
        decoded_payload.emplace_back(byte);
    }

    REQUIRE(correct_payload == decoded_payload);

    // clean up ready for the next loop
    remove("steg-solid_white.jpg");
    remove("steg-hello_world.txt");
}

TEST_CASE("Encode failure using the DCT technique", "[DiscreteCosineTransform]")
{
    DiscreteCosineTransform encode_dct = DiscreteCosineTransform("test/files/solid_white.png", 1);
    REQUIRE_THROWS_AS(encode_dct.Encode("test/files/lorem_ipsum.txt"), EncodeException);
}

TEST_CASE("Decode failure using the DCT technique", "[DiscreteCosineTransform]")
{
    DiscreteCosineTransform decode_dct = DiscreteCosineTransform("test/files/solid_white.png", 1);
    REQUIRE_THROWS_AS(decode_dct.Decode(), DecodeException);
}
