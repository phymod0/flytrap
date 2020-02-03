INCLUDE_DIRS := ../src/ ../
FLAGS := -g -Wall -Werror -Wextra -fsanitize=address -fsanitize=undefined \
		-Winline # -O3 -finline-functions -Winline
FLAGS += $(patsubst %,-I%,$(INCLUDE_DIRS))

C_FLAGS := $(FLAGS) -std=c99

C_SRC_FILES := $(wildcard test*.c)
C_TESTS := $(patsubst %.c,%,$(C_SRC_FILES))
C_EXEC := $(patsubst %.c,.__test_exec_%,$(C_SRC_FILES))

TESTS := $(C_TESTS)
EXEC := $(C_EXEC)

compile: $(TESTS)
check: $(EXEC) clean
clean:
	@rm -rf $(TESTS)

.DELETE_ON_ERROR:
$(C_EXEC):
	@$(CC) $(C_FLAGS) ctest.c $(patsubst .__test_exec_%,%.c,$@) -o $@
	@./$@
	@rm -rf $@

$(C_TESTS):
	@$(CC) $(C_FLAGS) ctest.c $@.c -o $@
