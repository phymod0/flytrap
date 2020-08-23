#include "../../../utils/logger.h"
#include "../../../utils/macros.h"
#include "../../rest.h"

#include <event2/buffer.h>
#include <event2/http.h>


/*
 * TODO(phymod0):
 *	- File hierarchy for handlers?
 *	- Function to return JSON data from an evhttp_request struct
 *	- Describe JSON parameters in each handler's POST data
 */


/* Handlers for all interaction types */
static int page_land_handler(void* server_ctx, struct evhttp_request* req,
			     int path_argc, char** path_argv);
static int value_change_handler(void* server_ctx, struct evhttp_request* req,
				int path_argc, char** path_argv);
static int page_focus_handler(void* server_ctx, struct evhttp_request* req,
			      int path_argc, char** path_argv);
static int manual_exit_handler(void* server_ctx, struct evhttp_request* req,
			       int path_argc, char** path_argv);
static int idle_timeout_handler(void* server_ctx, struct evhttp_request* req,
				int path_argc, char** path_argv);
static int mouse_move_handler(void* server_ctx, struct evhttp_request* req,
			      int path_argc, char** path_argv);
static int component_click_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv);
static int component_touch_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv);
static int component_focus_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv);
static int form_submission_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv);


static int page_land_handler(void* server_ctx, struct evhttp_request* req,
			     int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Page landing handler not yet implemented");
	return 0;
}


static int value_change_handler(void* server_ctx, struct evhttp_request* req,
				int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Value change handler not yet implemented");
	return 0;
}


static int page_focus_handler(void* server_ctx, struct evhttp_request* req,
			      int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Page focus handler not yet implemented");
	return 0;
}


static int manual_exit_handler(void* server_ctx, struct evhttp_request* req,
			       int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Manual exit handler not yet implemented");
	return 0;
}


static int idle_timeout_handler(void* server_ctx, struct evhttp_request* req,
				int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Idle timeout handler not yet implemented");
	return 0;
}


static int mouse_move_handler(void* server_ctx, struct evhttp_request* req,
			      int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Mouse move handler not yet implemented");
	return 0;
}


static int component_click_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Component click handler not yet implemented");
	return 0;
}


static int component_touch_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Component touch handler not yet implemented");
	return 0;
}


static int component_focus_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Component focus handler not yet implemented");
	return 0;
}


static int form_submission_handler(void* server_ctx, struct evhttp_request* req,
				   int path_argc, char** path_argv)
{
	FT_UNUSED(server_ctx);
	FT_UNUSED(req);
	FT_UNUSED(path_argc);
	FT_UNUSED(path_argv);

	LOGGER_WARN("Form submission handler not yet implemented");
	return 0;
}


#ifndef INTERACTION_ROUTE_PREFIX
#define INTERACTION_ROUTE_PREFIX "/interaction"
#endif /* INTERACTION_ROUTE_PREFIX */

const HTTPHandler interaction_handlers[] = {
    /**
     * Landed for the first time in the session
     */
    {INTERACTION_ROUTE_PREFIX "/land", {.POST = page_land_handler}},

    /**
     * Changed an input field
     */
    {INTERACTION_ROUTE_PREFIX "/edit", {.POST = value_change_handler}},

    /**
     * Focused or blurred the main page
     */
    {INTERACTION_ROUTE_PREFIX "/active", {.POST = page_focus_handler}},

    /**
     * Exited manually
     */
    {INTERACTION_ROUTE_PREFIX "/quit", {.POST = manual_exit_handler}},

    /**
     * Did nothing on an active page for some time
     */
    {INTERACTION_ROUTE_PREFIX "/idle", {.POST = idle_timeout_handler}},

    /**
     * Moved the mouse (accurate to the component)
     */
    {INTERACTION_ROUTE_PREFIX "/mouse_move", {.POST = mouse_move_handler}},

    /**
     * Clicked on a component
     */
    {INTERACTION_ROUTE_PREFIX "/click", {.POST = component_click_handler}},

    /**
     * Touched a component
     */
    {INTERACTION_ROUTE_PREFIX "/touch", {.POST = component_touch_handler}},

    /**
     * Focused or blurred a component
     */
    {INTERACTION_ROUTE_PREFIX "/focus", {.POST = component_focus_handler}},

    /**
     * Submitted a form
     */
    {INTERACTION_ROUTE_PREFIX "/submit", {.POST = form_submission_handler}},

    REST_END_HANDLERS,
};
