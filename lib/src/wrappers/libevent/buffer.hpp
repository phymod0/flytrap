#ifndef WRAPPER_LIBEVENT_BUFFER
#define WRAPPER_LIBEVENT_BUFFER


#include <event2/buffer.h>

#include <memory>
#include <string>
#include <vector>


namespace LibEvent
{
class Buffer
{
      public:
	Buffer();
	Buffer(evbuffer* evb);
	void setReadLimit(size_t limit);
	Buffer& operator<<(const std::string& str);
	Buffer& operator>>(std::string& str);
	operator bool();

      private:
	std::vector<char> readBuffer;
	std::unique_ptr<evbuffer, decltype(&evbuffer_free)> evb;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_BUFFER */
