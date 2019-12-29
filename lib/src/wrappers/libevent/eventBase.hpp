#ifndef WRAPPER_LIBEVENT_EVBASE
#define WRAPPER_LIBEVENT_EVBASE


#ifndef INCLUDED_BY_LIBEVENT
#error Do not include eventBase.hpp directly
#endif /* INCLUDED_BY_LIBEVENT */


#include <event2/event.h>

#include <memory>


namespace LibEvent
{
class EventBase
{
	friend class Http;

      public:
	EventBase();
	void dispatch();

      private:
	std::unique_ptr<event_base, void (*)(event_base*)> eventBase;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_EVBASE */
