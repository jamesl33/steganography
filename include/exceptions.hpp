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

#include <stdexcept>

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

class ImageException : public std::runtime_error
{
    public:
        /**
         * Default constructor for the ImageException class which is an
         * exception that is thrown when there is an issue processing an image.
         * @param message A detailed message explaining what occurred.
         */
        explicit ImageException(const std::string &message) : std::runtime_error(message) {};
};

class EncodeException : public std::runtime_error
{
    public:
        /**
         * Default constructor for the EncodeException class which is an
         * exception that is thrown when there is an issue during the encoding
         * process.
         * @param message A detailed message explaining what occurred.
         */
        explicit EncodeException(const std::string &message) : std::runtime_error(message) {};
};

class DecodeException : public std::runtime_error
{
    public:
        /**
         * Default constructor for the DecodeException class which is an
         * exception that is thrown when there is an issue during the decoding
         * process.
         * @param message A detailed message explaining what occurred.
         */
        explicit DecodeException(const std::string &message) : std::runtime_error(message) {};
};

#endif // EXCEPTIONS_HPP
