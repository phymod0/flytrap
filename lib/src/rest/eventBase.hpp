#ifndef REST_EVENT_BASE
#define REST_EVENT_BASE


#include <functional>
#include <string>
#include <vector>

#include "defs.hpp"
#include "request.hpp"
#include "response.hpp"


#ifndef INCLUDED_BY_REST
// #error Do not include restEventBase.hpp directly
#endif /* INCLUDED_BY_REST */


/*
 * TODO (phymod0):
 *	- Move Route constructor definition to .cpp
 */

// Sun 15 Dec 2019 01:49:08 PM PKT


namespace REST
{
class EventBase
{
	friend class Server;

      public:
	using RouteHandler = std::function<void(Request, Response)>;

	// hicpp: Define all 5 just for the move ctor
	EventBase(const EventBase& evBase) = default;
	EventBase& operator=(const EventBase& evBase) = default;
	EventBase(EventBase&& evBase) = default;
	EventBase& operator=(EventBase&& evBase) = default;
	~EventBase() = default;

      protected:
	void handleRoute(const std::string& route, HTTPMethod method,
			 const RouteHandler& handler);

      private:
	class Route
	{
	      public:
		Route(HTTPMethod method, std::string endpoint,
		      RouteHandler handler)
		    : method(method), endpoint(std::move(endpoint)),
		      handler(std::move(handler)){};

	      private:
		HTTPMethod method;
		std::string endpoint;
		RouteHandler handler;
	};
	std::vector<Route> routes;
};
} // namespace REST


#endif /* REST_EVENT_BASE */
