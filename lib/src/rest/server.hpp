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
	void addEventBase(EventBase evBase);

      private:
};
} // namespace REST


#endif /* REST_SERVER */
