#include "common.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>


void safe_free(void* mem)
{
	if (mem) {
		free(mem);
	}
}


/* TODO(phymod0): Use POSIX version when available */
char* str_dup(const char* str)
{
	size_t len = strlen(str);
	char* result = NULL;
	if (FT_VALLOC(result, len + 1)) {
		memcpy(result, str, len + 1);
	}
	return result;
}
