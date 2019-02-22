Steganography
-------------
Steganography is a C++ steganography tool which leverages the LSB and DCT
embedding techniques.

Dependencies
------------
- [OpenCV](https://opencv.org/)
- [Boost C++ Libraries](https://www.boost.org/)

Building
--------
The steganography executable is build using the CMake build system and requires
all of the libraries listed above in the dependencies section.

```sh
# Configure the build
cmake -B build

# Build steganography with the release optimisations
cmake --build build --config Release --target steganography

# Run the executable
./bin/steganography
```

Testing
-------
Unit testing is provided to determine if the application is working correctly,
the testing uses [Catch2](https://github.com/catchorg/Catch2) and can be run by
building and running the steganography-testing executable.

```sh
# Configure the build
cmake -B build

# Build steganography-testing with the release optimisations
cmake --build build --config Release --target steganography-testing

# Run the unit testing
./bin/steganography-testing
```

Usage
-----
A command line user interface is provided to allow the encoding/decoding of
data using either the least significant bit (LSB) or discrete cosine transform
(DCT) technique.

```sh
# Encode using the DCT technique
steganography encode --technique dct payload carrier

# Decode using the DCT technique
steganography decode --technique dct carrier

# Encode using the LSB technique
steganography encode --technique lsb payload carrier

# Decode using the LSB technique
steganography decode --technique lsb carrier
```

Documentation
-------------
Full documentation generated using [Doxygen](http://www.doxygen.nl/) is
available through GitHub
[Pages](https://github.coventry.ac.uk/pages/leej64/steganography/).

License
-------
Copyright (C) 2019 James Lee <jamesl33info@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
