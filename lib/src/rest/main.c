#include <event2/buffer.h>
#include <event2/http.h>
#include <stdio.h>

#include "../utils/logger.h"
#include "rest.h"


typedef struct {
	int request_counter;
} server_data;


int handle_get(void* data, struct evhttp_request* req, int path_argc,
	       char** path_argv)
{
	struct evbuffer* evb = evbuffer_new();
	evbuffer_add_printf(evb, "<h1>Greetings faggot</h1>");
	server_data* server_data = data;
	++server_data->request_counter;
	evhttp_send_reply(req, HTTP_OK, "OK :)", evb);
	evbuffer_free(evb);
	printf("Had argc: %d\n", path_argc);
	return 0;
	(void)path_argc;
	(void)path_argv;
}


int main(void)
{
	int res;
	const int port = 1234;
	server_data server_data = {0};
	RestCtx* ctx = rest_ctx_create();

	logger_log_to_stdout();
	if (!ctx) {
		goto err;
	}
	rest_bind_state(&server_data, ctx);
	res = rest_register_handlers(
	    (HTTPHandler[]){
		{
		    "/api/login/<?>/lol",
		    {
			.GET = handle_get,
		    },
		},
		REST_END_HANDLERS,
	    },
	    ctx);
	if (res < 0) {
		goto err;
	}
	if ((res = rest_bind_addr("localhost", port, ctx)) < 0) {
		goto err;
	}
	if ((res = rest_dispatch(ctx)) != 0) {
		printf("Nonzero exit code %d for dispatch\n", res);
	} else {
		printf("Dispatched with exist code 0!!!\n");
	}

	rest_ctx_destroy(ctx);
	return 0;

err:
	fprintf(stderr, "Failed\n");
	rest_ctx_destroy(ctx);
	return -1;
}
