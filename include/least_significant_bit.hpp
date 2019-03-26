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

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
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
         */
        LeastSignificantBit(const boost::filesystem::path &image_path) : Steganography(image_path)
        {
            this->image_capacity = (this->image.rows * this->image.cols * this->image.channels()) - 64;
        }

        /**
         * Encode the payload file into the carrier image by embedding into one or more
         * least significant bits.
         *
         * @param payload_path Path to the file we are encoding.
         * @exception EncodeException Thrown when encoding fails.
         */
        void Encode(const boost::filesystem::path &);

        /**
         * Decode the payload from the steganographic image by reading from one or more
         * least significant bits.
         */
        void Decode();

    private:
        /**
         * @property image_capacity
         * The total capacity of the carrier image in bits.
         */
        int image_capacity;

        /**
         * Encode a chunk of information into the carrier image.
         *
         * Before encoding a chunk of information you "should" first encode its length
         * using the EncodeChunkLength function.
         *
         * @param start The bit index to start encoding at.
         * @param it The position in the chunk of information to start encoding.
         * @param en The position in the chunk of information to stop encoding.
         */
        void EncodeChunk(const int &start, std::vector<unsigned char>::iterator it, std::vector<unsigned char>::iterator en);

        /**
         * Encode a 32bit integer stating the length of the following chunk into the
         * carrier image.
         *
         * @param start The bit index to start encoding at.
         * @param chunk_length The length of the next chunk in bytes.
         */
        void EncodeChunkLength(const int &start, const unsigned int &chunk);

        /**
         * Attempt to decode a chunk of information from the steganographic image.
         *
         * @param start The bit index to start decoding at.
         * @param end The bit index to stop decoding at.
         * @return The chunk of information read from the steganographic image.
         * @exception DecodeException Thrown when decoding fails.
         */
        std::vector<unsigned char> DecodeChunk(const int &start, const int &end);

        /**
         * Attempt to decode the 32bit integer stating the length of the following
         * chunk.
         *
         * @param start The bit index to start decoding at.
         * @return The length of the following chunk.
         * @exception DecodeException Thrown when decoding fails.
         */
        unsigned int DecodeChunkLength(const int &start);
};

#endif // LEAST_SIGNIFICANT_BIT_HPP
