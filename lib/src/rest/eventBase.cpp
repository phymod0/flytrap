#include "eventBase.hpp"


namespace REST
{
EventBase::EventBase(const EventBase& evBase) = default;


EventBase& EventBase::operator=(const EventBase& evBase) = default;


EventBase::EventBase(EventBase&& evBase) noexcept = default;


EventBase& EventBase::operator=(EventBase&& evBase) noexcept = default;


EventBase::~EventBase() = default;


void EventBase::handleRoute(const std::string& endpoint, HTTPMethod method,
			    const RouteHandler& handler)
{
	routes[endpoint.c_str()][method] = handler;
}


EventBase::RouteHandler& EventBase::RouteOps::operator[](HTTPMethod method)
{
	const int methodIdx = static_cast<int>(method);
	return handlers.at(methodIdx);
}
} // namespace REST
