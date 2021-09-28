cmake_minimum_required(VERSION 3.20)
project(ImageFC)

set(CMAKE_CXX_STANDARD 11)

add_library(
        ImageFC SHARED
        src/lib.cpp
        src/hog/HOGEncoder.cpp
        src/color/rgbHist.cpp
        src/flow/mcmf.cpp
)
add_executable(simple examples/simple.cpp)
target_link_libraries(simple ImageFC)

add_executable(batch examples/batch.cpp)
target_link_libraries(batch ImageFC)