#include "../../../utils/macros.h"
#include "../../rest.h"

#include <event2/buffer.h>
#include <event2/http.h>


static int probe_handler(void* server_ctx, struct evhttp_request* req,
			 int path_argc, char** path_argv)
{
	(void)(server_ctx);
	(void)(path_argc);
	(void)(path_argv);

	int ret;
	struct evbuffer* response_buffer = NULL;

	response_buffer = evbuffer_new();
	ret = evhttp_add_header(evhttp_request_get_output_headers(req),
				"location", "https://www.facebook.com/");
	if (ret == -1) {
		goto err;
	}
	evhttp_send_reply(req, HTTP_MOVETEMP, "Found", response_buffer);
	evbuffer_free(response_buffer);
	return 0;

err:
	if (response_buffer) {
		evbuffer_free(response_buffer);
	}
	return -1;
}


const HTTPHandler captive_portal_handlers[] = {
    // Android/Chrome
    {"/generate_204", {.GET = probe_handler}},
    // iOS
    {"/hotspot-detect.html", {.GET = probe_handler}},
    // TODO(phymod0): Windows
    // TODO(phymod0): OSX
    // TODO(phymod0): Other android devices?
    REST_END_HANDLERS,
};
