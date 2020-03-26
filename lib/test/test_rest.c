#define MOCK_TESTING
#define LOGGING_ENABLE 0


#ifdef MOCK_TESTING

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <time.h>

struct {
	struct event_base* event_base;
	struct event* event;
	int last_error;
	int function_called;
	int last_path_argc;
	char** last_path_argv;
	int last_http_method;
	int http_method_called;
} mock_data;

static int __mock_event_base_loopbreak(struct event_base* base)
{
	mock_data.event_base = base;
	return 0;
}
#define event_base_loopbreak __mock_event_base_loopbreak

static int __mock_event_add(struct event* ev, struct timeval* tv)
{
	(void)tv;
	mock_data.event_base = ev->ev_base;
	mock_data.event = ev;
	return 0;
}
#define event_add __mock_event_add

static int __mock_evhttp_send_error(struct evhttp_request* req, int reason_code,
				    const char* reason_str)
{
	(void)req;
	(void)reason_str;
	mock_data.last_error = reason_code;
	return 0;
}
#define evhttp_send_error __mock_evhttp_send_error

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


static int file_request_mock_handler(void* server_ctx,
				     struct evhttp_request* req,
				     const char* path)
{
	(void)server_ctx;
	(void)req;
	mock_data.function_called = true;
	if (strcmp(path, "Intentionally throw 500") == 0) {
		return -1;
	}
	return 0;
}


static char** path_argv_copy(int path_argc, char** path_argv)
{
	char** result = malloc(path_argc * sizeof *result);
	for (int i = 0; i < path_argc; ++i) {
		result[i] = strdup(path_argv[i]);
	}
	return result;
}


static bool path_argv_compare(int argc1, char** argv1, int argc2, char** argv2)
{
	if (argc1 != argc2) {
		return false;
	}
	for (int i = 0; i < argc1; ++i) {
		if (strcmp(argv1[i], argv2[i]) != 0) {
			return false;
		}
	}
	return true;
}


static void path_argv_free(int argc, char** argv)
{
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);
}


