#include <event2/event.h>
#include <event2/http.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "../adt/string_tree/string_tree.h"
#include "../config.h"
#include "../utils/logger.h"
#include "../utils/macros.h"
#include "rest.h"


struct RestCtx {
	struct event_base* base;
	struct event* term;
	struct evhttp* http;
	StringTree* handlers;
	HTTPFileRequestHandler file_request_handler;
	void* server_ctx;
};


static char* str_ndup(const char* str, size_t n);
static StringTree* get_path_subtree(StringTree* tree, const char* path);
static char** path_argv_create();
static void path_argv_destroy(char** argv);
static StringTree* find_path_subtree(StringTree* tree, const char* path,
				     int* path_argc, char*** path_argv);
static int register_single_handler(const HTTPHandler* handler, RestCtx* ctx);
static void handle_signal(int sig, short events, void* data);
static const char* get_method_str(enum evhttp_cmd_type method);
static char* get_request_path(struct evhttp_request* req);
static int validate_path(const char* path);
static void document_request_cb(struct evhttp_request* req, const char* path,
				RestCtx* ctx);
static void generic_handler_cb(struct evhttp_request* req, void* data);
static struct event_base* event_base_create_and_init();


RestCtx* rest_ctx_create()
{
	RestCtx* ctx = NULL;
	struct event_base* base = NULL;
	struct event* term = NULL;
	struct evhttp* http = NULL;
	StringTree* handlers = NULL;

	ctx = malloc(sizeof *ctx);
	base = event_base_create_and_init(&term);
	http = evhttp_new(base);
	handlers = string_tree_create();
	if (!ctx || !base || !http || !handlers) {
		goto err;
	}

	ctx->base = base;
	ctx->term = term;
	evhttp_set_gencb(http, generic_handler_cb, ctx);
	ctx->http = http;
	ctx->handlers = handlers;
	ctx->file_request_handler = NULL;
	ctx->server_ctx = NULL;
	return ctx;

err:
	LOGGER_ERROR("Couldn't create REST context");
	string_tree_destroy(handlers);
	if (http) {
		evhttp_free(http);
	}
	if (term) {
		event_free(term);
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
	if (ctx->term) {
		event_free(ctx->term);
	}
	if (ctx->base) {
		event_base_free(ctx->base);
	}
	free(ctx);
}


int rest_register_handlers(const HTTPHandler handlers[], RestCtx* ctx)
{
	for (const HTTPHandler* handler = handlers; handler->path; ++handler) {
		if (register_single_handler(handler, ctx) < 0) {
			LOGGER_WARN("Could not register some handlers");
			return -1;
		}
		LOGGER_INFO("Registered path %s with the following "
			    "handlers:%s%s%s%s",
			    handler->path, handler->methods.PUT ? " PUT" : "",
			    handler->methods.GET ? " GET" : "",
			    handler->methods.POST ? " POST" : "",
			    handler->methods.DELETE ? " DELETE" : "");
	}
	return 0;
}


void rest_set_file_handler_cb(HTTPFileRequestHandler handler, RestCtx* ctx)
{
	ctx->file_request_handler = handler;
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
	switch (event_base_dispatch(ctx->base)) {
	case 0:
	case 1:
		LOGGER_INFO("Event loop dispatch was successful");
		return 0;
	}
	LOGGER_DEBUG("Failed to dispatch event loop");
	return -1;
}


int rest_loopbreak(RestCtx* ctx)
{
	if (event_base_loopbreak(ctx->base) < 0) {
		LOGGER_ERROR("Failed to break event loop");
		return -1;
	}
	LOGGER_INFO("Terminating event loop");
	return 0;
}


static char* str_ndup(const char* str, size_t n)
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
	char* _path = str_ndup(path, REST_PATH_MAXSZ);
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
	LOGGER_ERROR("Out of memory");
	free(_path);
	return NULL;
}


