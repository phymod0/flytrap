#ifndef WRAPPER_LIBEVENT_HTTP
#define WRAPPER_LIBEVENT_HTTP


#ifndef INCLUDED_BY_LIBEVENT
#error Do not include http.hpp directly
#endif /* INCLUDED_BY_LIBEVENT */


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
	using HandlerFn = std::function<void(Request& req)>;
	Http(const EventBase& evBase);
	void setHandler(HandlerFn handlerFn);
	void bind(const std::string& ip, int port);

      private:
	std::unique_ptr<evhttp, void (*)(evhttp*)> evHttp;
	HandlerFn handler;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_HTTP */
