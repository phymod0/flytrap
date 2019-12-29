#include "eventBase.hpp"

#include <stdexcept>


static event_base* createEventBase()
{
	event_base* evBase = event_base_new();
	if (not evBase) {
		throw std::runtime_error("Failed to create event base");
	}
	return evBase;
}


static void destroyEventBase(event_base* eventBase)
{
	if (eventBase)
		event_base_free(eventBase);
}


namespace LibEvent
{
EventBase::EventBase() : eventBase(createEventBase(), destroyEventBase) {}


void EventBase::dispatch() { event_base_dispatch(eventBase.get()); }
} // namespace LibEvent
