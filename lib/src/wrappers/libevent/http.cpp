#define INCLUDED_BY_LIBEVENT
#include "http.hpp"
#undef INCLUDED_BY_LIBEVENT


static evhttp* createHttp(event_base* base)
{
	evhttp* evHttp = evhttp_new(base);
	if (not evHttp) {
		throw std::runtime_error("Failed to create http server");
	}
	return evHttp;
}


static void destroyHttp(evhttp* evHttp)
{
	if (evHttp)
		evhttp_free(evHttp);
}


static void callHandler(evhttp_request* req, void* data)
{
	LibEvent::Http::HandlerFn handler = *(LibEvent::Http::HandlerFn*)data;
	handler(LibEvent::Request(req));
}


namespace LibEvent
{
Http::Http(const EventBase& evBase)
    : evHttp(createHttp(evBase.eventBase.get()), destroyHttp)
{
}


void Http::setHandler(HandlerFn handlerFn)
{
	handler = std::move(handlerFn);
	evhttp_set_gencb(evHttp.get(), callHandler, &handler);
}


void Http::bind(const std::string& ip, int port)
{
	evhttp_bind_socket(evHttp.get(), ip.c_str(), port);
}
} // namespace LibEvent
