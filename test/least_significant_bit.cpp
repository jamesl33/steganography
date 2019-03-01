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

TEST_CASE("Encode/Decode using the LSB technique", "[LeastSignificantBit]")
{
    std::vector<unsigned char> correct_payload = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\n'};

    for (int depth = 1; depth < 8; depth++)
    {
        LeastSignificantBit encode_lsb = LeastSignificantBit("test/files/solid_white.png", depth);
        encode_lsb.Encode("test/files/hello_world.txt");

        // check to see if the steganographic image was saved with the correct filename
        std::ifstream steg_carrier("steg-solid_white.png");
        REQUIRE(steg_carrier.good());

        LeastSignificantBit decode_lsb = LeastSignificantBit("steg-solid_white.png", depth);
        decode_lsb.Decode();

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
        remove("steg-solid_white.png");
        remove("steg-hello_world.txt");
    }
}
