#include "server.hpp"


namespace REST
{
Server::Server(std::string ip, int port) : ip(std::move(ip)), port(port) {}


void Server::start() {}


void Server::stop() {}


void Server::addEventBase(const EventBase& evBase) {}
} // namespace REST
