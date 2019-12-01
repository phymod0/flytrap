cmake_minimum_required (VERSION 3.8)
INCLUDE(CheckTypeSize)

project(flytrap)

add_subdirectory(lib)

set(CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