static int http_mock_method(void* server_ctx, struct evhttp_request* req,
			    int path_argc, char** path_argv)
{
	(void)server_ctx;
	(void)req;
	mock_data.last_path_argc = path_argc;
	mock_data.last_path_argv = path_argv_copy(path_argc, path_argv);
	mock_data.last_http_method = req->type;
	mock_data.http_method_called = true;
	if (path_argc == 1 &&
	    strcmp(path_argv[0], "intentionally_throw_500") == 0) {
		return -1;
	}
	return 0;
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
					CHECK_INCLUDE(allocation_success,
						      actual != NULL &&
							  expected != NULL);
					if (actual == NULL ||
					    expected == NULL) {
						continue;
					}
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

	DEFINE_CHECK(correct_strs, "Correct strings are returned");

#define STREQ(str1, str2) (strcmp(str1, str2) == 0)
#define VERIFY_FOR_METHOD(method)                                              \
	CHECK_INCLUDE(correct_strs,                                            \
		      STREQ(get_method_str(EVHTTP_REQ_##method), #method))

	VERIFY_FOR_METHOD(GET);
	VERIFY_FOR_METHOD(POST);
	VERIFY_FOR_METHOD(HEAD);
	VERIFY_FOR_METHOD(PUT);
	VERIFY_FOR_METHOD(DELETE);
	VERIFY_FOR_METHOD(OPTIONS);
	VERIFY_FOR_METHOD(TRACE);
	VERIFY_FOR_METHOD(CONNECT);
	VERIFY_FOR_METHOD(PATCH);
	CHECK_INCLUDE(correct_strs, STREQ(get_method_str(9), "unknown"));

#undef VERIFY_FOR_METHOD
#undef STREQ

	ASSERT_CHECK(correct_strs);
}


DEFINE_TEST(test_get_request_path)
{
	DESCRIBE_TEST("Unit test for get_request_path");

#ifdef MOCK_TESTING
	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(correct_parsed, "URIs are parsed correctly");

#define VERIFY(_uri, path)                                                     \
	{                                                                      \
		struct evhttp_request req;                                     \
		req.uri = _uri;                                                \
		char* result = get_request_path(&req);                         \
		CHECK_INCLUDE(mem_alloc, result != NULL);                      \
		CHECK_INCLUDE(correct_parsed, strcmp(path, result) == 0);      \
		free(result);                                                  \
	}

#define scheme "http://"
#define domain "asdf.io"
#define port "1234"
#define params "?abc=1&def=2"
#define fragment "#fragment"
#define VERIFY_FOR_PATH(path)                                                  \
	{                                                                      \
		VERIFY(scheme domain path, path);                              \
		VERIFY(scheme domain port path, path);                         \
		VERIFY(scheme domain path params, path);                       \
		VERIFY(scheme domain port path params, path);                  \
		VERIFY(scheme domain path fragment, path);                     \
		VERIFY(scheme domain port path fragment, path);                \
		VERIFY(scheme domain path params fragment, path);              \
		VERIFY(scheme domain port path params fragment, path);         \
	}

	VERIFY_FOR_PATH("/");
	VERIFY_FOR_PATH("/abc/");
	VERIFY_FOR_PATH("/abc/def/ghi");
	VERIFY_FOR_PATH("/abc//def/ghi///");

#undef VERIFY_FOR_PATH
#undef fragment
#undef params
#undef port
#undef domain
#undef scheme

#undef VERIFY

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(correct_parsed);
#endif /* MOCK_TESTING */
}


DEFINE_TEST(test_validate_path)
{
	DESCRIBE_TEST("Unit test for validate_path");

	DEFINE_CHECK(validation_sound, "Validation is sound");
	DEFINE_CHECK(validation_complete, "Validation is complete");

	/* TODO(phymod0): Find a better way */
#define EXPECT_INVALID(path)                                                   \
	CHECK_INCLUDE(validation_sound, validate_path(path) == -1)
#define EXPECT_VALID(path)                                                     \
	CHECK_INCLUDE(validation_complete, validate_path(path) == 0)

	EXPECT_INVALID("..");
	EXPECT_INVALID("../");
	EXPECT_INVALID("../../");
	EXPECT_INVALID("../protected_folder");
	EXPECT_INVALID("../protected_folder/");
	EXPECT_INVALID(".//../protected_folder");
	EXPECT_INVALID("./public_folder/../../protected_folder");

	EXPECT_VALID(".");
	EXPECT_VALID("./");
	EXPECT_VALID("public_folder");
	EXPECT_VALID("public_folder/");
	EXPECT_VALID("./public_folder");
	EXPECT_VALID("./public_folder/");
	EXPECT_VALID("./public_folder/./");

#undef EXPECT_VALID
#undef EXPECT_INVALID

	ASSERT_CHECK(validation_sound);
	ASSERT_CHECK(validation_complete);
}


DEFINE_TEST(test_document_request_cb)
{
	DESCRIBE_TEST("Unit test for document_request_cb");

#ifdef MOCK_TESTING
	DEFINE_CHECK(correct_errors, "Correct http error codes sent");
	DEFINE_CHECK(handler_sound, "Handler calling sound");
	DEFINE_CHECK(handler_complete, "Handler calling complete");

	{ // Test for 405 response on non-existent (NULL) handler
		struct evhttp_request req;
		const char* path = NULL;
		RestCtx ctx = {
		    .file_request_handler = NULL,
		};

		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		document_request_cb(&req, path, &ctx);
		CHECK_INCLUDE(correct_errors,
			      mock_data.last_error == HTTP_BADMETHOD);
		CHECK_INCLUDE(handler_sound,
			      mock_data.function_called == false);
	}

	{ // Test for 400 response on invalid path
		struct evhttp_request req;
		RestCtx ctx = {
		    .file_request_handler = file_request_mock_handler,
		};

		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
#define TEST_ON_BAD_PATH(path)                                                 \
	{                                                                      \
		document_request_cb(&req, path, &ctx);                         \
		CHECK_INCLUDE(correct_errors,                                  \
			      mock_data.last_error == HTTP_BADREQUEST);        \
		CHECK_INCLUDE(handler_sound,                                   \
			      mock_data.function_called == false);             \
		mock_data.last_error = HTTP_OK;                                \
	}

		TEST_ON_BAD_PATH("..");
		TEST_ON_BAD_PATH("../");
		TEST_ON_BAD_PATH("../../");
		TEST_ON_BAD_PATH("../protected_folder");
		TEST_ON_BAD_PATH("../protected_folder/");
		TEST_ON_BAD_PATH(".//../protected_folder");
		TEST_ON_BAD_PATH("./public_folder/../../protected_folder");

#undef TEST_ON_BAD_PATH
	}

	{ // Test for 500 response on the handler throwing -1
		struct evhttp_request req;
		const char* path = "Intentionally throw 500";
		RestCtx ctx = {
		    .file_request_handler = file_request_mock_handler,
		};

		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		document_request_cb(&req, path, &ctx);
		CHECK_INCLUDE(correct_errors,
			      mock_data.last_error == HTTP_INTERNAL);
		CHECK_INCLUDE(handler_complete,
			      mock_data.function_called == true);
	}

	{ // Test for no error on a valid request with the handler returning 0
		struct evhttp_request req;
		const char* path = "./path/to/document.html";
		RestCtx ctx = {
		    .file_request_handler = file_request_mock_handler,
		};

		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		document_request_cb(&req, path, &ctx);
		CHECK_INCLUDE(correct_errors, mock_data.last_error == HTTP_OK);
		CHECK_INCLUDE(handler_complete,
			      mock_data.function_called == true);
	}

	ASSERT_CHECK(correct_errors);
	ASSERT_CHECK(handler_sound);
	ASSERT_CHECK(handler_complete);
#endif /* MOCK_TESTING */
}


DEFINE_TEST(test_generic_handler_cb) // NOLINT
{
	DESCRIBE_TEST("Unit test for generic_handler_cb");

#ifdef MOCK_TESTING
	DEFINE_CHECK(correct_errors, "Correct http error codes sent");
	DEFINE_CHECK(document_request_cb_called_sound,
		     "document_request_cb not called when not required");
	DEFINE_CHECK(document_request_cb_called_complete,
		     "document_request_cb called when required");
	DEFINE_CHECK(no_handlers, "Safe to have no registered handlers");
	DEFINE_CHECK(no_methods, "Safe to have no registered methods");
	DEFINE_CHECK(null_method, "Safe for the requested method to be null");
	DEFINE_CHECK(unsupported_method,
		     "Safe for the requested method to not be supported");
	DEFINE_CHECK(method_called, "Correct method called when available");
	DEFINE_CHECK(correct_args, "Correct arguments passed to HTTP methods");

#define MOCK_ADDR "127.0.0.1"
#define MOCK_PORT 1234
#define GOOD_URI "http://asdf.io:1234/abc/def/ghi?abc=123&xyz=789"
#define BAD_URI "H&*4pujsdOJ..df.fasd./[asd[g;l[pdf[gdfsa"

	{ // Test for 400 on invalid path
		struct event_base* base = event_base_new();
		struct evhttp_connection* conn = evhttp_connection_base_new(
		    base, NULL, MOCK_ADDR, MOCK_PORT);
		struct evhttp_request req = {.evcon = conn, .uri = BAD_URI};
		RestCtx ctx = {
		    .file_request_handler = file_request_mock_handler,
		};
		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		generic_handler_cb(&req, &ctx);
		CHECK_INCLUDE(correct_errors,
			      mock_data.last_error == HTTP_BADREQUEST);
		CHECK_INCLUDE(document_request_cb_called_sound,
			      !mock_data.function_called);
		evhttp_connection_free(conn);
		event_base_free(base);
	}

	{ // Test for document handling on missing path handlers/methods
		struct event_base* base = event_base_new();
		struct evhttp_connection* conn = evhttp_connection_base_new(
		    base, NULL, MOCK_ADDR, MOCK_PORT);
		struct evhttp_request req = {.evcon = conn, .uri = GOOD_URI};
		StringTree* handlers = string_tree_create();
		StringTree* handlers_subtree = handlers;
		RestCtx ctx;

		// When the path isn't registered
		ctx.handlers = NULL;
		ctx.file_request_handler = file_request_mock_handler;
		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		generic_handler_cb(&req, &ctx);
		CHECK_INCLUDE(correct_errors, mock_data.last_error == HTTP_OK);
		CHECK_INCLUDE(document_request_cb_called_complete,
			      mock_data.function_called);
		CHECK_INCLUDE(no_handlers, true);

		// Otherwise when the path doesn't have registered methods
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "abc");
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "def");
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "ghi");
		ctx.handlers = handlers_subtree;
		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		generic_handler_cb(&req, &ctx);
		CHECK_INCLUDE(correct_errors, mock_data.last_error == HTTP_OK);
		CHECK_INCLUDE(document_request_cb_called_complete,
			      mock_data.function_called);
		CHECK_INCLUDE(no_methods, true);

		string_tree_destroy(handlers);
		evhttp_connection_free(conn);
		event_base_free(base);
	}

	{ // Test for HTTP 405 on method not set
		struct event_base* base = event_base_new();
		struct evhttp_connection* conn = evhttp_connection_base_new(
		    base, NULL, MOCK_ADDR, MOCK_PORT);
		struct evhttp_request req = {
		    .evcon = conn, .uri = GOOD_URI, .type = EVHTTP_REQ_GET};

		StringTree* handlers = string_tree_create();
		StringTree* handlers_subtree = handlers;
		HTTPMethods methods = {
		    .GET = NULL,
		    .PUT = NULL,
		    .POST = NULL,
		    .DELETE = NULL,
		};
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "abc");
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "def");
		handlers_subtree =
		    string_tree_get_subtree(handlers_subtree, "ghi");
		string_tree_set_value(handlers_subtree, &methods);
		RestCtx ctx = {
		    .file_request_handler = file_request_mock_handler,
		    .handlers = handlers,
		};

		mock_data.last_error = HTTP_OK;
		mock_data.function_called = false;
		generic_handler_cb(&req, &ctx);
		CHECK_INCLUDE(correct_errors,
			      mock_data.last_error == HTTP_BADMETHOD);
		CHECK_INCLUDE(document_request_cb_called_sound,
			      !mock_data.function_called);
		CHECK_INCLUDE(null_method, true);

		string_tree_destroy(handlers);
		evhttp_connection_free(conn);
		event_base_free(base);
	}

	{ // Test for HTTP 405 on unsupported method
		struct event_base* base = event_base_new();
		struct evhttp_connection* conn = evhttp_connection_base_new(
		    base, NULL, MOCK_ADDR, MOCK_PORT);
#define VERIFY_FOR(method)                                                     \
	{                                                                      \
		struct evhttp_request req = {                                  \
		    .evcon = conn, .uri = GOOD_URI, .type = (method)};         \
                                                                               \
		StringTree* handlers = string_tree_create();                   \
		StringTree* handlers_subtree = handlers;                       \
		handlers_subtree =                                             \
		    string_tree_get_subtree(handlers_subtree, "abc");          \
		handlers_subtree =                                             \
		    string_tree_get_subtree(handlers_subtree, "def");          \
		handlers_subtree =                                             \
		    string_tree_get_subtree(handlers_subtree, "ghi");          \
		string_tree_set_value(handlers_subtree, &(HTTPMethods){        \
							    .GET = NULL,       \
							    .PUT = NULL,       \
							    .POST = NULL,      \
							    .DELETE = NULL,    \
							});                    \
		RestCtx ctx = {                                                \
		    .file_request_handler = file_request_mock_handler,         \
		    .handlers = handlers,                                      \
		};                                                             \
                                                                               \
		mock_data.last_error = HTTP_OK;                                \
		mock_data.function_called = false;                             \
		generic_handler_cb(&req, &ctx);                                \
		CHECK_INCLUDE(correct_errors,                                  \
			      mock_data.last_error == HTTP_BADMETHOD);         \
		CHECK_INCLUDE(document_request_cb_called_sound,                \
			      !mock_data.function_called);                     \
		CHECK_INCLUDE(unsupported_method, true);                       \
                                                                               \
		string_tree_destroy(handlers);                                 \
	}

		VERIFY_FOR(EVHTTP_REQ_HEAD);
		VERIFY_FOR(EVHTTP_REQ_OPTIONS);
		VERIFY_FOR(EVHTTP_REQ_TRACE);
		VERIFY_FOR(EVHTTP_REQ_CONNECT);
		VERIFY_FOR(EVHTTP_REQ_PATCH);
		VERIFY_FOR(1 << 9);
		VERIFY_FOR(1 << 10);

		evhttp_connection_free(conn);
		event_base_free(base);
#undef VERIFY_FOR
	}

	{ // Test for invocation of well-defined methods
		struct event_base* base = event_base_new();
		struct evhttp_connection* conn = evhttp_connection_base_new(
		    base, NULL, MOCK_ADDR, MOCK_PORT);
#define VERIFY_FOR(registered_path, request_uri_path, expected_argc,           \
		   expected_argv, http_method, expected_http_status)           \
	{                                                                      \
		struct evhttp_request req = {                                  \
		    .evcon = conn,                                             \
		    .uri = "http://asdf.io:1234" request_uri_path              \
			   "?abc=123&xyz=789",                                 \
		    .type = EVHTTP_REQ_##http_method,                          \
		};                                                             \
                                                                               \
		HTTPMethods methods = {                                        \
		    .http_method = http_mock_method,                           \
		};                                                             \
		StringTree* handlers = string_tree_create();                   \
		StringTree* handlers_subtree = handlers;                       \
		char* _path = strdup(registered_path);                         \
		for (char* tok = strtok(_path, "/"); tok;                      \
		     tok = strtok(NULL, "/")) {                                \
			handlers_subtree =                                     \
			    string_tree_get_subtree(handlers_subtree, tok);    \
		}                                                              \
		free(_path);                                                   \
		string_tree_set_value(handlers_subtree, &methods);             \
		RestCtx ctx = {                                                \
		    .file_request_handler = file_request_mock_handler,         \
		    .handlers = handlers,                                      \
		};                                                             \
                                                                               \
		mock_data.last_error = HTTP_OK;                                \
		mock_data.function_called = false;                             \
		mock_data.http_method_called = false;                          \
		mock_data.last_path_argc = -1;                                 \
		mock_data.last_path_argv = NULL;                               \
		mock_data.last_http_method = -1;                               \
		generic_handler_cb(&req, &ctx);                                \
		CHECK_INCLUDE(correct_errors,                                  \
			      mock_data.last_error == (expected_http_status)); \
		CHECK_INCLUDE(document_request_cb_called_sound,                \
			      !mock_data.function_called);                     \
		CHECK_INCLUDE(method_called, mock_data.http_method_called);    \
		CHECK_INCLUDE(correct_args, mock_data.last_http_method ==      \
						EVHTTP_REQ_##http_method);     \
		CHECK_INCLUDE(correct_args,                                    \
			      path_argv_compare(mock_data.last_path_argc,      \
						mock_data.last_path_argv,      \
						expected_argc,                 \
						expected_argv));               \
                                                                               \
		path_argv_free(mock_data.last_path_argc,                       \
			       mock_data.last_path_argv);                      \
		string_tree_destroy(handlers);                                 \
	}
#define MAKE_ARGV(...) ((char**)((const char*[]){__VA_ARGS__}))
#define VERIFY_FOR_METHOD(method)                                              \
	{                                                                      \
		VERIFY_FOR("/", "/", 0, MAKE_ARGV(NULL), method, HTTP_OK);     \
		VERIFY_FOR("/abc", "/abc", 0, MAKE_ARGV(NULL), method,         \
			   HTTP_OK);                                           \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD, "/abc", 1,          \
			   MAKE_ARGV("abc"), method, HTTP_OK);                 \
		VERIFY_FOR("/intentionally_throw_500",                         \
			   "/intentionally_throw_500", 0, MAKE_ARGV(NULL),     \
			   method, HTTP_OK);                                   \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD,                     \
			   "/intentionally_throw_500", 1,                      \
			   MAKE_ARGV("intentionally_throw_500"), method,       \
			   HTTP_INTERNAL);                                     \
                                                                               \
		VERIFY_FOR("/abc/def", "/abc/def", 0, MAKE_ARGV(NULL), method, \
			   HTTP_OK);                                           \
		VERIFY_FOR("/abc/" REST_PATH_SEGMENT_WILDCARD, "/abc/def", 1,  \
			   MAKE_ARGV("def"), method, HTTP_OK);                 \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD "/def", "/abc/def",  \
			   1, MAKE_ARGV("abc"), method, HTTP_OK);              \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD                      \
			   "/" REST_PATH_SEGMENT_WILDCARD,                     \
			   "/abc/def", 2, MAKE_ARGV("abc", "def"), method,     \
			   HTTP_OK);                                           \
                                                                               \
		VERIFY_FOR("/abc/def/ghi", "/abc/def/ghi", 0, MAKE_ARGV(NULL), \
			   method, HTTP_OK);                                   \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD "/def/ghi",          \
			   "/abc/def/ghi", 1, MAKE_ARGV("abc"), method,        \
			   HTTP_OK);                                           \
		VERIFY_FOR("/abc/" REST_PATH_SEGMENT_WILDCARD "/ghi",          \
			   "/abc/def/ghi", 1, MAKE_ARGV("def"), method,        \
			   HTTP_OK);                                           \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD                      \
			   "/" REST_PATH_SEGMENT_WILDCARD "/ghi",              \
			   "/abc/def/ghi", 2, MAKE_ARGV("abc", "def"), method, \
			   HTTP_OK);                                           \
		VERIFY_FOR("/abc/def/" REST_PATH_SEGMENT_WILDCARD,             \
			   "/abc/def/ghi", 1, MAKE_ARGV("ghi"), method,        \
			   HTTP_OK);                                           \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD                      \
			   "/def/" REST_PATH_SEGMENT_WILDCARD,                 \
			   "/abc/def/ghi", 2, MAKE_ARGV("abc", "ghi"), method, \
			   HTTP_OK);                                           \
		VERIFY_FOR("/abc/" REST_PATH_SEGMENT_WILDCARD                  \
			   "/" REST_PATH_SEGMENT_WILDCARD,                     \
			   "/abc/def/ghi", 2, MAKE_ARGV("def", "ghi"), method, \
			   HTTP_OK);                                           \
		VERIFY_FOR("/" REST_PATH_SEGMENT_WILDCARD                      \
			   "/" REST_PATH_SEGMENT_WILDCARD                      \
			   "/" REST_PATH_SEGMENT_WILDCARD,                     \
			   "/abc/def/ghi", 3, MAKE_ARGV("abc", "def", "ghi"),  \
			   method, HTTP_OK);                                   \
	}

		VERIFY_FOR_METHOD(GET);
		VERIFY_FOR_METHOD(PUT);
		VERIFY_FOR_METHOD(POST);
		VERIFY_FOR_METHOD(DELETE);

