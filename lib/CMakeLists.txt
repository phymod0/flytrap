include_directories(src)

add_library(flytrap
    STATIC
    src/transpose/writer.cpp
    src/transpose/reader.cpp
    src/adt/trie/trie.cpp
    src/wrappers/libevent/buffer.cpp
    src/wrappers/libevent/request.cpp
    src/wrappers/libevent/eventBase.cpp
    src/wrappers/libevent/http.cpp
    src/utils/logger.c
    src/adt/trie/stack.c
    src/adt/trie/trie.c
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
