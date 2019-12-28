#ifndef WRAPPER_LIBEVENT_HTTP
#define WRAPPER_LIBEVENT_HTTP


#include "eventBase.hpp"
#include "http.hpp"
#include "request.hpp"

#include <event2/http.h>

#include <functional>
#include <memory>


namespace LibEvent
{
class Http
{
      public:
	using HandlerFn = std::function<void(Request)>;
	Http(const EventBase& evBase, std::string ip, int port);
	void setHandler(HandlerFn handlerFn);

      private:
	std::unique_ptr<evhttp, void (*)(evhttp*)> evHttp;
	HandlerFn handler;
	std::string ip;
	int port;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_HTTP */
