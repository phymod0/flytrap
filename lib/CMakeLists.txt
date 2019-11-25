include_directories(src)

add_library(flytrap
    STATIC
    src/transpose/writer.cpp
    src/transpose/reader.cpp
)
