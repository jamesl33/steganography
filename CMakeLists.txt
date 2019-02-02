cmake_minimum_required(VERSION 3.0)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(DOWNLOAD https://raw.githubusercontent.com/myint/optparse/v1.0/optparse.h include/optparse.hpp)

include_directories(include)

file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp")

add_executable(steganography ${SOURCE_FILES})

find_package(Boost REQUIRED filesystem)
find_package(OpenCV REQUIRED)

target_link_libraries(steganography ${Boost_LIBRARIES} ${OpenCV_LIBS})
