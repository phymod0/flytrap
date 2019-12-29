#define INCLUDED_BY_LIBEVENT
#include "buffer.hpp"
#undef INCLUDED_BY_LIBEVENT

#include <stdexcept>


#ifndef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#define LIBEVENT_BUFFER_DEFAULT_READ_LIMIT 4096
#endif /* LIBEVENT_BUFFER_DEFAULT_READ_LIMIT */


static evbuffer* createBuffer()
{
	evbuffer* evBuffer = evbuffer_new();
	if (not evBuffer) {
		throw std::runtime_error("Failed to create buffer");
	}
	return evBuffer;
}


static void destroyBuffer(evbuffer* evBuffer) noexcept
{
	evbuffer_free(evBuffer);
}


static void keepBuffer(evbuffer* evBuffer) noexcept { (void)evBuffer; }


namespace LibEvent
{
Buffer::Buffer()
    : readBuffer(LIBEVENT_BUFFER_DEFAULT_READ_LIMIT),
      evb(createBuffer(), destroyBuffer)
{
}


Buffer::Buffer(evbuffer* evBuffer)
    : readBuffer(LIBEVENT_BUFFER_DEFAULT_READ_LIMIT), evb(evBuffer, keepBuffer)
{
}


void Buffer::setReadLimit(size_t limit)
{
	readBuffer = std::vector<char>(limit);
}


Buffer& Buffer::operator<<(const std::string& str)
{
	size_t len = str.length();
	const char* data = str.c_str();
	if (evbuffer_add(evb.get(), data, len) == -1) {
		throw std::runtime_error("Failed to write buffer data");
	}
	return *this;
}


Buffer& Buffer::operator>>(std::string& str)
{
	size_t len = readBuffer.size();
	char* data = readBuffer.data();
	int nRead = evbuffer_remove(evb.get(), data, len);
	if (nRead == -1) {
		throw std::runtime_error("Failed to read buffer data");
	}
	if (nRead > 0) {
		str = std::string(data, nRead);
	}
	return *this;
}


Buffer::operator bool() { return evbuffer_get_length(evb.get()) > 0; }
} // namespace LibEvent


#ifndef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#undef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#endif /* LIBEVENT_BUFFER_DEFAULT_READ_LIMIT */
