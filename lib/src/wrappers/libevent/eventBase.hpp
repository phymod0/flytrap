#ifndef WRAPPER_LIBEVENT_EVBASE
#define WRAPPER_LIBEVENT_EVBASE


#ifndef INCLUDED_BY_LIBEVENT
#error Do not include eventBase.hpp directly
#endif /* INCLUDED_BY_LIBEVENT */


#include <event2/event.h>

#include <functional>
#include <list>
#include <map>
#include <memory>


namespace LibEvent
{
class EventBase
{
	friend class Http;

      public:
	using SignalHandlerFn = std::function<void(void)>;
	EventBase();
	void setSignalHandler(int signal, SignalHandlerFn handler);
	void dispatch();
	void loopBreak();

      private:
	using HandlerMap = std::map<int, SignalHandlerFn>;
	using EventSmartPtr = std::unique_ptr<event, decltype(&event_free)>;
	std::unique_ptr<event_base, void (*)(event_base*)> eventBase;
	HandlerMap handlers;
	std::list<EventSmartPtr> signalEvents;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_EVBASE */