static char** path_argv_create()
{
	return calloc(REST_PATH_MAX_WILDCARDS, sizeof(char*));
}


static void path_argv_destroy(char** argv)
{
	if (!argv) {
		return;
	}
	for (int i = 0; i < REST_PATH_MAX_WILDCARDS && argv[i] != NULL; ++i) {
		free(argv[i]);
	}
	free(argv);
}


static StringTree* find_path_subtree(StringTree* tree, const char* path,
				     int* path_argc, char*** path_argv)
{
	const char* wc = REST_PATH_SEGMENT_WILDCARD;
	char* dup_path = NULL;
	char** argv = NULL;
	int argc;

	if (!(dup_path = str_ndup(path, REST_PATH_MAXSZ))) {
		goto oom;
	}
	if (!(argv = path_argv_create())) {
		goto oom;
	}

	argc = 0;
	for (char* tok = strtok(dup_path, "/"); tok; tok = strtok(NULL, "/")) {
		StringTree* subtree;
		if (!tree) {
			LOGGER_INFO("No match in path tree for %s", path);
			break;
		}
		subtree = string_tree_find_subtree(tree, tok);
		if (subtree) {
			tree = subtree;
			continue;
		}
		if (!(tree = string_tree_find_subtree(tree, wc))) {
			continue;
		}
		if (argc >= REST_PATH_MAX_WILDCARDS) {
			LOGGER_WARN("Wildcard table is full");
			continue;
		}
		if (!(argv[argc++] = str_ndup(tok, REST_PATH_MAXSZ))) {
			goto oom;
		}
	}
	*path_argc = argc;
	*path_argv = argv;
	free(dup_path);
	return tree;

oom:
	LOGGER_ERROR("Out of memory");
	path_argv_destroy(argv);
	free(dup_path);
	return NULL;
}


static int register_single_handler(const HTTPHandler* handler, RestCtx* ctx)
{
	const char* path = handler->path;
	void* methods = (void*)&handler->methods;

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

	string_tree_set_value(tree, methods);
	LOGGER_INFO("Registered methods at %p for path %s", methods, path);
	return 0;
}


static void handle_signal(int sig, short events, void* data)
{
	FT_UNUSED(sig);
	FT_UNUSED(events);

	struct event_base* base = data;
	event_base_loopbreak(base);
	LOGGER_INFO("Terminating event loop due to signal %i", sig);
}


static const char* get_method_str(enum evhttp_cmd_type method)
{
	switch (method) {
	case EVHTTP_REQ_GET:
		return "GET";
	case EVHTTP_REQ_POST:
		return "POST";
	case EVHTTP_REQ_HEAD:
		return "HEAD";
	case EVHTTP_REQ_PUT:
		return "PUT";
	case EVHTTP_REQ_DELETE:
		return "DELETE";
	case EVHTTP_REQ_OPTIONS:
		return "OPTIONS";
	case EVHTTP_REQ_TRACE:
		return "TRACE";
	case EVHTTP_REQ_CONNECT:
		return "CONNECT";
	case EVHTTP_REQ_PATCH:
		return "PATCH";
	default:
		return "unknown";
	}
}


static char* get_request_path(struct evhttp_request* req)
{
	const char* uri;
	struct evhttp_uri* decoded = NULL;
	const char* path;
	char* decoded_path = NULL;

	uri = evhttp_request_get_uri(req);
	if (!(decoded = evhttp_uri_parse(uri))) {
		goto err;
	}
	path = evhttp_uri_get_path(decoded);
	if (!(decoded_path = evhttp_uridecode(path, 0, NULL))) {
		goto err;
	}
	evhttp_uri_free(decoded);
	return decoded_path;

err:
	if (decoded) {
		evhttp_uri_free(decoded);
	}
	if (decoded_path) {
		free(decoded_path);
	}
	LOGGER_ERROR("Failed to read request path");
	return NULL;
}


