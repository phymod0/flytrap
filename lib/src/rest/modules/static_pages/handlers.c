#include "../../../utils/logger.h"
#include "../../../utils/macros.h"
#include "../../server_ctx.h"

#include <event2/buffer.h>
#include <event2/http.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


static const struct table_entry {
	const char* extension;
	const char* content_type;
} content_type_table[] = {
    {"txt", "text/plain"},
    {"c", "text/plain"},
    {"h", "text/plain"},
    {"html", "text/html"},
    {"htm", "text/htm"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"pdf", "application/pdf"},
    {"ps", "application/postscript"},
    {"js", "application/javascript"},
    {NULL, NULL},
};


static const char* guess_content_type(const char* path)
{
	const char* last_period;
	const char* extension;
	const struct table_entry* ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/')) {
		goto not_found;
	}
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		size_t extension_strlen = strlen(extension);
		if (!strncasecmp(ent->extension, extension, extension_strlen)) {
			return ent->content_type;
		}
	}

not_found:
	return "application/misc";
}


static char* add_strs(const char* str1, const char* str2)
{
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	char* result = malloc(len1 + len2 + 1);
	if (!result) {
		return NULL;
	}
	strncpy(result, str1, len1);
	strncpy(result + len1, str2, len2);
	return result;
}


int document_request_handler(void* server_ctx, struct evhttp_request* req,
			     const char* path)
{
	FT_UNUSED(server_ctx);

	int fd = 0;
	int open_flags = O_RDONLY;
	struct evbuffer* response_buffer = NULL;
	ServerCtx* ctx = server_ctx;
	char* file_path = NULL;

	if (!(file_path = add_strs(ctx->config.docroot, path))) {
		LOGGER_ERROR("Out of memory");
		goto err;
	}

	if (!(response_buffer = evbuffer_new())) {
		LOGGER_ERROR("Failed to create buffer");
		goto err;
	}

#ifdef __ANDROID__
	open_flags |= O_CLOEXEC;
#endif /* __ANDROID__ */
	if ((fd = open(file_path, open_flags) < 0)) {
		const char* error = strerror(errno);
		LOGGER_ERROR("Failed to open %s (%s)", file_path, error);
		goto err;
	}

	if (evhttp_add_header(evhttp_request_get_output_headers(req),
			      "Content-Type", guess_content_type(path)) == -1) {
		LOGGER_ERROR("Failed to include content type");
		goto err;
	}

	if (evbuffer_add_file(response_buffer, fd, 0, -1) == -1) {
		LOGGER_ERROR("Failed to add file");
		goto err;
	}

	free(file_path);
	evhttp_send_reply(req, HTTP_OK, "OK", response_buffer);
	evbuffer_free(response_buffer);
	return 0;

err:
	free(file_path);
	if (response_buffer) {
		evbuffer_free(response_buffer);
	}
	if (fd > 0) {
		close(fd);
	}
	return -1;
}
