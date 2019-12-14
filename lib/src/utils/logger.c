#include "logger.h"


static unsigned int log_levels = LOGGER_LEVEL_DEBUG | LOGGER_LEVEL_INFO |
				 LOGGER_LEVEL_WARN | LOGGER_LEVEL_ERROR;
static FILE* logger_fp = NULL;


void logger_close(void)
{
	if (logger_fp && logger_fp != stdout && logger_fp != stderr) {
		fclose(logger_fp);
	}
}


int logger_set_filepath(const char* path)
{
	logger_close();
	FILE* fp = fopen(path, "a");
	if (!fp)
		return -1;
	logger_fp = fp;
	return 0;
}


void logger_log_to_stdout(void)
{
	logger_close();
	logger_fp = stdout;
}


void logger_log_to_stderr(void)
{
	logger_close();
	logger_fp = stderr;
}


FILE* logger_get_file(void) { return logger_fp; }


void logger_set_levels(unsigned int levels) { log_levels = levels; }


void logger_set_min_level(unsigned int level) { log_levels = (level << 1) - 1; }


int logger_has_level(unsigned int level) { return log_levels & level; }
