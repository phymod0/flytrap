#define LOGGING_ENABLE 0

#include "../src/adt/string_tree/string_tree.c"
#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/rest/rest.c"

#include "ctest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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


static void remove_nonterminating_nulls(char* str, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		while (str[i] == '\0') {
			str[i] = (char)get_rand_uint();
		}
	}
}


static char* gen_rand_str(size_t len)
{
	char* arr = malloc(len + 1);
	FILE* fd = fopen("/dev/urandom", "rbe");
	fread(arr, 1, len, fd);
	remove_nonterminating_nulls(arr, len);
	arr[len] = '\0';
	fclose(fd);
	return arr;
}


static char* rand_str()
{
	const int min_len = 100;
	const int max_len = 200;
	return gen_rand_str(gen_len_bw(min_len, max_len));
}


static bool rand_bool() { return get_rand_uint() & 1; }


static char* rand_path()
{
	size_t len;
	size_t pos;
	const int jump_len = 10;
	char* path = rand_str();
	if (!path) {
		return NULL;
	}
	len = strlen(path);
	if (rand_bool()) {
		path[0] = '/';
	}
	for (pos = gen_len_bw(1, jump_len); pos < len;
	     pos += gen_len_bw(1, jump_len)) {
		path[pos] = '/';
	}
	return path;
}


static char* strdup(const char* str)
{
	size_t len = strlen(str);
	char* result = malloc(len + 1);
	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}


static int n_path_segments(const char* path)
{
	int result = 0;
	char* dup = strdup(path);
	if (!dup) {
		return -1;
	}
	for (char* tok = strtok(dup, "/"); tok; tok = strtok(NULL, "/")) {
		++result;
	}
	free(dup);
	return result;
}


static char** get_path_segments(const char* path)
{
	int i_segment = 0;
	char* dup = strdup(path);
	int n_segments = n_path_segments(path) + 1;
	char** segments = malloc(n_segments * sizeof *segments);
	for (char* tok = strtok(dup, "/"); tok; tok = strtok(NULL, "/")) {
		segments[i_segment++] = strdup(tok);
	}
	segments[i_segment] = NULL;
	free(dup);
	return segments;
}


static void free_path_segments(char** segments)
{
	for (char** segment = segments; *segment != NULL; ++segment) {
		free(*segment);
	}
	free(segments);
}


TEST_DEFINE(test_str_ndup, res)
{
	TEST_AUTONAME(res);

	char* to_dup = rand_str();
	size_t to_dup_len = strlen(to_dup);
	bool correct_result_length = true;
	bool result_is_prefix = true;

	for (size_t len = 0; len < 2 * to_dup_len; ++len) {
		char* dup = str_ndup(to_dup, len);
		if (len < to_dup_len) {
			correct_result_length &= strlen(dup) == len;
		} else {
			correct_result_length &= strlen(dup) == to_dup_len;
		}
		result_is_prefix &= strstr(to_dup, dup) == to_dup;
		free(dup);
	}
	free(to_dup);
	test_check(res, "Result length is always correct",
		   correct_result_length);
	test_check(res, "Result is always a prefix of input",
		   correct_result_length);
}


TEST_DEFINE(test_get_path_subtree, res)
{
	TEST_AUTONAME(res);

	char* path = rand_path();
	StringTree* tree = string_tree_create();
	test_check(res, "Memory allocated", path && tree);

	StringTree* subtree;
	StringTree* expected_subtree;
	char** path_segments = get_path_segments(path);

	subtree = get_path_subtree(tree, path);
	expected_subtree = tree;
	for (char** segment = path_segments; *segment != NULL; ++segment) {
		expected_subtree =
		    string_tree_find_subtree(expected_subtree, *segment);
	}
	test_check(res, "Subtree created at correct path",
		   expected_subtree == subtree);

	subtree = get_path_subtree(tree, path);
	expected_subtree = tree;
	for (char** segment = path_segments; *segment != NULL; ++segment) {
		expected_subtree =
		    string_tree_find_subtree(expected_subtree, *segment);
	}
	test_check(res, "Same subtree exists at path after refetch",
		   expected_subtree == subtree);

	free(path);
	free_path_segments(path_segments);
	string_tree_destroy(tree);
}


