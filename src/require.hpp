#ifndef REQUIRE
#define REQUIRE


#include <string.h>
#include <exception>
#include <sys/errno.h>


namespace CErrors {
static inline void require(bool expected)
{
        if (expected)
                return;
        const char* error = strerror(errno);
        throw std::runtime_error(error);
}
}


#endif /* REQUIRE */
