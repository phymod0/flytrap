#define MOCK_TESTING
#define LOGGING_ENABLE 0


#ifdef MOCK_TESTING

#include <event2/http.h>

struct {
	struct event_base* event_base;
	struct evhttp* last_http;
	const char* last_ip;
	int last_port;
} mock_data;


static int __mock_event_base_loopbreak(struct event_base* base)
{
	mock_data.event_base = base;
	return base ? 0 : -1;
}
#define event_base_loopbreak __mock_event_base_loopbreak


static int __mock_evhttp_bind_socket(struct evhttp* http, const char* ip,
				     int port)
{
	mock_data.last_http = http;
	mock_data.last_ip = ip;
	mock_data.last_port = port;
	return 0;
}
#define evhttp_bind_socket __mock_evhttp_bind_socket


static int __mock_event_base_dispatch(struct event_base* base)
{
	if (base == (struct event_base*)0x1) {
		return 0;
	}
	if (base == (struct event_base*)0x2) {
		return 1;
	}
	return (int)(unsigned long long)base - 1;
}
#define event_base_dispatch __mock_event_base_dispatch


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


static StringTree* get_path_in_tree(StringTree* tree, const char* path)
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
	return tree;
}


DEFINE_TEST(test_rest_ctx_create)
{
	DESCRIBE_TEST("Unit test for rest_ctx_create");

	DEFINE_CHECK(mem_alloc, "Allocations successful");
	DEFINE_CHECK(base_alloc, "Memory allocated for ctx->base");
	DEFINE_CHECK(http_alloc, "Memory allocated for ctx->http");
	DEFINE_CHECK(http_cb_correct,
		     "Correct callback function set for ctx->http");
	DEFINE_CHECK(term_alloc, "Memory allocated for ctx->term");
	DEFINE_CHECK(handlers_alloc, "Memory allocated for ctx->handlers");
	DEFINE_CHECK(file_request_handler_unset,
		     "ctx->file_request_handler set to NULL");
	DEFINE_CHECK(server_ctx_unset, "ctx->server_ctx set to NULL");

	RestCtx* ctx = rest_ctx_create();

	CHECK_INCLUDE(mem_alloc, ctx != NULL);
	CHECK_INCLUDE(base_alloc, ctx->base != NULL);
	CHECK_INCLUDE(http_alloc, ctx->http != NULL);
	CHECK_INCLUDE(http_cb_correct, ctx != NULL);
	CHECK_INCLUDE(term_alloc, ctx->term != NULL);
	CHECK_INCLUDE(handlers_alloc, ctx->handlers != NULL);
	CHECK_INCLUDE(file_request_handler_unset,
		      ctx->file_request_handler == NULL);
	CHECK_INCLUDE(server_ctx_unset, ctx->server_ctx == NULL);

	string_tree_destroy(ctx->handlers);
	if (ctx->http) {
		evhttp_free(ctx->http);
	}
	if (ctx->term) {
		event_free(ctx->term);
	}
	if (ctx->base) {
		event_base_free(ctx->base);
	}
	free(ctx);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(base_alloc);
	ASSERT_CHECK(http_alloc);
	ASSERT_CHECK(http_cb_correct);
	ASSERT_CHECK(term_alloc);
	ASSERT_CHECK(handlers_alloc);
	ASSERT_CHECK(file_request_handler_unset);
	ASSERT_CHECK(server_ctx_unset);
}


DEFINE_TEST(test_rest_ctx_destroy)
{
	DESCRIBE_TEST("Unit test for rest_ctx_destroy");

	rest_ctx_destroy(rest_ctx_create());
}


