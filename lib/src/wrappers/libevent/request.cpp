#include "request.hpp"

#include <event2/http.h>
#include <event2/keyvalq_struct.h>


static evhttp_uri* createURI(evhttp_request* req)
{
	const char* uri = evhttp_request_get_uri(req);
	if (not uri) {
		return nullptr;
	}

	evhttp_uri* decoded = evhttp_uri_parse(uri);
	if (not decoded) {
		throw std::runtime_error("Failed to decode URI");
	}

	return decoded;
}


static void freeURI(evhttp_uri* uri) noexcept
{
	if (uri)
		evhttp_uri_free(uri);
}


static char* createPath(evhttp_uri* uri)
{
	if (not uri) {
		return nullptr;
	}

	const char* path = evhttp_uri_get_path(uri);
	if (not path) {
		return nullptr;
	}

	char* decoded = evhttp_uridecode(path, 0, nullptr);
	if (not decoded) {
		throw std::runtime_error("Failed to decode URI path");
	}

	return decoded;
}


static void freePath(void* path) noexcept
{
	if (path)
		free(path);
}


static std::unordered_map<std::string, std::string>
evkeyvalqToMap(evkeyvalq* kvq)
{
	std::unordered_map<std::string, std::string> result;
	for (evkeyval* kv = kvq->tqh_first; kv; kv = kv->next.tqe_next) {
		const std::string key(kv->key);
		const std::string value(kv->value);
		result[key] = value;
	}
	return result;
}


namespace LibEvent
{
Request::Request(evhttp_request* req)
    : req(req), decodedURI(nullptr, freeURI), decodedPath(nullptr, freePath)
{
}


std::string Request::getURIPath() { return std::string(getDecodedPath()); }


evhttp_cmd_type Request::getMethod() { return evhttp_request_get_command(req); }


std::unordered_map<std::string, std::string> Request::getParams()
{
	const char* query = evhttp_uri_get_query(getDecodedURI());
	evkeyvalq params;
	evhttp_parse_query_str(query, &params);
	std::unordered_map<std::string, std::string> result =
	    evkeyvalqToMap(&params);
	evhttp_clear_headers(&params);
	return result;
}


std::unordered_map<std::string, std::string> Request::getRequestHeaders()
{
	evkeyvalq* headers = evhttp_request_get_input_headers(req);
	return evkeyvalqToMap(headers);
}


void Request::setResponseHeader(const std::string& key, const std::string& val)
{
	evhttp_add_header(evhttp_request_get_output_headers(req), key.c_str(),
			  val.c_str());
}


std::string Request::body() {}


void Request::sendReply(int code, const std::string& data) {}


void Request::sendError(int code, const std::string& data) {}


evhttp_uri* Request::getDecodedURI()
{
	if (not decodedURI) {
		decodedURI.reset(createURI(req));
	}
	return decodedURI.get();
}


char* Request::getDecodedPath()
{
	if (not decodedPath) {
		decodedPath.reset(createPath(getDecodedURI()));
	}
	return decodedPath.get();
}
} // namespace LibEvent
