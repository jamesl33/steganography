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
#include "steganography.hpp"
#include "exceptions.hpp"

/**
 * This class is created because we must perform testing on a subclass of
 * Steganography which implements the required abstract methods.
 */
class TestSteganography : public Steganography
{
    public:
        using Steganography::Steganography;

        virtual void Encode(const boost::filesystem::path &image_path) {}
        virtual void Decode() {}
};

TEST_CASE("Failure to open given image", "[Steganography]")
{
    REQUIRE_THROWS_AS(TestSteganography("test/files/nonexistent.png"), ImageException);
}
