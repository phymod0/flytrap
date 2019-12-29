#include "libevent.hpp"

#include <iostream>
#include <string>


LibEvent::Http::HandlerFn handler = [](LibEvent::Request&& req) {
	std::string path = req.getURIPath();
	evhttp_cmd_type method = req.getMethod();

	std::cout << "Got method: " << method << std::endl;
	std::cout << "Got request path: " << path << std::endl;

	req.sendReply(200);
};


int main()
{
	LibEvent::EventBase base;

	LibEvent::Http http(base);
	http.setHandler(handler);
	http.bind("0.0.0.0", 3000);

	base.dispatch();

	return 0;
}
