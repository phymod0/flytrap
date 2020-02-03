cmake_minimum_required (VERSION 3.8)
INCLUDE(CheckTypeSize)

project(flytrap LANGUAGES C)

add_subdirectory(lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