#undef VERIFY_FOR_METHOD
#undef MAKE_ARGV
#undef VERIFY_FOR
		evhttp_connection_free(conn);
		event_base_free(base);
	}

#undef BAD_URI
#undef GOOD_URI
#undef MOCK_PORT
#undef MOCK_ADDR

	ASSERT_CHECK(correct_errors);
	ASSERT_CHECK(document_request_cb_called_sound);
	ASSERT_CHECK(document_request_cb_called_complete);
	ASSERT_CHECK(no_handlers);
	ASSERT_CHECK(no_methods);
	ASSERT_CHECK(null_method);
	ASSERT_CHECK(unsupported_method);
	ASSERT_CHECK(method_called);
	ASSERT_CHECK(correct_args);
#endif /* MOCK_TESTING */
}


DEFINE_TEST(test_event_base_create_and_init)
{
	DESCRIBE_TEST("Unit test for event_base_create_and_init");

	DEFINE_CHECK(mem_alloc, "Allocations successful");
	DEFINE_CHECK(correct_return, "Correct sigint event is returned");
	DEFINE_CHECK(correct_sigterm, "Correct sigint values are set");
#ifdef MOCK_TESTING
	DEFINE_CHECK(added, "event_add called");
#endif /* MOCK_TESTING */

	struct event* sigterm = NULL;
	struct event_base* base = event_base_create_and_init(&sigterm);

	struct event_base* actual_base = NULL;
	event_callback_fn callback;
	void* cb_data;
	event_get_assignment(sigterm, &actual_base, NULL, NULL, &callback,
			     &cb_data);

	CHECK_INCLUDE(mem_alloc, base != NULL);
	CHECK_INCLUDE(mem_alloc, sigterm != NULL);
	CHECK_INCLUDE(correct_return, callback == handle_signal);
	CHECK_INCLUDE(correct_sigterm, actual_base == base);
	CHECK_INCLUDE(correct_sigterm, (struct event_base*)cb_data == base);
#ifdef MOCK_TESTING
	CHECK_INCLUDE(added, mock_data.event_base == base);
	CHECK_INCLUDE(added, mock_data.event == sigterm);
#endif /* MOCK_TESTING */

	event_free(sigterm);
	event_base_free(base);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(correct_return);
	ASSERT_CHECK(correct_sigterm);
#ifdef MOCK_TESTING
	ASSERT_CHECK(added);
#endif /* MOCK_TESTING */
}


// TODO(phymod0): Test interfaces


START(test_str_ndup, test_get_path_subtree, test_path_argv_create,
      test_path_argv_destroy, test_find_path_subtree,
      test_register_single_handler, test_handle_signal, test_get_method_str,
      test_get_request_path, test_validate_path, test_document_request_cb,
      test_generic_handler_cb, test_event_base_create_and_init)
