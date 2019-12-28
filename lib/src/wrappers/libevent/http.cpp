#include "http.hpp"


static evhttp* createHttp(const LibEvent::EventBase& evBase)
{
	evhttp* evHttp = evhttp_new(evBase.c_event_base());
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
Http::Http(const EventBase& evBase, std::string ip, int port)
    : evHttp(createHttp(evBase), destroyHttp), ip(std::move(ip)), port(port)
{
}


void Http::setHandler(HandlerFn handlerFn)
{
	handler = std::move(handlerFn);
	evhttp_set_gencb(evHttp.get(), callHandler, &handler);
}
} // namespace LibEvent
