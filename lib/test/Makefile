INCLUDE_DIRS := ../src/ ../
CFLAGS := -g -Wall -Werror -Wextra -std=c++17 -fsanitize=address \
	-fsanitize=undefined -Winline # -O3 -finline-functions -Winline
CFLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))

SRC_FILES := $(wildcard test*.cpp)
TESTS := $(patsubst %.cpp,%,$(SRC_FILES))
EXEC := $(patsubst %.cpp,.__test_exec_%,$(SRC_FILES))

compile: $(TESTS)
check: $(EXEC) clean
clean:
	@rm -rf $(TESTS)
.DELETE_ON_ERROR:
$(EXEC):
	@$(CXX) $(CFLAGS) ctest.c $(patsubst .__test_exec_%,%.cpp,$@) -o $@
	@./$@
	@rm -rf $@
$(TESTS):
	@$(CXX) $(CFLAGS) ctest.c $@.c -o $@
