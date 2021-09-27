cmake_minimum_required(VERSION 3.20)
project(ImageFC)

set(CMAKE_CXX_STANDARD 20)

add_library(
        ImageFC SHARED
        src/lib.cpp
        src/hog/HOGEncoder.cpp
        src/color/rgbHist.cpp
        src/flow/mcmf.cpp
)

add_executable(examples examples/example.cpp)
target_link_libraries(examples ImageFC)
