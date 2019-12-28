#ifndef WRAPPER_LIBEVENT_REQUEST
#define WRAPPER_LIBEVENT_REQUEST


#include <event2/http.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>


namespace LibEvent
{
class Request
{
      public:
	Request(evhttp_request* req);
	std::string getURIPath();
	evhttp_cmd_type getMethod();
	std::unordered_map<std::string, std::string> getParams();

	std::unordered_map<std::string, std::string> getRequestHeaders();
	void setResponseHeader(const std::string& key, const std::string& val);

	std::string body();
	void sendReply(int code, const std::string& data);
	void sendError(int code, const std::string& data);

      private:
	evhttp_request* req;

	std::unique_ptr<evhttp_uri, decltype(&evhttp_uri_free)> decodedURI;
	std::unique_ptr<char, decltype(&free)> decodedPath;

	evhttp_uri* getDecodedURI();
	char* getDecodedPath();
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_REQUEST */
