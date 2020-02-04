#include <event2/event.h>
#include <event2/http.h>
#include <stdlib.h>
#include <string.h>

#include "../adt/string_tree/string_tree.h"
#include "../config.h"
#include "../utils/logger.h"
#include "event_base.h"


struct RestCtx {
	struct event_base* base;
	struct evhttp* http;
	StringTree* handlers;
	void* server_ctx;
};


static char* str_n_dup(const char* str, size_t n);
static StringTree* get_path_subtree(StringTree* tree, const char* path);
static int register_single_handler(const HTTPHandler* handler, RestCtx* ctx);
static void generic_handler_callback(struct evhttp_request* req, void* data);


RestCtx* rest_ctx_create()
{
	RestCtx* ctx = NULL;
	struct event_base* base = NULL;
	struct evhttp* http = NULL;
	StringTree* handlers = NULL;

	ctx = malloc(sizeof *ctx);
	base = event_base_new();
	http = evhttp_new(base);
	handlers = string_tree_create();
	if (!ctx || !base || !http || !handlers) {
		goto err;
	}

	ctx->base = base;
	evhttp_set_gencb(http, generic_handler_callback, ctx);
	ctx->http = http;
	ctx->handlers = handlers;
	ctx->server_ctx = NULL;
	return ctx;

err:
	LOGGER_ERROR("Couldn't create REST context");
	string_tree_destroy(handlers);
	if (http) {
		evhttp_free(http);
	}
	if (base) {
		event_base_free(base);
	}
	free(ctx);
	return NULL;
}


void rest_ctx_destroy(RestCtx* ctx)
{
	string_tree_destroy(ctx->handlers);
	if (ctx->http) {
		evhttp_free(ctx->http);
	}
	if (ctx->base) {
		event_base_free(ctx->base);
	}
	free(ctx);
}


int rest_register_handlers(const HTTPHandler handlers[], RestCtx* ctx)
{
	for (const HTTPHandler* handler = handlers; handler->route; ++handler) {
		if (register_single_handler(handler, ctx) < 0) {
			LOGGER_WARN("Could not register some handlers");
			return -1;
		}
		LOGGER_INFO("Registered route %s with the following "
			    "handlers:%s%s%s%s",
			    handler->route, handler->methods.PUT ? " PUT" : "",
			    handler->methods.GET ? " GET" : "",
			    handler->methods.POST ? " POST" : "",
			    handler->methods.DELETE ? " DELETE" : "");
	}
	return 0;
}


int rest_bind_addr(const char* ip, int port, RestCtx* ctx)
{
	return evhttp_bind_socket(ctx->http, ip, port);
}


void rest_bind_state(void* server_ctx, RestCtx* ctx)
{
	ctx->server_ctx = server_ctx;
}


int rest_dispatch(RestCtx* ctx)
{
	/* Not implemented */
	/* TODO: Method for setting generic callback and signal handler */
	/* TODO: init() in ctor? */
	return -1;
}


int rest_loopbreak(RestCtx* ctx)
{
	/* Not implemented */
	return -1;
}


static char* str_n_dup(const char* str, size_t n)
{
	size_t len = strlen(str);
	size_t min = len < n ? len : n;
	char* result;
	if ((result = malloc(min + 1))) {
		memcpy(result, str, min);
		result[min] = '\0';
	}
	return result;
}


static StringTree* get_path_subtree(StringTree* tree, const char* path)
{
	char* _path = str_n_dup(path, REST_PATH_MAXSZ);
	if (!_path) {
		goto err;
	}

	for (char* tok = strtok(_path, "/"); tok; tok = strtok(NULL, "/")) {
		if (!tree) {
			goto err;
		}
		tree = string_tree_get_subtree(tree, tok);
	}
	free(_path);
	return tree;

err:
	free(_path);
	return NULL;
}


static int register_single_handler(const HTTPHandler* handler, RestCtx* ctx)
{
	const char* path = handler->route;
	if (strlen(path) > REST_PATH_MAXSZ) {
		LOGGER_WARN("Path %s will be truncated", path);
	}

	StringTree* tree = get_path_subtree(ctx->handlers, path);
	if (!tree) {
		LOGGER_ERROR("Failed to make subtree for path %s", path);
		return -1;
	}
	if (string_tree_get_value(tree)) {
		LOGGER_WARN("Handlers for path %s already registered", path);
	}

	string_tree_set_value(tree, (HTTPMethods*)&handler->methods);
	return 0;
}


static void generic_handler_callback(struct evhttp_request* req, void* data)
{
	RestCtx* ctx = data;
	StringTree* handlers = ctx->handlers;
	void* server_ctx = ctx->server_ctx;

	/* TODO: Call correct handler */
	(void)req;
	(void)handlers;
	(void)server_ctx;
}
