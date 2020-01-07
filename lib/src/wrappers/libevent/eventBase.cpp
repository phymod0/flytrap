#define INCLUDED_BY_LIBEVENT
#include "eventBase.hpp"
#undef INCLUDED_BY_LIBEVENT

#include <stdexcept>


using HandlerMap = std::map<int, LibEvent::EventBase::SignalHandlerFn>;
using EventSmartPtr = std::unique_ptr<event, decltype(&event_free)>;


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


static void executeSignalHandler(int signal, short events, void* cbData)
{
	const HandlerMap& handlers = *(HandlerMap*)cbData;
	handlers.at(signal)();
}


namespace LibEvent
{
EventBase::EventBase() : eventBase(createEventBase(), destroyEventBase) {}


void EventBase::setSignalHandler(int signal, EventBase::SignalHandlerFn handler)
{
	handlers[signal] = handler;
	event* term = evsignal_new(eventBase.get(), signal,
				   executeSignalHandler, &handlers);
	event_add(term, NULL);
	signalEvents.push_back(EventSmartPtr(term, event_free));
}


void EventBase::dispatch() { event_base_dispatch(eventBase.get()); }


void EventBase::loopBreak() { event_base_loopbreak(eventBase.get()); }
} // namespace LibEvent