DEFINE_TEST(test_rest_register_handlers)
{
	DESCRIBE_TEST("Unit test for rest_register_handlers");

	DEFINE_CHECK(mem_alloc, "Memory allocations successful");
	DEFINE_CHECK(paths_registered, "All provided paths registered");
	DEFINE_CHECK(path_data_correct, "Correct data at all registered paths");

	int ret;
	RestCtx* ctx;
	HTTPHandler handlers[] = {
	    {
		.path = "/",
		.methods =
		    {
			.GET = (HTTPMethod)NULL,                  // NOLINT
			.PUT = (HTTPMethod)0x8877665544332211,    // NOLINT
			.POST = (HTTPMethod)0x1122334455667788,   // NOLINT
			.DELETE = (HTTPMethod)0x2244668811335577, // NOLINT
		    },
	    },
	    {
		.path = "/a",
		.methods =
		    {
			.GET = (HTTPMethod)0x1122334455667788,    // NOLINT
			.PUT = (HTTPMethod)0x1133557722446688,    // NOLINT
			.POST = (HTTPMethod)0x2244668811335577,   // NOLINT
			.DELETE = (HTTPMethod)0x8877665544332211, // NOLINT
		    },
	    },
	    {
		.path = "/a/b",
		.methods =
		    {
			.GET = (HTTPMethod)0x8877665544332211,    // NOLINT
			.PUT = (HTTPMethod)0x2244668811335577,    // NOLINT
			.POST = (HTTPMethod)0x2244668811335577,   // NOLINT
			.DELETE = (HTTPMethod)0x1122334455667788, // NOLINT
		    },
	    },
	    {
		.path = "/a/b/c",
		.methods =
		    {
			.GET = (HTTPMethod)0x1122334455667788,    // NOLINT
			.PUT = (HTTPMethod)0x1133557722446688,    // NOLINT
			.POST = (HTTPMethod)0x8877665544332211,   // NOLINT
			.DELETE = (HTTPMethod)0x1133557722446688, // NOLINT
		    },
	    },
	    {
		.path = "/d/b/c",
		.methods =
		    {
			.GET = (HTTPMethod)0x123,    // NOLINT
			.PUT = (HTTPMethod)0x456,    // NOLINT
			.POST = (HTTPMethod)0x789,   // NOLINT
			.DELETE = (HTTPMethod)0xABC, // NOLINT
		    },
	    },
	    REST_END_HANDLERS,
	};

	ctx = rest_ctx_create();
	CHECK_INCLUDE(mem_alloc, ctx != NULL);
	ret = rest_register_handlers(handlers, ctx);
	CHECK_INCLUDE(mem_alloc, ret != -1);
	for (HTTPHandler* handler = handlers; handler->path; ++handler) {
		StringTree* tree =
		    get_path_in_tree(ctx->handlers, handler->path);
		HTTPMethods* methods;
		CHECK_INCLUDE(paths_registered, tree != NULL);
		if (tree != NULL) {
			methods = (HTTPMethods*)string_tree_get_value(tree);
			CHECK_INCLUDE(path_data_correct,
				      methods->GET == handler->methods.GET);
		}
	}

	rest_ctx_destroy(ctx);

	ASSERT_CHECK(mem_alloc);
	ASSERT_CHECK(paths_registered);
	ASSERT_CHECK(path_data_correct);
}


DEFINE_TEST(test_rest_set_file_handler_cb)
{
	DESCRIBE_TEST("Unit test for rest_set_file_handler_cb");

	DEFINE_CHECK(file_request_handler_set,
		     "file_request_handler set correctly");

	RestCtx ctx = {.file_request_handler = NULL};

	rest_set_file_handler_cb((HTTPFileRequestHandler)0x123, &ctx); // NOLINT
	CHECK_INCLUDE(file_request_handler_set,
		      ctx.file_request_handler ==
			  (HTTPFileRequestHandler)0x123);

	rest_set_file_handler_cb((HTTPFileRequestHandler)NULL, &ctx); // NOLINT
	CHECK_INCLUDE(file_request_handler_set,
		      ctx.file_request_handler == (HTTPFileRequestHandler)NULL);

	rest_set_file_handler_cb(
	    (HTTPFileRequestHandler)0x1122334455667788, // NOLINT
	    &ctx);
	CHECK_INCLUDE(file_request_handler_set,
		      ctx.file_request_handler ==
			  (HTTPFileRequestHandler)0x1122334455667788);

	ASSERT_CHECK(file_request_handler_set);
}


DEFINE_TEST(test_rest_bind_addr)
{
	DESCRIBE_TEST("Unit test for rest_bind_addr");

#ifdef MOCK_TESTING
	DEFINE_CHECK(evhttp_bind_socket_called,
		     "evhttp_bind_socket called with correct arguments");
	DEFINE_CHECK(evhttp_bind_socket_return,
		     "evhttp_bind_socket returns success indication");

	int ret;
	RestCtx ctx = {.http = (struct evhttp*)0x123}; // NOLINT

	mock_data.last_http = NULL;
	mock_data.last_ip = NULL;
	mock_data.last_port = 0;
	ret = rest_bind_addr("192.168.1.1", 1234, &ctx); // NOLINT
	CHECK_INCLUDE(evhttp_bind_socket_return, ret == 0);
	CHECK_INCLUDE(evhttp_bind_socket_called,
		      mock_data.last_http == (struct evhttp*)0x123);
	CHECK_INCLUDE(evhttp_bind_socket_called,
		      strcmp(mock_data.last_ip, "192.168.1.1") == 0);
	CHECK_INCLUDE(evhttp_bind_socket_called,
		      mock_data.last_port == 1234); // NOLINT

	ASSERT_CHECK(evhttp_bind_socket_called);
	ASSERT_CHECK(evhttp_bind_socket_return);
#endif /* MOCK_TESTING */
}


