# This file is a part of "Steganography" a C++ steganography tool.
#
# Copyright (C) 2019 James Lee <jamesl33info@gmail.com>.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TARGET = steganography

CC=g++
CFLAGS = -std=c++11 $(shell pkg-config --cflags opencv)

LINKER = g++
LFLAGS = $(shell pkg-config --libs opencv) -lboost_filesystem -lboost_system

SRC_EXT = cpp
INC_EXT = hpp

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SRC_LST = $(wildcard $(SRC_DIR)/*.$(SRC_EXT))
INC_LST = $(wildcard $(SRC_DIR)/*.$(INC_EXT))
OBJ_LST = $(SRC_LST:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD_DIR)/%.o)

CURL = curl --silent --create-dirs
ECHO = echo
MKDIR = mkdir --parent
RM = rm -rf

$(BIN_DIR)/$(TARGET) : $(OBJ_LST)
	@$(MKDIR) $(BIN_DIR)
	@$(LINKER) $(OBJ_LST) $(LFLAGS) -o $@
	@$(ECHO) "Successfully linked '"$@"'"

$(OBJ_LST) : $(BUILD_DIR)/%.o : $(SRC_DIR)/%.$(SRC_EXT) $(INC_LST)
	@$(MKDIR) $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(ECHO) "Successfully compiled '"$<"' to '"$@"'"

.PHONY: configure
configure:
	@$(CURL) https://raw.githubusercontent.com/myint/optparse/v1.0/optparse.h --output $(SRC_DIR)/optparse.hpp

.PHONY: clean
clean:
	@$(RM) $(OBJ_LST)
	@$(ECHO) "Successfully cleaned object files"

.PHONY: remove
remove: clean
	@$(RM) $(BUILD_DIR)
	@$(RM) $(BIN_DIR)
	@$(ECHO) "Successfully removed temporary directories"

.PHONY: purge
purge: remove
	@$(RM) $(SRC_DIR)/optparse.hpp
	@$(ECHO) "Successfully removed external libraries"
