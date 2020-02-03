include_directories(src)

add_library(flytrap
    STATIC
    src/utils/logger.c
    src/adt/trie/stack.c
    src/adt/trie/trie.c
)

add_custom_target(test
    COMMAND cd ../../lib/test && make check
)

set(CMAKE_C_STANDARD 99)
if(MSVC)
	set(CMAKE_C_FLAGS "/W4 /WX")
else()
	set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -std=c99 -pedantic -O3")
endif()
