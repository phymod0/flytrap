#include "server.hpp"
#include "../utils/splitIterator.hpp"


namespace REST
{
Server::Server(std::string ip, int port) : ip(std::move(ip)), port(port) {}


void Server::start() {}


void Server::stop() {}


void Server::addEventBase(const EventBase& evBase)
{
	for (const auto& route : evBase.routes) {
		const std::string& endpoint = route.first;
		const EventBase::RouteOps& routeOps = route.second;
		SplitIterator<'/'> pathSegments(endpoint);
		Tree<std::string, EventBase::RouteOps>& subhandlers = handlers;
		for (const std::string& pathSegment : pathSegments) {
			subhandlers = subhandlers[pathSegment];
		}
		subhandlers.value() = routeOps;
	}
}
} // namespace REST
