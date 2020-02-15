#include <event2/buffer.h>
#include <event2/http.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/logger.h"
#include "register_modules.h"
#include "rest.h"


typedef struct {
	int request_counter;
	char* last_message;
} server_data;


#define MAX_BUFSZ 256


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


int handle_get(void* data, struct evhttp_request* req, int argc, char** argv)
{
	struct evbuffer* evb = evbuffer_new();
	char* arg = argc >= 1 ? argv[0] : NULL;
	server_data* server_data = data;

	evbuffer_add_printf(evb,
			    "<b>Greetings faggot, you just sent request #%d "
			    "with parameter %s</b>",
			    server_data->request_counter + 1, arg);
	evbuffer_add_printf(evb, "<br>");
	evbuffer_add_printf(evb, "<i>Last parameter received was <u>%s</u></i>",
			    server_data->last_message);

	if (server_data->last_message) {
		free(server_data->last_message);
	}
	server_data->last_message = str_n_dup(arg ? arg : ":(", MAX_BUFSZ);
	++server_data->request_counter;

	evhttp_send_reply(req, HTTP_OK, "OK :)", evb);
	evbuffer_free(evb);
	return 0;
}


int main(void)
{
	int res;
	const int port = 1234;
	server_data server_data = {0};
	RestCtx* ctx = rest_ctx_create();

	logger_log_to_stdout();
	// logger_set_levels(LOGGER_LEVEL_INFO);
	if (!ctx) {
		goto err;
	}
	rest_bind_state(&server_data, ctx);
	if ((res = rest_register_all_modules(ctx)) < 0) {
		goto err;
	}
	if ((res = rest_bind_addr("192.168.1.1", port, ctx)) < 0) {
		goto err;
	}
	if ((res = rest_dispatch(ctx)) != 0) {
		printf("Nonzero exit code %d for dispatch\n", res);
	} else {
		printf("Dispatched with exit code 0!!!\n");
	}

	rest_ctx_destroy(ctx);
	return 0;

err:
	fprintf(stderr, "Failed\n");
	rest_ctx_destroy(ctx);
	return -1;
}
