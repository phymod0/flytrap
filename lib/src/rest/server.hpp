#ifndef REST_SERVER
#define REST_SERVER


#include <string>

#include "defs.hpp"
#include "eventBase.hpp"
#include "request.hpp"
#include "response.hpp"


#ifndef INCLUDED_BY_REST
// #error Do not include restServer.hpp directly
#endif /* INCLUDED_BY_REST */


namespace REST
{
class Server
{
      public:
	Server(std::string ip, int port);
	void start();
	void stop();
	void addEventBase(const EventBase& evBase);

      private:
	std::string ip;
	int port;
	// TODO(phymod0): Add trie mapping endpoints to (method, handler) pairs
};
} // namespace REST


#endif /* REST_SERVER */
