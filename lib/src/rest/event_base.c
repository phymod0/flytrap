#include <event2/event.h>
#include <event2/http.h>
#include <stdlib.h>

#include "../adt/string_tree/string_tree.h"
#include "../utils/logger.h"
#include "event_base.h"


struct RestCtx {
	struct event_base* base;
	struct evhttp* http;
	StringTree* handlers;
	void* server_ctx;
};


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


/* TODO: Implement remaining methods */
