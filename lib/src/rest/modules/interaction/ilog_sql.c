#ifdef ILOG_SQL


#include "ilog.h"


struct ILogCtx {
	int stub;
};

struct ILogFilter {
	int stub;
};

struct ILogCursor {
	int stub;
};

struct ILog {
	int stub;
};


ILogError ilog_create_file(const char* filename, const char* fields[],
			   size_t n_fields, ILogCtx** ctx)
{
	(void)filename;
	(void)fields;
	(void)n_fields;
	(void)ctx;
	return ILOG_ESUCCESS;
}


ILogError ilog_load_from_file(const char* filename, ILogCtx** ctx)
{
	(void)filename;
	(void)ctx;
	return ILOG_ESUCCESS;
}


void ilog_destroy(ILogCtx* ctx) { (void)ctx; }


ILogError ilog_get_logs(ILogCtx* ctx, size_t start, ILogFilter* filter,
			ILogCursor** cursor)
{
	(void)ctx;
	(void)start;
	(void)filter;
	(void)cursor;
	return ILOG_ESUCCESS;
}


void ilog_cursor_destroy(ILogCursor* cursor) { (void)cursor; }


ILogError ilog_cursor_read(ILogCursor* cursor, ILog** log)
{
	(void)cursor;
	(void)log;
	return ILOG_ESUCCESS;
}


ILogError ilog_cursor_step(ILogCursor* cursor)
{
	(void)cursor;
	return ILOG_ESUCCESS;
}


bool ilog_cursor_ended(ILogCursor* cursor)
{
	(void)cursor;
	return true;
}


ILog* ilog_create_log() { return NULL; }


char* ilog_get_field_value(ILog* log, const char* field_name)
{
	(void)log;
	(void)field_name;
	return NULL;
}


ILogError ilog_set_field_value(ILog* log, const char* field_name,
			       const char* value)
{
	(void)log;
	(void)field_name;
	(void)value;
	return ILOG_ESUCCESS;
}


void ilog_destroy_log(ILog* log) { (void)log; }


ILogError ilog_write_log(ILogCtx* ctx, ILog* log)
{
	(void)ctx;
	(void)log;
	return ILOG_ESUCCESS;
}


ILogFilter* ilog_filter_equal(const char* field_name, const char* value)
{
	(void)field_name;
	(void)value;
	return NULL;
}


ILogFilter* ilog_filter_str_greater(const char* field_name, const char* value)
{
	(void)field_name;
	(void)value;
	return NULL;
}


ILogFilter* ilog_filter_str_less(const char* field_name, const char* value)
{
	(void)field_name;
	(void)value;
	return NULL;
}


ILogFilter* ilog_filter_int_greater(const char* field_name, long long int value)
{
	(void)field_name;
	(void)value;
	return NULL;
}


ILogFilter* ilog_filter_int_less(const char* field_name, long long int value)
{
	(void)field_name;
	(void)value;
	return NULL;
}


ILogFilter* ilog_filter_conjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B)
{
	(void)filter_A;
	(void)filter_B;
	return NULL;
}


ILogFilter* ilog_filter_disjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B)
{
	(void)filter_A;
	(void)filter_B;
	return NULL;
}


ILogFilter* ilog_filter_inversion(const ILogFilter* filter)
{
	(void)filter;
	return NULL;
}


void ilog_filter_destroy(ILogFilter* filter) { (void)filter; }


const char* ilog_last_error_str(void) { return NULL; }


#endif /* ILOG_SQL */
