#ifndef LOGGER
#define LOGGER


#include "../config.h"

#ifdef __cplusplus
extern "C" {
#include <cstdio>
#else /* __cplusplus */
#include <stdio.h>
#endif /* __cplusplus */


void logger_close(void);
int logger_set_filepath(const char* path);
void logger_log_to_stdout(void);
void logger_log_to_stderr(void);
FILE* logger_get_file(void);

void logger_set_levels(unsigned int levels);
void logger_set_min_level(unsigned int level);
int logger_has_level(unsigned int level);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#define LOGGER_LEVEL_DEBUG (1U << 3U)
#define LOGGER_LEVEL_INFO (1U << 2U)
#define LOGGER_LEVEL_WARN (1U << 1U)
#define LOGGER_LEVEL_ERROR (1U << 0U)


#ifdef LOGGING_ENABLE
#define LOGGER_LOG(LOGGER_LEVEL, ...)                                          \
	{                                                                      \
		FILE* fp = logger_get_file();                                  \
		if (fp && logger_has_level(LOGGER_LEVEL)) {                    \
			fprintf(fp, "[%s %s] ", __TIME__, __DATE__);           \
			fprintf(fp, "%s:%d: ", __FILE__, __LINE__);            \
			fprintf(fp, __VA_ARGS__);                              \
			fprintf(fp, "\n");                                     \
			fflush(fp);                                            \
		}                                                              \
	}
#else /* LOGGING_ENABLE */
#define LOGGER_LOG(LOGGER_LEVEL, ...)
#endif /* LOGGING_ENABLE */

#define LOGGER_DEBUG(...) LOGGER_LOG(LOGGER_LEVEL_DEBUG, __VA_ARGS__)
#define LOGGER_INFO(...) LOGGER_LOG(LOGGER_LEVEL_INFO, __VA_ARGS__)
#define LOGGER_WARN(...) LOGGER_LOG(LOGGER_LEVEL_WARN, __VA_ARGS__)
#define LOGGER_ERROR(...) LOGGER_LOG(LOGGER_LEVEL_ERROR, __VA_ARGS__)


#endif /* LOGGER */
