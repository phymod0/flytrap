#include "buffer.hpp"

#include <stdexcept>


#ifndef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#define LIBEVENT_BUFFER_DEFAULT_READ_LIMIT 4096
#endif /* LIBEVENT_BUFFER_DEFAULT_READ_LIMIT */

#ifndef LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT
#define LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT 4096
#endif /* LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT */


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
    : readLimit(LIBEVENT_BUFFER_DEFAULT_READ_LIMIT),
      writeLimit(LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT),
      evb(createBuffer(), destroyBuffer)
{
}


Buffer::Buffer(evbuffer* evBuffer)
    : readLimit(LIBEVENT_BUFFER_DEFAULT_READ_LIMIT),
      writeLimit(LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT), evb(evBuffer, keepBuffer)
{
}


void Buffer::setReadLimit(size_t limit) { readLimit = limit; }


void Buffer::setWriteLimit(size_t limit) { writeLimit = limit; }


Buffer& Buffer::operator<<(const std::string& str) {}


Buffer& Buffer::operator>>(std::string& str) {}
} // namespace LibEvent


#ifndef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#undef LIBEVENT_BUFFER_DEFAULT_READ_LIMIT
#endif /* LIBEVENT_BUFFER_DEFAULT_READ_LIMIT */

#ifndef LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT
#undef LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT
#endif /* LIBEVENT_BUFFER_DEFAULT_WRITE_LIMIT */
