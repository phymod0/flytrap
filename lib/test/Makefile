INCLUDE_DIRS := ../src/ ../
FLAGS := -g -Wall -Werror -Wextra -fsanitize=address -fsanitize=undefined \
		-Winline # -O3 -finline-functions -Winline
FLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))

C_FLAGS := $(FLAGS) -std=c11
CXX_FLAGS := $(FLAGS) -std=c++17

C_SRC_FILES := $(wildcard test*.c)
C_TESTS := $(patsubst %.c,%,$(C_SRC_FILES))
C_EXEC := $(patsubst %.c,.__test_exec_%,$(C_SRC_FILES))

CXX_SRC_FILES := $(wildcard test*.cpp)
CXX_TESTS := $(patsubst %.cpp,%,$(CXX_SRC_FILES))
CXX_EXEC := $(patsubst %.cpp,.__test_exec_%,$(CXX_SRC_FILES))

TESTS := $(C_TESTS) $(CXX_TESTS)
EXEC := $(C_EXEC) $(CXX_EXEC)

compile: $(TESTS)
check: $(EXEC) clean
clean:
	@rm -rf $(TESTS)

.DELETE_ON_ERROR:
$(C_EXEC):
	@$(CC) $(C_FLAGS) ctest.c $(patsubst .__test_exec_%,%.c,$@) -o $@
	@./$@
	@rm -rf $@
.DELETE_ON_ERROR:
$(CXX_EXEC):
	@$(CXX) $(CXX_FLAGS) ctest.c $(patsubst .__test_exec_%,%.cpp,$@) -o $@
	@./$@
	@rm -rf $@

$(C_TESTS):
	@$(CC) $(C_FLAGS) ctest.c $@.c -o $@
$(CXX_TESTS):
	@$(CXX) $(CXX_FLAGS) ctest.c $@.c -o $@
