include_directories(src)

add_library(flytrap
    STATIC
    src/transpose/writer.cpp
    src/transpose/reader.cpp
    src/trie/trie.cpp
    src/tree/tree.cpp
    src/utils/splitIterator.cpp
    src/utils/logger.c
    src/trie/stack.c
    src/trie/trie.c
)

add_custom_target(test
    COMMAND cd ../../lib/test && make check
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
	set(CMAKE_C_FLAGS "/W4 /WX")
	set(CMAKE_CXX_FLAGS "/W4 /WX")
else()
	set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -std=c11 -pedantic -O3")
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -std=c++17 -pedantic -O3")
endif()

check_type_size("uintmax_t" MAX_UINT_SIZE)
if(${MAX_UINT_SIZE} LESS 2)
	message(FATAL_ERROR "Cannot run on 8-bit systems")
endif()
