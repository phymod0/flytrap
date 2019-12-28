#ifndef WRAPPER_LIBEVENT_BUFFER
#define WRAPPER_LIBEVENT_BUFFER


#include <event2/buffer.h>

#include <memory>
#include <string>


namespace LibEvent
{
class Buffer
{
      public:
	Buffer();
	Buffer(evbuffer* evb);
	void setReadLimit(size_t limit);
	void setWriteLimit(size_t limit);
	Buffer& operator<<(const std::string& str);
	Buffer& operator>>(std::string& str);

      private:
	size_t readLimit;
	size_t writeLimit;
	std::unique_ptr<evbuffer, decltype(&evbuffer_free)> evb;
};
} // namespace LibEvent


#endif /* WRAPPER_LIBEVENT_BUFFER */