static int validate_path(const char* path)
{
	/* TODO(phymod0): Find a better way */
	return strstr(path, "..") ? -1 : 0;
}


static void document_request_cb(struct evhttp_request* req, const char* path,
				RestCtx* ctx)
{
	HTTPFileRequestHandler handler = ctx->file_request_handler;
	if (!handler) {
		LOGGER_INFO("Rejecting file request (not implemented)");
		evhttp_send_error(req, HTTP_BADMETHOD, "Not supported");
	} else if (validate_path(path) < 0) {
		LOGGER_INFO("Rejected path %s", path);
		evhttp_send_error(req, HTTP_BADREQUEST, "Bad request");
	} else if (handler(ctx->server_ctx, req, path) != 0) {
		LOGGER_ERROR("Exception in document request handler");
		evhttp_send_error(req, HTTP_INTERNAL, "Internal error");
	} else {
		LOGGER_INFO("Document request handler ran without errors");
	}
}


static void generic_handler_cb(struct evhttp_request* req, void* data)
{
	RestCtx* ctx;
	struct evhttp_connection* conn;
	enum evhttp_cmd_type method;
	const char* method_str;
	char* peer_ip;
	ev_uint16_t peer_port;
	char* path = NULL;
	int argc = 0;
	char** argv = NULL;
	StringTree* handlers;
	HTTPMethods* methods;
	HTTPMethod method_fn;

#if LOGGING_ENABLE == 0
	FT_UNUSED(method_str);
#endif /* LOGGING_ENABLE */

	ctx = data;
	conn = evhttp_request_get_connection(req);
	method = evhttp_request_get_command(req);
	method_str = get_method_str(method);
	evhttp_connection_get_peer(conn, &peer_ip, &peer_port);
	if (!(path = get_request_path(req))) {
		LOGGER_ERROR("Failed to serve request");
		evhttp_send_error(req, HTTP_BADREQUEST, "Bad request");
		goto done;
	}

	LOGGER_INFO("Received %s request for %s from %s:%i", method_str, path,
		    peer_ip, peer_port);

	if (!(handlers =
		  find_path_subtree(ctx->handlers, path, &argc, &argv)) ||
	    !(methods = string_tree_get_value(handlers))) {
		LOGGER_DEBUG("Treating %s as a path to a document", path);
		document_request_cb(req, path, ctx);
		goto done;
	}

	switch (method) {
	case EVHTTP_REQ_GET:
		method_fn = methods->GET;
		break;
	case EVHTTP_REQ_POST:
		method_fn = methods->POST;
		break;
	case EVHTTP_REQ_PUT:
		method_fn = methods->PUT;
		break;
	case EVHTTP_REQ_DELETE:
		method_fn = methods->DELETE;
		break;
	default:
		method_fn = NULL;
	}

	if (!method_fn) {
		LOGGER_INFO("Requested method %s not implemented for path %s",
			    method_str, path);
		evhttp_send_error(req, HTTP_BADMETHOD, "Bad method");
		goto done;
	}

	if (method_fn(ctx->server_ctx, req, argc, argv) != 0) {
		LOGGER_INFO("%s %s raised an internal error", method_str, path);
		evhttp_send_error(req, HTTP_INTERNAL, "Internal error");
		goto done;
	}
	LOGGER_INFO("%s %s succeeded with return value 0", method_str, path);

done:
	path_argv_destroy(argv);
	free(path);
}


static struct event_base* event_base_create_and_init(struct event** sigterm)
{
	struct event_base* base = NULL;
	struct event* term = NULL;

	base = event_base_new();
	term = evsignal_new(base, SIGINT, handle_signal, base);
	if (!base || !term || event_add(term, NULL)) {
		goto err;
	}

	*sigterm = term;
	return base;

err:
	if (term) {
		event_free(term);
	}
	if (base) {
		event_base_free(base);
	}
	return NULL;
}
