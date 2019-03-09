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

#include <fstream>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "exceptions.hpp"

#ifndef STEGANOGRAPHY_HPP
#define STEGANOGRAPHY_HPP

/**
 * Abstract base class which each steganography technique will extend.
 */
class Steganography
{
    public:
        /**
         * Default constructor for Steganography class, will be overridden for
         * different steganography techniques.
         * @param image_path The path to the input carrier image.
         */
        explicit Steganography(const boost::filesystem::path &image_path)
        {
            this->image_path = image_path;
            this->image = cv::imread(image_path.string(), cv::IMREAD_UNCHANGED);

            if (!this->image.data)
            {
                throw ImageException("Error: Failed to open input image");
            }
        }

        /**
         * @pure Encode
         * Function that must be overridden by the subclass which encodes a payload
         * into the carrier image using the steganographic technique defined in the
         * subclass.
         */
        virtual void Encode(const boost::filesystem::path &) = 0;

        /**
         * @pure Decode
         * Function that must be overridden by the subclass which decodes a payload
         * from the carrier image using the steganographic technique defined in the
         * subclass.
         */
        virtual void Decode() = 0;

    protected:
        /**
         * @property image_path
         * The path to the carrier image stored on disk. This image will not be
         * modified.
         */
        boost::filesystem::path image_path;

        /**
         * @property image
         * An in memory copy of the input image in which we will embed the data.
         */
        cv::Mat image;

        std::vector<unsigned char> ReadPayload(const boost::filesystem::path &);
        void WritePayload(const boost::filesystem::path &, const std::vector<unsigned char> &);

        /**
         * Set the n'th significant bit of a generic type.
         *
         * This function is designed to set the n'th significant bit of integer/char types.
         * It assumes that it is being used correctly; there is no incorrect input detection.
         *
         * @tparam target "Any" type which works with bytewise operators, the target to set the bit of.
         * @param bit Which bit to set in the target.
         * @param value The value the target bit will be set too.
         */
        template <class T>
        inline void SetBit(T *target, const int &bit, const int &value)
        {
            *target ^= (-(unsigned int)value ^ *target) & (1UL << bit);
        }

        /**
         * Get the n'th significant bit of a generic type.
         *
         * This function is designed to get the n'th significant bit of integer/char types.
         * It assumes that it is being used correctly; there is no incorrect input detection.
         *
         * @tparam target "Any" type which works with bytewise operators, the target to set the but of.
         * @param bit Which bit to get in the target.
         * @return The n'th significant bit.
         */
        template <class T>
        inline int GetBit(const T &target, const int &bit)
        {
            return (target >> bit) & 1UL;
        }
};

#endif // STEGANOGRAPHY_HPP
