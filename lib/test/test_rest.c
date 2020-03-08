#define LOGGING_ENABLE 0

#include "../src/adt/string_tree/string_tree.c"
#include "../src/adt/trie/stack.c"
#include "../src/adt/trie/trie.c"
#include "../src/config.h"
#include "../src/rest/rest.c"

#define WITHOUT_CTEST_NAMESPACE
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


static char* rand_path_with_len(size_t len)
{
	size_t pos;
	const int jump_len = 10;
	char* path = gen_rand_str(len);
	if (!path) {
		return NULL;
	}
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


static bool verify_path_in_tree(StringTree* tree, const char* path)
{
	char** segments = get_path_segments(path);
	for (char** segment = segments; *segment; ++segment) {
		if (!tree) {
			free_path_segments(segments);
			return false;
		}
		tree = string_tree_find_subtree(tree, *segment);
	}
	free_path_segments(segments);
	return tree != NULL;
}


DEFINE_TEST(test_str_ndup)
{
	DESCRIBE_TEST("Unit test for str_ndup");

	DEFINE_CHECK(correct_result_length, "Result length is always correct");
	DEFINE_CHECK(result_is_prefix,
		     "Result is always a prefix of its input");

	char* to_dup = rand_str();
	size_t to_dup_len = strlen(to_dup);

	for (size_t len = 0; len < 2 * to_dup_len; ++len) {
		char* dup = str_ndup(to_dup, len);
		if (len < to_dup_len) {
			CHECK_INCLUDE(correct_result_length,
				      strlen(dup) == len);
		} else {
			CHECK_INCLUDE(correct_result_length,
				      strlen(dup) == to_dup_len);
		}
		CHECK_INCLUDE(result_is_prefix, strstr(to_dup, dup) == to_dup);
		free(dup);
	}
	free(to_dup);

	ASSERT_CHECK(correct_result_length);
	ASSERT_CHECK(result_is_prefix);
}


DEFINE_TEST(test_get_path_subtree)
{
	DESCRIBE_TEST("Unit test for get_path_subtree");

	DEFINE_CHECK(mem_alloc, "Memory successfully allocated");
	DEFINE_CHECK(subtree_path, "Subtree(s) created at correct path");
	DEFINE_CHECK(idempotent, "Calling more than once has no effect");

	char* path = rand_path();
	StringTree* tree = string_tree_create();
	CHECK_INCLUDE(mem_alloc, path != NULL && tree != NULL);

	StringTree* subtree;
	StringTree* expected_subtree;
	char** path_segments = get_path_segments(path);

	subtree = get_path_subtree(tree, path);
	expected_subtree = tree;
	for (char** segment = path_segments; *segment != NULL; ++segment) {
		expected_subtree =
		    string_tree_find_subtree(expected_subtree, *segment);
	}
	CHECK_INCLUDE(subtree_path, expected_subtree == subtree);

	for (int i = 0; i < 4; ++i) {
		subtree = get_path_subtree(tree, path);
		expected_subtree = tree;
		for (char** segment = path_segments; *segment != NULL;
		     ++segment) {
			expected_subtree = string_tree_find_subtree(
			    expected_subtree, *segment);
		}
		CHECK_INCLUDE(idempotent, expected_subtree == subtree);
	}

	free(path);
	free_path_segments(path_segments);
	string_tree_destroy(tree);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(subtree_path);
	ASSERT_CHECK(idempotent);
}


DEFINE_TEST(test_path_argv_create)
{
	DESCRIBE_TEST("Unit test for path_argv_create");

	DEFINE_CHECK(correct_allocation,
		     "Expected number of allocated pointers are NULL");

#define USELESS_NAME_FOR_USELESS_CPPCG_WARNING 10
	size_t test_size = USELESS_NAME_FOR_USELESS_CPPCG_WARNING;
#undef USELESS_NAME_FOR_USELESS_CPPCG_WARNING
	for (size_t size = 1; size < test_size; ++size) {
		char** path = path_argv_create();
		for (size_t i = 0; i < size; ++i) {
			CHECK_INCLUDE(correct_allocation, path[i] == NULL);
		}
		free(path);
	}

	ASSERT_CHECK(correct_allocation);
}


DEFINE_TEST(test_path_argv_destroy)
{
	DESCRIBE_TEST("Unit test for path_argv_destroy");

#define USELESS_NAME_FOR_USELESS_CPPCG_WARNING 10
	size_t test_size = USELESS_NAME_FOR_USELESS_CPPCG_WARNING;
	for (size_t size = 1; size < test_size; ++size) {
		char** path = path_argv_create();
		for (size_t i = 0; i < size; ++i) {
			path[i] = malloc(gen_len_bw(
			    1, USELESS_NAME_FOR_USELESS_CPPCG_WARNING));
		}
		path_argv_destroy(path);
	}
#undef USELESS_NAME_FOR_USELESS_CPPCG_WARNING
}


DEFINE_TEST(test_find_path_subtree)
{
	DESCRIBE_TEST("Unit test for find_path_subtree");

	DEFINE_CHECK(allocation_success, "Allocations succeeded");
	DEFINE_CHECK(wildcard_matched, "Wildcard segments matched");
	DEFINE_CHECK(preferred_matches,
		     "Exact matches take preference over wildcard matches");
	DEFINE_CHECK(paths_truncated,
		     "Only paths greater than the limit are truncated");
	DEFINE_CHECK(correct_argc, "Correct path argument count is returned");
	DEFINE_CHECK(correct_argv, "Correct path argument vector is returned");

	{ // Test wildcard matching
		const StringTree* subtree;
		int argc = 0;
		char** argv = NULL;
		StringTree* tree = string_tree_create();
		const char* paths_to_add[] = {
		    "/asdf/qwerty/zxcv",
		    "/" REST_PATH_SEGMENT_WILDCARD "/qwerty/zxcv",
		    "/asdf/" REST_PATH_SEGMENT_WILDCARD "/zxcv",
		    "/asdf/qwerty/" REST_PATH_SEGMENT_WILDCARD};
		const size_t n_paths =
		    (sizeof paths_to_add / sizeof paths_to_add[0]) - 1;
		const StringTree* subtrees[n_paths];

		CHECK_INCLUDE(allocation_success, tree != NULL);

		for (size_t i = 0; i < n_paths; ++i) {
			const char* path = paths_to_add[i];
			subtrees[i] = get_path_subtree(tree, path);
			CHECK_INCLUDE(allocation_success, subtrees[i] != NULL);
		}

		subtree =
		    find_path_subtree(tree, paths_to_add[0], &argc, &argv);
		path_argv_destroy(argv);

		CHECK_INCLUDE(wildcard_matched, subtree != NULL);
		CHECK_INCLUDE(preferred_matches, subtree == subtrees[0]);

		string_tree_destroy(tree);
	}

	{ // Test path truncation
		StringTree* tree = string_tree_create();
		char* good_path;
		char* bad_path;

		CHECK_INCLUDE(allocation_success, tree != NULL);

		good_path = rand_path_with_len(REST_PATH_MAXSZ);
		CHECK_INCLUDE(allocation_success,
			      get_path_subtree(tree, good_path) != NULL);
		CHECK_INCLUDE(paths_truncated,
			      verify_path_in_tree(tree, good_path));
		free(good_path);

		bad_path = rand_path_with_len(REST_PATH_MAXSZ * 2);
		CHECK_INCLUDE(allocation_success,
			      get_path_subtree(tree, bad_path) != NULL);
		CHECK_INCLUDE(paths_truncated,
			      !verify_path_in_tree(tree, bad_path));
		free(bad_path);

		string_tree_destroy(tree);
	}

	// TODO(phymod0): Remaining checks

	ASSERT_CHECK(allocation_success);
	ASSERT_CHECK(wildcard_matched);
	ASSERT_CHECK(preferred_matches);
	ASSERT_CHECK(paths_truncated);
	ASSERT_CHECK(correct_argc);
	ASSERT_CHECK(correct_argv);
}


DEFINE_TEST(test_register_single_handler)
{
	DESCRIBE_TEST("Unit test for register_single_handler");
}


DEFINE_TEST(test_handle_signal)
{
	DESCRIBE_TEST("Unit test for handle_signal");
}


DEFINE_TEST(test_get_method_str)
{
	DESCRIBE_TEST("Unit test for get_method_str");
}


DEFINE_TEST(test_get_request_path)
{
	DESCRIBE_TEST("Unit test for get_request_path");
}


DEFINE_TEST(test_validate_path)
{
	DESCRIBE_TEST("Unit test for test_validate_path");
}


DEFINE_TEST(test_event_base_create_and_init)
{
	DESCRIBE_TEST("Unit test for event_base_create_and_init");
}


// TODO(phymod0): Remaining tests


START(test_str_ndup, test_get_path_subtree, test_path_argv_create,
      test_path_argv_destroy, test_find_path_subtree,
      test_register_single_handler, test_handle_signal, test_get_method_str,
      test_get_request_path, test_validate_path,
      test_event_base_create_and_init)