DEFINE_TEST(test_rest_bind_state)
{
	DESCRIBE_TEST("Unit test for rest_bind_state");

	DEFINE_CHECK(server_ctx_set, "server_ctx set correctly");

	RestCtx ctx = {.server_ctx = NULL};

	rest_bind_state((RestCtx*)0x123, &ctx); // NOLINT
	CHECK_INCLUDE(server_ctx_set, ctx.server_ctx == (RestCtx*)0x123);

	rest_bind_state((RestCtx*)NULL, &ctx);
	CHECK_INCLUDE(server_ctx_set, ctx.server_ctx == (RestCtx*)NULL);

	rest_bind_state((RestCtx*)0x1122334455667788, &ctx); // NOLINT
	CHECK_INCLUDE(server_ctx_set,
		      ctx.server_ctx == (RestCtx*)0x1122334455667788);

	ASSERT_CHECK(server_ctx_set);
}


DEFINE_TEST(test_rest_dispatch)
{
	DESCRIBE_TEST("Unit test for rest_dispatch");

#ifdef MOCK_TESTING
	DEFINE_CHECK(dispatch_success_return,
		     "rest_dispatch returns 0 on event_base_dispatch success");
	DEFINE_CHECK(dispatch_failure_return,
		     "rest_dispatch returns -1 on event_base_dispatch failure");

	int ret;
	RestCtx ctx;

	ctx.base = (struct event_base*)0x0;
	ret = rest_dispatch(&ctx);
	CHECK_INCLUDE(dispatch_failure_return, ret == -1);

	ctx.base = (struct event_base*)0x1;
	ret = rest_dispatch(&ctx);
	CHECK_INCLUDE(dispatch_success_return, ret == 0);

	ctx.base = (struct event_base*)0x2;
	ret = rest_dispatch(&ctx);
	CHECK_INCLUDE(dispatch_success_return, ret == 0);

	ctx.base = (struct event_base*)0x3;
	ret = rest_dispatch(&ctx);
	CHECK_INCLUDE(dispatch_failure_return, ret == -1);

	ASSERT_CHECK(dispatch_success_return);
	ASSERT_CHECK(dispatch_failure_return);
#endif /* MOCK_TESTING */
}


DEFINE_TEST(test_rest_loopbreak)
{
	DESCRIBE_TEST("Unit test for rest_loopbreak");

#ifdef MOCK_TESTING
	DEFINE_CHECK(correct_args,
		     "Correct arguments passed to event_base_loopbreak");
	DEFINE_CHECK(correct_return,
		     "Correct return value returned from event_base_loopbreak");

	int ret;
	RestCtx ctx;
	mock_data.event_base = (struct event_base*)0x1;

	ctx.base = NULL;
	ret = rest_loopbreak(&ctx);
	CHECK_INCLUDE(correct_return, ret == -1);
	CHECK_INCLUDE(correct_args, mock_data.event_base == NULL);

	ctx.base = (struct event_base*)0x1;
	ret = rest_loopbreak(&ctx);
	CHECK_INCLUDE(correct_return, ret == 0);
	CHECK_INCLUDE(correct_args,
		      mock_data.event_base == (struct event_base*)0x1);

	ctx.base = (struct event_base*)0x2;
	ret = rest_loopbreak(&ctx);
	CHECK_INCLUDE(correct_return, ret == 0);
	CHECK_INCLUDE(correct_args,
		      mock_data.event_base == (struct event_base*)0x2);

	ASSERT_CHECK(correct_args);
	ASSERT_CHECK(correct_return);
#endif /* MOCK_TESTING */
}


START(test_rest_ctx_create, test_rest_ctx_destroy, test_rest_register_handlers,
      test_rest_set_file_handler_cb, test_rest_bind_addr, test_rest_bind_state,
      test_rest_dispatch, test_rest_loopbreak)
