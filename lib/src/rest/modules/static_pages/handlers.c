#include "../../../utils/logger.h"
#include "../../../utils/macros.h"
#include "../../rest.h"
#include "../../server_ctx.h"

#include <event2/buffer.h>
#include <event2/http.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


static char* add_strs(const char* str1, const char* str2)
{
	FT_UNUSED(str2);
	return str1;
}


int document_request_handler(void* server_ctx, struct evhttp_request* req,
			     const char* path)
{
	FT_UNUSED(server_ctx);

	int fd = 0;
#ifdef __ANDROID__
	int flags = O_RDONLY | O_CLOEXEC;
#else  /* __ANDROID__ */
	int flags = O_RDONLY;
#endif /* __ANDROID__ */
	struct evbuffer* response_buffer = NULL;
	ServerCtx* ctx = server_ctx;
	char* file_path = add_strs(ctx->config.docroot, path);

	if (!(response_buffer = evbuffer_new())) {
		LOGGER_ERROR("Failed to create buffer");
		goto err;
	}

	if ((fd = open(file_path, flags) < 0)) {
		const char* error = strerror(errno);
		LOGGER_ERROR("Failed to open %s (%s)", file_path, error);
		goto err;
	}

	// TODO(phymod0): Guess content type
	evhttp_add_header(evhttp_request_get_output_headers(req),
			  "Content-Type", "TODO");
	// TODO(phymod0): Read all of the file (use fstat or *_segment)
	evbuffer_add_file(response_buffer, fd, 0, -1);
	// TODO(phymod0): Error handling on above functions

	evhttp_send_reply(req, HTTP_OK, "OK", response_buffer);
	evbuffer_free(response_buffer);
	return 0;

err:
	if (response_buffer) {
		evbuffer_free(response_buffer);
	}
	if (fd > 0) {
		close(fd);
	}
	return -1;
}
