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
#include <string>
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

            // Convert the image to floating point and split the channels
            this->image.convertTo(this->image, CV_32F);
            cv::split(this->image, this->channels);
        }

        /**
         * Encode the payload file into the carrier image by swapping DCT coefficients.
         *
         * @param payload_path Path to the file we are encoding.
         * @exception EncodeException thrown when encoding fails.
         */
        void Encode(const boost::filesystem::path &);

        /**
         * Decode the payload from the steganographic image by comparing DCT
         * coefficients.
         */
        void Decode();

    private:
        /**
         * @property
         * The color channels from the carrier image, used in the embedding
         * process.
         */
        std::vector<cv::Mat> channels;

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

        /**
         * Encode a chunk of information into the carrier image.
         *
         * Before encoding a chunk of information you "should" first encode its length
         * using the EncodeChunkLength function.
         *
         * @param start The bit index to start encoding at.
         * @param chunk The chunk of information which will be encoded.
         */
        void EncodeChunk(const int &, const std::vector<unsigned char> &);

        /**
         * Encode a 32bit integer stating the length of the following chunk into the
         * carrier image.
         *
         * @param start The bit index to start encoding at.
         * @param chunk_length The length of the next chunk in bytes.
         */
        void EncodeChunkLength(const int &, const unsigned int &);

        /**
         * Attempt to decode a chunk of information from the steganographic image.
         *
         * @param start The bit index to start decoding at.
         * @param end The bit index to stop decoding at.
         * @return The chunk of information read from the steganographic image.
         * @exception DecodeException Thrown when decoding fails.
         */
        std::vector<unsigned char> DecodeChunk(const int &, const int &);

        /**
         * Attempt to decode the 32bit integer stating the length of the following
         * chunk.
         *
         * @param start The bit index to start decoding at.
         * @return The length of the following chunk.
         * @exception DecodeException Thrown when decoding fails.
         */
        unsigned int DecodeChunkLength(const int &);

        /**
         * Swap two DCT coefficients.
         *
         * Swap two DCT coefficients and apply a persistence value to ensure that the
         * data survives the compression process.
         *
         * @param block A pointer to the block which is currently be operated on.
         * @param value The value which is being stored, will be 0 or 1.
         */
        void SwapCoefficients(cv::Mat *, const int &, const std::tuple<int, int> &, const std::tuple<int, int> &);
};

#endif // DISCRETE_COSINE_TRANSFORM_HPP
