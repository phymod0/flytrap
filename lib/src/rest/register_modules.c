#include "register_modules.h"
#include "../utils/logger.h"


#define REGISTER_MODULE_HANDLERS(handlers)                                     \
	{                                                                      \
		extern const HTTPHandler handlers[];                           \
		if (rest_register_handlers(handlers, ctx) != 0) {              \
			goto err;                                              \
		}                                                              \
	}


int rest_register_all_modules(RestCtx* ctx)
{
	LOGGER_INFO("Registering modules");

	REGISTER_MODULE_HANDLERS(captive_portal_handlers);
	REGISTER_MODULE_HANDLERS(interaction_handlers);

	int document_request_handler(void*, struct evhttp_request*,
				     const char*);
	rest_set_file_handler_cb(document_request_handler, ctx);

	LOGGER_INFO("Modules registered");
	return 0;

err:
	LOGGER_ERROR("Failed to register all modules");
	return -1;
}


#undef REGISTER_MODULE
