include_directories(src)

add_library(flytrap
    STATIC
    src/transpose/writer.cpp
    src/transpose/reader.cpp
)

add_custom_target(test
    COMMAND cd ../../lib/test && make check
)

check_type_size("uintmax_t" MAX_UINT_SIZE)
if(${MAX_UINT_SIZE} LESS 2)
	message(FATAL_ERROR "Cannot run on 8-bit systems")
endif()
