cmake_minimum_required(VERSION 3.14)
project(CPU)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Algorithm)
include_directories(Common)

add_executable(CPU main.cpp)
