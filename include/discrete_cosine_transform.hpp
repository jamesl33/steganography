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
#include <opencv2/imgproc/imgproc.hpp>
#include "steganography.hpp"

#ifndef DISCRETE_COSINE_TRANSFORM_HPP
#define DISCRETE_COSINE_TRANSFORM_HPP

class DiscreteCosineTransform : public Steganography {
    public:
        explicit DiscreteCosineTransform(const boost::filesystem::path& image_path, int persistence) : Steganography(image_path) {
            this -> persistence = persistence;
        }

        void Encode(const boost::filesystem::path&);
        void Decode();
    private:
        int persistence;

        void EncodeChunk(const int&, const std::vector<unsigned char>&);
        void EncodeChunkLength(const int&, const unsigned int&);

        std::vector<unsigned char> DecodeChunk(const int&, const int&);
        unsigned int DecodeChunkLength(const int&);
};

#endif // DISCRETE_COSINE_TRANSFORM_HPP
