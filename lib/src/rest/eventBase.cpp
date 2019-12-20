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
	routes.emplace_back(Route(method, endpoint, handler));
}


EventBase::Route::Route(HTTPMethod method, std::string endpoint,
			EventBase::RouteHandler handler)
    : method(method), endpoint(std::move(endpoint)), handler(std::move(handler))
{
}
} // namespace REST
