cmake_minimum_required(VERSION 3.3)
project(kaleidoscope)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    doc/fibonacci.ks
    doc/garmmer.txt
    CMakeLists.txt
    LICENSE
    main.cpp
    README.md)

add_executable(kaleidoscope ${SOURCE_FILES})