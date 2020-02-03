#ifndef REST_EVENT_BASE
#define REST_EVENT_BASE


#include <event2/http.h>


/**
 * HTTP handler type.
 *
 * @param server_ctx Server state passed to <code>rest_bind_state</code>
 * @param request libevent request object for the HTTP request in context
 * @param route_argc Number of substituted path wildcards
 * @param route_argv Substituted path wildcards
 * @returns 0 on successful execution, or -1 to indicate a 5XX error
 */
typedef int (*HTTPMethod)(void* server_ctx, struct evhttp_request* request,
			  int route_argc, const char** route_argv);

/** Set of HTTP methods */
typedef struct {
	HTTPMethod POST;
	HTTPMethod GET;
	HTTPMethod PUT;
	HTTPMethod DELETE;
} HTTPMethods;

/** Set of HTTP methods for a particular route */
typedef struct {
	const char* route;
	HTTPMethods methods;
} HTTPHandler;

/** Reserved HTTPHandler object to mark the end of an array */
#define REST_END_HANDLERS                                                      \
	(HTTPHandler) { .route = NULL }

/** REST event base context */
struct RestCtx;
#ifndef REST_FWD
#define REST_FWD
typedef struct RestCtx RestCtx;
#endif /* REST_FWD */


/**
 * Create a REST event base context.
 *
 * @returns Context or NULL if out of memory
 */
RestCtx* rest_ctx_create();

/**
 * Destroy a REST event base context.
 *
 * @param ctx REST event base context to destroy
 */
void rest_ctx_destroy(RestCtx* ctx);

/**
 * Register all routes to handle.
 *
 * @param handlers Array of <code>HTTPHandler</code> objects
 * @returns 0 on success, -1 on failure
 */
int rest_register_handlers(const HTTPHandler handlers[], RestCtx* ctx);

/**
 * Bind a REST event base to a TCP IP/port.
 *
 * @param ip IPv4 address to bind to
 * @param port Port number
 * @param ctx REST event base context
 * @return 0 on success, -1 on failure
 */
int rest_bind_addr(const char* ip, int port, RestCtx* ctx);

/**
 * Bind a REST event base to a server state.
 *
 * @param server_ctx Server state
 * @param ctx REST event base context
 */
void rest_bind_state(void* server_ctx, RestCtx* ctx);

/**
 * Start the REST server.
 *
 * @param ctx REST event base context
 * @returns 0 on success, -1 on failure
 */
int rest_dispatch(RestCtx* ctx);

/**
 * Stop the REST server.
 *
 * @param ctx REST event base context
 * @returns 0 on success, -1 on failure
 */
int rest_loopbreak(RestCtx* ctx);


#endif /* REST_EVENT_BASE */
