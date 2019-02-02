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

#include <string>
#include <optparse.hpp>
#include "least_significant_bit.hpp"

void help(optparse::OptionParser parser, std::string command) {
    if (command == "help") {
        std::cout << parser.format_help() << std::endl;
    }
    else if (command == "en" || command == "encode") {
        std::cout << "Usage: encode [options] image payload" << std::endl;
        std::cout << std::endl << "Options:" << std::endl << parser.format_option_help();
    } else if (command == "de" || command == "decode") {
        std::cout << "Usage: decode [options] image" << std::endl;
        std::cout << std::endl << "Options:" << std::endl << parser.format_option_help();
    }
}

int main(int argc, char **argv) {
    optparse::OptionParser parser = optparse::OptionParser()
        .usage("%prog [options] <command> [arguments]\n\n"
               "where <command> is one of:\n\n"
               "\tencode (en) - Encode a file into a carrier image\n"
               "\tdecode (de) - Decode a file from a carrier image\n\n"
               "Use \"%prog help <command>\" for help on a specific command");

    parser.add_option("-d", "--depth")
        .help("encode/decode depth, excepts values between 1 and 8")
        .type("int")
        .set_default(1);

    const optparse::Values options = parser.parse_args(argc, argv);
    const std::vector<std::string> arguments = parser.args();

    if (arguments.size() == 0) {
        std::cout << parser.format_help() << std::endl;
        exit(0);
    }

    if (int(options.get("depth")) < 1 || int(options.get("depth")) > 8) {
        std::cerr << "Error: Invalid depth value" << std::endl;
        exit(1);
    }

    if (arguments[0] == "help") {
        if (arguments.size() == 2) {
            help(parser, arguments[1]);
        } else {
            help(parser, "help");
        }
    } else if (arguments[0] == "en" || arguments[0] == "encode") {
        if (arguments.size() == 3) {
            LeastSignificantBit lsb = LeastSignificantBit(arguments[1], options.get("depth"));
            lsb.Encode(arguments[2]);
        } else {
            help(parser, "encode");
        }
    } else if (arguments[0] == "de" || arguments[0] == "decode") {
        if (arguments.size() == 2) {
            LeastSignificantBit lsb = LeastSignificantBit(arguments[1], options.get("depth"));
            lsb.Decode();
        } else {
            help(parser, "decode");
        }
    }
}
