#define LOGGING_ENABLE 0

#include "../src/adt/string_tree/string_tree.c"
#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/rest/rest.c"

#include "ctest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if 0
static size_t get_rand_uint()
{
	size_t random;
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(&random, sizeof random, 1, fd);
	fclose(fd);
	return random;
}


static inline size_t gen_len_bw(size_t min, size_t max)
{
	return (size_t)((get_rand_uint() % (max - min + 1)) + min);
}


static char* gen_rand_str(size_t len)
{
	char* arr = malloc(len + 1);
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(arr, 1, len, fd);
	arr[len] = '\0';
	fclose(fd);
	return arr;
}
#endif


TEST_DEFINE(test_str_ndup, res)
{
	TEST_AUTONAME(res);

	const char* to_dup = "Hello world!";
	size_t to_dup_len = strlen(to_dup);
	bool correct_result_length = true;
	bool result_is_prefix = true;

	for (size_t len = 0; len < 2 * to_dup_len; ++len) {
		char* dup = str_ndup("Hello world!", len);
		if (len < to_dup_len) {
			correct_result_length &= strlen(dup) == len;
		} else {
			correct_result_length &= strlen(dup) == to_dup_len;
		}
		result_is_prefix &= strstr(to_dup, dup) == to_dup;
		free(dup);
	}
	test_check(res, "Result length is always correct",
		   correct_result_length);
	test_check(res, "Result is always a prefix of input",
		   correct_result_length);
}


TEST_DEFINE(test_get_path_subtree, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_path_argv_create, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_path_argv_destroy, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_find_path_subtree, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_START(test_str_ndup, test_get_path_subtree, test_path_argv_create,
	   test_path_argv_destroy, test_find_path_subtree)
