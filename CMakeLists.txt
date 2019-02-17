cmake_minimum_required(VERSION 3.0)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

file(DOWNLOAD https://raw.githubusercontent.com/myint/optparse/v1.0/optparse.h include/optparse.hpp)

if (WITH_TESTS)
    file(DOWNLOAD https://raw.githubusercontent.com/catchorg/Catch2/v2.6.0/single_include/catch2/catch.hpp include/catch.hpp)
endif()

include_directories(include)

set(SOURCE_FILES
    src/least_significant_bit.cpp
    src/discrete_cosine_transform.cpp
    src/steganography.cpp
)

set(TEST_FILES
    # TODO - Add some unit testing
)

add_executable(steganography src/main.cpp ${SOURCE_FILES})
add_executable(steganography-testing test/main.cpp ${SOURCE_FILES} ${TEST_FILES})

find_package(Boost REQUIRED filesystem)
find_package(OpenCV REQUIRED)

target_link_libraries(steganography ${Boost_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(steganography-testing ${Boost_LIBRARIES} ${OpenCV_LIBS})
