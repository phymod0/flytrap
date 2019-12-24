#ifndef REST_EVENT_BASE
#define REST_EVENT_BASE


#include <array>
#include <functional>
#include <list>
#include <string>

#include "../adt/tree/tree.hpp"
#include "defs.hpp"
#include "request.hpp"
#include "response.hpp"


#ifndef INCLUDED_BY_REST
// #error Do not include restEventBase.hpp directly
#endif /* INCLUDED_BY_REST */


namespace REST
{
class EventBase
{
	friend class Server;

      public:
	using RouteHandler = std::function<void(Request, Response)>;

	// (hicpp) Define all 5 just for the move ctor
	EventBase(const EventBase& evBase);
	EventBase& operator=(const EventBase& evBase);
	EventBase(EventBase&& evBase) noexcept;
	EventBase& operator=(EventBase&& evBase) noexcept;
	~EventBase();

      protected:
	void handleRoute(const std::string& endpoint, HTTPMethod method,
			 const RouteHandler& handler);

      private:
	class RouteOps
	{
		static constexpr size_t nSupportedMethods =
		    static_cast<size_t>(HTTPMethod::N_SUPPORTED_METHODS);

	      public:
		RouteHandler& operator[](HTTPMethod method);

	      private:
		std::array<RouteHandler, nSupportedMethods> handlers;
	};
	std::unordered_map<std::string, RouteOps> routes;
};
} // namespace REST


#endif /* REST_EVENT_BASE */
