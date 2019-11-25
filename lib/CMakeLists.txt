include_directories(src)

add_library(flytrap
    STATIC
    src/transpose/writer.cpp
    src/transpose/reader.cpp
)

add_custom_target(test
    COMMAND cd ../../lib/test && make check
)
