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

#include <queue>
#include "steganography.hpp"
#include "exceptions.hpp"

#ifndef LEAST_SIGNIFICANT_BIT_HPP
#define LEAST_SIGNIFICANT_BIT_HPP

class LeastSignificantBit : public Steganography
{
    public:
        /**
         * Default constructor for LeastSignificantBit class overrides the default
         * constructor for the Steganography class.
         * @param image_path The path to the input carrier image.
         * @param bit_depth The amount of significant bits to set during the embedding process.
         */
        LeastSignificantBit(const boost::filesystem::path &image_path, int bit_depth) : Steganography(image_path)
        {
            this->bit_depth = bit_depth;
            this->image_capacity = (this->image.rows * this->image.cols * this->image.channels()) * this -> bit_depth;
        }

        void Encode(const boost::filesystem::path &);
        void Decode();

    private:
        /**
         * @property bit_depth
         * The amount of least significant bits to set during the embedding process.
         */
        int bit_depth;

        /**
         * @property image_capacity
         * The total capacity of the carrier image in bits.
         */
        int image_capacity;

        void EncodeChunk(const int &, const std::vector<unsigned char> &);
        void EncodeChunkLength(const int &, const unsigned int &);

        std::vector<unsigned char> DecodeChunk(const int &, const int &);
        unsigned int DecodeChunkLength(const int &);
};

#endif // LEAST_SIGNIFICANT_BIT_HPP
