#include "../src/adt/trie/stack.c"

#define WITHOUT_CTEST_NAMESPACE
#include "ctest.h"

#include <stdio.h>


static size_t get_rand_uint()
{
	size_t random;
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(&random, sizeof random, 1, fd);
	fclose(fd);
	return random;
}


DEFINE_TEST(test_stack_create)
{
	DESCRIBE_TEST("Test for empty stack at creation");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(initially_empty, "Stack initially empty");
	DEFINE_CHECK(pop_top_null, "Pop and top initially return NULL");

	Stack* s = stack_create(STACK_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, s != NULL);

	CHECK_INCLUDE(initially_empty, stack_empty(s));
	CHECK_INCLUDE(pop_top_null, !stack_pop(s) && !stack_pop(s) &&
					!stack_pop(s) && !stack_top(s) &&
					!stack_top(s));

	stack_destroy(s);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(initially_empty);
	ASSERT_CHECK(pop_top_null);
}


DEFINE_TEST(test_stack_pushpop)
{
	DESCRIBE_TEST("Test for stack pushing/popping");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(pop_in_reverse, "Stack popped in reverse push order");
	DEFINE_CHECK(empty_on_remove, "Stack empty after full removal");

	Stack* s = stack_create(STACK_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, s != NULL);
	const char* message = "racecar Hello world! wasitacatisaw";
	const char* reverse = "wasitacatisaw !dlrow olleH racecar";

	bool match = true;
	for (; *message; ++message) {
		char* dup = malloc(1);
		*dup = *message;
		stack_push(s, dup);
	}
	for (; *reverse; ++reverse) {
		char* c = stack_pop(s);
		if (*c != *reverse) {
			match = false;
			free(c);
			break;
		}
		free(c);
	}
	CHECK_INCLUDE(pop_in_reverse, match);
	CHECK_INCLUDE(empty_on_remove, stack_empty(s));

	stack_destroy(s);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(pop_in_reverse);
	ASSERT_CHECK(empty_on_remove);
}


DEFINE_TEST(test_stack_top)
{
	DESCRIBE_TEST("Test for stack peeking");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(correct_on_push, "Top correct while pushing");
	DEFINE_CHECK(correct_on_pop, "Top correct while popping");

	Stack* s = stack_create(STACK_OPS_FREE);
	CHECK_INCLUDE(mem_alloc, s != NULL);
	const char* message = "Hello world!";

	bool match = true;
	for (; *message; ++message) {
		char* dup = malloc(1);
		*dup = *message;
		stack_push(s, dup);
		if (!stack_top(s) || *(char*)stack_top(s) != *message) {
			match = false;
		}
	}
	CHECK_INCLUDE(correct_on_push, match);

	match = true;
	message = "!dlrow olleH";
	for (; *message; ++message) {
		char* c;
		for (int i = 0; i < 4; ++i) {
			c = stack_top(s);
		}
		if (!c || *c != *message) {
			match = false;
			free(c);
			break;
		}
		free(c);
		stack_pop(s);
	}
	CHECK_INCLUDE(correct_on_pop, match);

	stack_destroy(s);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(correct_on_push);
	ASSERT_CHECK(correct_on_pop);
}


DEFINE_TEST(asan_test_stack_destroy)
{
	DESCRIBE_TEST("Test for stack destruction");

	Stack* s = stack_create(STACK_OPS_FREE);

#define THOUSAND 1000
	size_t n_push = (get_rand_uint() % THOUSAND) + THOUSAND;
#undef THOUSAND
	for (size_t i = 0; i < n_push; ++i) { // NOLINT
		int* r = malloc(sizeof(int));
		stack_push(s, r);
	}

	stack_destroy(s);
}


START(test_stack_create, test_stack_pushpop, test_stack_top,
      asan_test_stack_destroy)