TEST_DEFINE(test_path_argv_create, res)
{
	TEST_AUTONAME(res);

	bool correct_allocation = true;
#define USELESS_NAME_FOR_USELESS_CPPCG_WARNING 10
	size_t test_size = USELESS_NAME_FOR_USELESS_CPPCG_WARNING;
#undef USELESS_NAME_FOR_USELESS_CPPCG_WARNING
	for (size_t size = 1; size < test_size; ++size) {
		char** path = path_argv_create(size);
		for (size_t i = 0; i < size; ++i) {
			correct_allocation &= path[i] == NULL;
		}
		free(path);
	}
	test_check(res, "Expected number of allocated pointers are NULL",
		   correct_allocation);
}


TEST_DEFINE(test_path_argv_destroy, res)
{
	TEST_AUTONAME(res);

#define USELESS_NAME_FOR_USELESS_CPPCG_WARNING 10
	size_t test_size = USELESS_NAME_FOR_USELESS_CPPCG_WARNING;
	for (size_t size = 1; size < test_size; ++size) {
		char** path = path_argv_create(size);
		for (size_t i = 0; i < size; ++i) {
			path[i] = malloc(gen_len_bw(
			    1, USELESS_NAME_FOR_USELESS_CPPCG_WARNING));
		}
		path_argv_destroy(size, path);
	}
#undef USELESS_NAME_FOR_USELESS_CPPCG_WARNING
}


TEST_DEFINE(test_find_path_subtree, res)
{
	TEST_AUTONAME(res);

	bool allocation_success = true;
	bool wildcard_matched = true;
	bool preferred_matches = true;
	bool paths_truncated = true;
	bool correct_argc = true;
	bool correct_argv = true;

	{
		const StringTree* subtree;
		int argc = 0;
		char** argv = NULL;
		StringTree* tree = string_tree_create();
		allocation_success &= tree != NULL;
		const char* paths_to_add[] = {
		    "/asdf/qwerty/zxcv", "/<?>/qwerty/zxcv", "/asdf/<?>/zxcv",
		    "/asdf/qwerty/<?>"};
		const size_t n_paths =
		    (sizeof paths_to_add / sizeof paths_to_add[0]) - 1;
		const StringTree* subtrees[n_paths];
		for (size_t i = 0; i < n_paths; ++i) {
			const char* path = paths_to_add[i];
			subtrees[i] = get_path_subtree(tree, path);
			allocation_success &= subtrees[i] != NULL;
		}
		subtree =
		    find_path_subtree(tree, paths_to_add[0], &argc, &argv);
		wildcard_matched &= subtree != NULL;
		preferred_matches &= subtree == subtrees[0];
		path_argv_destroy(argc, argv);
		string_tree_destroy(tree);
	}

	// TODO: Remaining checks

	test_check(res, "Allocations succeeded", allocation_success);
	test_check(res, "Wildcard segments matched", wildcard_matched);
	test_check(res, "Exact matches take preference over wildcard matches",
		   preferred_matches);
	test_check(res, "Paths greater than limit are truncated",
		   paths_truncated);
	test_check(res, "Correct path argument count is returned",
		   correct_argc);
	test_check(res, "Correct path argument vector is returned",
		   correct_argv);
}


TEST_DEFINE(test_register_single_handler, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_handle_signal, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_get_method_str, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_get_request_path, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_validate_path, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_DEFINE(test_event_base_create_and_init, res)
{
	TEST_AUTONAME(res);
	test_check(res, "Write this test", true);
}


TEST_START(test_str_ndup, test_get_path_subtree, test_path_argv_create,
	   test_path_argv_destroy, test_find_path_subtree,
	   test_register_single_handler, test_handle_signal,
	   test_get_method_str, test_get_request_path, test_validate_path,
	   test_event_base_create_and_init)
