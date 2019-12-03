#ifndef REQUIRE
#define REQUIRE

#include <cstring>
#include <stdexcept>
#include <sys/errno.h>

// TODO: Implement errorOn and use where require won't work

namespace CErrors
{

inline void require(bool expected)
{
	if (expected) {
		return;
	}
	const char* error = strerror(errno);
	throw std::runtime_error(error);
}

} // namespace CErrors

#endif /* REQUIRE */
