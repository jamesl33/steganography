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
#include <tuple>
#include <opencv2/imgproc/imgproc.hpp>
#include "steganography.hpp"
#include "exceptions.hpp"

#ifndef DISCRETE_COSINE_TRANSFORM_HPP
#define DISCRETE_COSINE_TRANSFORM_HPP

class DiscreteCosineTransform : public Steganography
{
    public:
        /**
         * Default constructor for the DiscreteCosineTransform class which overrides
         * the default constructor from the Steganography class.
         * @param image_path The path to the input carrier image.
         * @param swap_count The amount of DCT coefficients to swap.
         * @param persistence The persistence value for this instance.
         */
        explicit DiscreteCosineTransform(const boost::filesystem::path &image_path, int swap_count, int persistence) : Steganography(image_path)
        {
            this->swap_count = swap_count;
            this->persistence = persistence;
            this->image_capacity = ((this->image.rows - 8) / 8) * ((this->image.cols - 8) / 8) * this->swap_count;
        }

        void Encode(const boost::filesystem::path &);
        void Decode();

    private:
        /**
         * @property swap_count
         * How many DCT coefficients to swap. A higher swap count allows for higher
         * storage capacity, however, will cause more visual degradation.
         */
        int swap_count;

        /**
         * @property persistence
         * Value which will be applied during the DCT coefficient swapping. Higher
         * values ensure that the data persists, however, cause more visual
         * degradation.
         */
        int persistence;

        /**
         * @property
         * The total capacity of the carrier image in bits.
         */
        int image_capacity;

        void EncodeChunk(const int &, const std::vector<unsigned char> &);
        void EncodeChunkLength(const int &, const unsigned int &);

        std::vector<unsigned char> DecodeChunk(const int &, const int &);
        unsigned int DecodeChunkLength(const int &);

        void SwapCoefficients(cv::Mat *, const int &, const std::tuple<int, int> &, const std::tuple<int, int> &);
};

#endif // DISCRETE_COSINE_TRANSFORM_HPP
