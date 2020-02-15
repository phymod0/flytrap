#include "register_modules.h"
#include "../utils/logger.h"

int rest_register_all_modules(RestCtx* ctx)
{
	LOGGER_INFO("Registering modules");

	extern const HTTPHandler captive_portal_handlers[];
	if (rest_register_handlers(captive_portal_handlers, ctx) < 0) {
		goto err;
	}

	LOGGER_INFO("Modules registered");
	return 0;

err:
	LOGGER_ERROR("Failed to register all modules");
	return -1;
}
