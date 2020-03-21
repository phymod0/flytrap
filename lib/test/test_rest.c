#define MOCK_TESTING
#define LOGGING_ENABLE 0


#ifdef MOCK_TESTING

#include <event2/event.h>

struct {
	struct event_base* event_base;
} mock_data;

static int __mock_event_base_loopbreak(struct event_base* base)
{
	mock_data.event_base = base;
	return 0;
}
#define event_base_loopbreak __mock_event_base_loopbreak

#endif /* MOCK_TESTING */


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


static bool is_path_in_tree(StringTree* tree, const char* path)
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
			      is_path_in_tree(tree, good_path));
		free(good_path);

		bad_path = rand_path_with_len(REST_PATH_MAXSZ * 2);
		CHECK_INCLUDE(allocation_success,
			      get_path_subtree(tree, bad_path) != NULL);
		CHECK_INCLUDE(paths_truncated,
			      !is_path_in_tree(tree, bad_path));
		free(bad_path);

		string_tree_destroy(tree);
	}

	{ // Test wildcard substitution
		typedef struct {
			const char* path;
			int expected_argc;
			char* expected_argv[REST_PATH_MAX_WILDCARDS];
		} TestData;
		typedef struct {
			const char* base_path;
			TestData test_data[16]; // NOLINT
		} TData;

		TData t_data[4] = {
		    {
			// Path with 0 segments
			"/",
			{
			    {
				"/",
				0,
				{NULL},
			    },
			},
		    },

		    {
			// Path with 1 segment
			"/a",
			{
			    {
				"/a",
				0,
				{NULL},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD,
				1,
				{"a"},
			    },
			},
		    },

		    {
			// Path with 2 segments
			"/a/b",
			{
			    {
				"/a/b",
				0,
				{NULL},
			    },
			    {
				"/a/" REST_PATH_SEGMENT_WILDCARD,
				1,
				{"b"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD "/b",
				1,
				{"a"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD
				"/" REST_PATH_SEGMENT_WILDCARD,
				2,
				{"a", "b"},
			    },
			},
		    },

		    {
			// Path with 3 segments
			"/a/b/c",
			{
			    {
				"/a/b/c",
				0,
				{NULL},
			    },
			    {
				"/a/b/" REST_PATH_SEGMENT_WILDCARD,
				1,
				{"c"},
			    },
			    {
				"/a/" REST_PATH_SEGMENT_WILDCARD "/c",
				1,
				{"b"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD "/b/c",
				1,
				{"a"},
			    },
			    {
				"/a/" REST_PATH_SEGMENT_WILDCARD
				"/" REST_PATH_SEGMENT_WILDCARD,
				2,
				{"b", "c"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD
				"/b/" REST_PATH_SEGMENT_WILDCARD,
				2,
				{"a", "c"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD
				"/" REST_PATH_SEGMENT_WILDCARD "/c",
				2,
				{"a", "b"},
			    },
			    {
				"/" REST_PATH_SEGMENT_WILDCARD
				"/" REST_PATH_SEGMENT_WILDCARD
				"/" REST_PATH_SEGMENT_WILDCARD,
				3,
				{"a", "b", "c"},
			    },
			},
		    },
		};

		const int t_data_sz = sizeof t_data / sizeof t_data[0];
		for (int i = 0; i < t_data_sz; ++i) {
			const char* base_path = t_data[i].base_path;
			TestData* test_data = t_data[i].test_data;

			for (int j = 0; j < (1 << i); ++j) {
				StringTree* tree;
				StringTree* subtree;
				TestData data = test_data[j];
				int expected_argc = data.expected_argc;
				char** expected_argv = data.expected_argv;
				int argc;
				char** argv;

				tree = string_tree_create();
				CHECK_INCLUDE(allocation_success, tree != NULL);
				subtree = get_path_subtree(tree, data.path);
				CHECK_INCLUDE(allocation_success,
					      subtree != NULL);
				subtree = find_path_subtree(tree, base_path,
							    &argc, &argv);
				CHECK_INCLUDE(wildcard_matched,
					      subtree != NULL);
				CHECK_INCLUDE(correct_argc,
					      argc == expected_argc); // NOLINT
				for (int k = 0; k < argc; ++k) {
					char* actual = argv[k];
					char* expected = expected_argv[k];
					CHECK_INCLUDE(
					    correct_argv,
					    strcmp(actual, expected) == 0);
				}

				path_argv_destroy(argv);
				string_tree_destroy(tree);
			}
		}
	}

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

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(path_correct, "Handlers are set at the correct path");
	DEFINE_CHECK(replaced, "Handlers are inserted with replacement");

	RestCtx* ctx = rest_ctx_create();
	StringTree* tree;
	HTTPHandler handler = {
	    .path = "/a/b/c/d",
	    .methods =
		{
		    .GET = (HTTPMethod)0x1122334455667788,    // NOLINT
		    .POST = (HTTPMethod)0x1133557722446688,   // NOLINT
		    .DELETE = (HTTPMethod)0x8877665544332211, // NOLINT
		    .PUT = (HTTPMethod)0x2244668811335577,    // NOLINT
		},
	};
	int argc;
	char** argv;

	CHECK_INCLUDE(mem_alloc, ctx != NULL);
	CHECK_INCLUDE(mem_alloc, register_single_handler(&handler, ctx) != -1);
	tree = find_path_subtree(ctx->handlers, handler.path, &argc, &argv);
	path_argv_destroy(argv);
	CHECK_INCLUDE(path_correct, tree != NULL);
	CHECK_INCLUDE(path_correct,
		      (HTTPMethods*)(tree->val) == &(handler.methods));

	handler.methods = (HTTPMethods){
	    .GET = (HTTPMethod)0x8877665544332211,    // NOLINT
	    .POST = (HTTPMethod)0x2244668811335577,   // NOLINT
	    .DELETE = (HTTPMethod)0x1122334455667788, // NOLINT
	    .PUT = (HTTPMethod)0x1133557722446688,    // NOLINT
	};
	CHECK_INCLUDE(mem_alloc, register_single_handler(&handler, ctx) != -1);
	tree = find_path_subtree(ctx->handlers, handler.path, &argc, &argv);
	path_argv_destroy(argv);
	CHECK_INCLUDE(path_correct, tree != NULL);
	CHECK_INCLUDE(replaced,
		      (HTTPMethods*)(tree->val) == &(handler.methods));

	rest_ctx_destroy(ctx);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(path_correct);
	ASSERT_CHECK(replaced);
}


DEFINE_TEST(test_handle_signal)
{
	DESCRIBE_TEST("Unit test for handle_signal");
#ifdef MOCK_TESTING

	DEFINE_CHECK(correct_arg, "Correct arg passed to event_base_loopbreak");

#define RANDOM_ADDRESS 0x1122334455667788
	handle_signal(0, 0, (void*)RANDOM_ADDRESS);
	CHECK_INCLUDE(correct_arg, mock_data.event_base ==
				       (struct event_base*)RANDOM_ADDRESS);
#undef RANDOM_ADDRESS

	ASSERT_CHECK(correct_arg);
#endif /* MOCK_TESTING */
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


// TODO(phymod0): Test interfaces


START(test_str_ndup, test_get_path_subtree, test_path_argv_create,
      test_path_argv_destroy, test_find_path_subtree,
      test_register_single_handler, test_handle_signal, test_get_method_str,
      test_get_request_path, test_validate_path,
      test_event_base_create_and_init)
