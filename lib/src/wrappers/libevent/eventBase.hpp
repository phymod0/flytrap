#ifndef WRAPPER_LIBEVENT_EVBASE
#define WRAPPER_LIBEVENT_EVBASE


#include <event2/event.h>

#include <memory>


namespace LibEvent
{
class EventBase
{
      public:
	EventBase();
	void dispatch();
	event_base* c_event_base() const;

      private:
	std::unique_ptr<event_base, void (*)(event_base*)> eventBase;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_EVBASE */
