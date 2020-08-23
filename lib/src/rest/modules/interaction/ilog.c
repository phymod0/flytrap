#include "ilog.h"
#include "../../../config.h"
#include "../../../utils/common.h"
#include "../../../utils/logger.h"
#include "../../../utils/macros.h"

#include <stdlib.h>
#include <string.h>


/*
 * XXX: Filtering is not functional (disabled by default)
 *
 * TODO(phymod0):
 *	- Finger tables for linearithmic cursor position setting
 *	- Redesign and implement filtering!
 */


typedef enum {
	ILOG_FLT_BASIC,
	ILOG_FLT_INVERSION,
	ILOG_FLT_CONJUNCTION,
	ILOG_FLT_DISJUNCTION,
} ILogFilterType;

typedef enum {
	ILOG_RLT_NUMLESS,
	ILOG_RLT_NUMGREATER,
	ILOG_RLT_STRLESS,
	ILOG_RLT_STRGREATER,
	ILOG_RLT_EQUAL,
} ILogRelationType;

struct ILogList {
	ILog *head, *last_saved;
};

struct ILogFilter {
	ILogFilterType filter_type;
	struct ILogFilter* children[2];
	long long int int_value;
	char* str_value;
	ILogRelationType relation_type;
};

struct ILogCursor {
	ILogList* ilog_list;
	ILog* current_ilog;
	ILogFilter* filter;
};

struct ILog {
	ILog* next;
	ILogID id;
	time_t timestamp;
	unsigned char mac_addr[MAC_ADDRLEN];
	int type, subtype;
	char* json_data;
};


static ILog* ilog_create(const unsigned char* mac_addr, int type, int subtype,
			 const char* json_data);
static void ilog_list_prepend(ILogList* ilog_list, ILog* ilog);
static void ilog_destroy(ILog* ilog);
static bool filter_satisfied(ILogFilter* filter, ILog* ilog);
static ILogCursor* cursor_create(ILogList* ilog_list, ILogFilter* filter);
static void cursor_destroy(ILogCursor* cursor);
static ILog* next_or_null(ILog* ilog);
static void cursor_increment_position(ILogCursor* cursor);
static void cursor_set_position(ILogCursor* cursor, ILogID to);
static void cursor_advance(ILogCursor* cursor);


ILogError ilog_list_create(ILogList** ilog_list)
{
	ILogList* result = NULL;

	if (FT_ALLOC(result) == NULL) {
		goto oom;
	}
	result->head = result->last_saved = NULL;

	*ilog_list = result;
	return ILOG_ESUCCESS;

oom:
	LOGGER_ERROR("Out of memory");
	return ILOG_ENOMEM;
}


void ilog_list_destroy(ILogList* ilog_list)
{
	if (ilog_list == NULL) {
		return;
	}

	ILog* next = NULL;
	for (ILog* ilog = ilog_list->head; ilog != NULL; ilog = next) {
		next = ilog->next;
		ilog_destroy(ilog);
	}
	free(ilog_list);
}


ILogError ilog_list_insert(ILogList* ilog_list, const unsigned char* mac_addr,
			   int type, int subtype, const char* json_data)
{
	ILog* ilog = ilog_create(mac_addr, type, subtype, json_data);
	if (ilog == NULL) {
		LOGGER_ERROR("Out of memory");
		return ILOG_ENOMEM;
	}

	ilog_list_prepend(ilog_list, ilog);
	return ILOG_ESUCCESS;
}


ILogError ilog_get_logs(ILogList* ilog_list, ILogID start, ILogFilter* filter,
			ILogCursor** cursor_dst)
{
	ILogCursor* cursor = NULL;
	if ((cursor = cursor_create(ilog_list, filter)) == NULL) {
		LOGGER_ERROR("Out of memory");
		cursor_destroy(cursor);
		return ILOG_ENOMEM;
	}
	cursor_set_position(cursor, start);
	cursor_advance(cursor);

	LOGGER_DEBUG("Returning interaction logs in cursor %p", (void*)cursor);
	*cursor_dst = cursor;
	return ILOG_ESUCCESS;
}


void ilog_cursor_destroy(ILogCursor* cursor) { cursor_destroy(cursor); }


const ILog* ilog_cursor_read(ILogCursor* cursor)
{
	return cursor->current_ilog;
}


void ilog_cursor_step(ILogCursor* cursor)
{
	cursor_increment_position(cursor);
	cursor_advance(cursor);
}


bool ilog_cursor_ended(ILogCursor* cursor)
{
	return cursor->current_ilog == NULL;
}


time_t ilog_get_timestamp(ILog* log) { return log->timestamp; }


ILogID ilog_get_id(ILog* log) { return log->id; }


const unsigned char* ilog_get_mac_addr(ILog* log) { return log->mac_addr; }


int ilog_get_type(ILog* log) { return log->type; }


int ilog_get_subtype(ILog* log) { return log->subtype; }


const char* ilog_get_json_data(ILog* log) { return log->json_data; }


ILogFilter* ilog_filter_equal(const char* field_name, const char* value)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(field_name);
	FT_UNUSED(value);
	return NULL;
}


ILogFilter* ilog_filter_str_greater(const char* field_name, const char* value)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(field_name);
	FT_UNUSED(value);
	return NULL;
}


ILogFilter* ilog_filter_str_less(const char* field_name, const char* value)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(field_name);
	FT_UNUSED(value);
	return NULL;
}


ILogFilter* ilog_filter_int_greater(const char* field_name, long long int value)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(field_name);
	FT_UNUSED(value);
	return NULL;
}


ILogFilter* ilog_filter_int_less(const char* field_name, long long int value)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(field_name);
	FT_UNUSED(value);
	return NULL;
}


ILogFilter* ilog_filter_conjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(filter_A);
	FT_UNUSED(filter_B);
	return NULL;
}


ILogFilter* ilog_filter_disjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(filter_A);
	FT_UNUSED(filter_B);
	return NULL;
}


ILogFilter* ilog_filter_inversion(const ILogFilter* filter)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(filter);
	return NULL;
}


void ilog_filter_destroy(ILogFilter* filter)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(filter);
}


const char* ilog_error_description(ILogError err)
{
	const char* descriptions[] = {
	    [ILOG_ESUCCESS] = "Successful",
	    [ILOG_ENOMEM] = "Out of memory",
	    [ILOG_ECUREND] = "Operated on an ended cursor",
	    [ILOG_ELOAD] = "Failed to load/create log file",
	    [ILOG_EREAD] = "Failed to read from log file",
	    [ILOG_EUPDATE] = "Failed to update log file",
	};

	if (err < 0 || err >= ILOG_N_ERRORS) {
		LOGGER_WARN("Received bad error code %d. Valid error codes "
			    "range from %d to %d inclusive.",
			    err, 0, ILOG_N_ERRORS - 1);
		return "Bad error code";
	}
	return descriptions[err];
}

static ILog* ilog_create(const unsigned char* mac_addr, int type, int subtype,
			 const char* json_data)
{
	struct ILog* result = NULL;
	char* json_data_copy = NULL;

	if (FT_ALLOC(result) == NULL ||
	    (json_data != NULL &&
	     (json_data_copy = str_dup(json_data)) == NULL)) {
		LOGGER_ERROR("Out of memory");
		goto oom;
	}
	result->next = NULL;
	result->id = 0;
	result->timestamp = time(NULL);
	memcpy(result->mac_addr, mac_addr, sizeof result->mac_addr);
	result->type = type;
	result->subtype = subtype;
	result->json_data = json_data_copy;

	return result;

oom:
	safe_free(result);
	safe_free(json_data_copy);
	return NULL;
}


static void ilog_list_prepend(ILogList* ilog_list, ILog* ilog)
{
	ilog->next = ilog_list->head;
	ilog->id = ilog_list->head ? ilog_list->head->id + 1 : 0;
	ilog_list->head = ilog;
}

static void ilog_destroy(ILog* ilog)
{
	if (ilog == NULL) {
		return;
	}
	safe_free(ilog->json_data);
	free(ilog);
}


static bool filter_satisfied(ILogFilter* filter, ILog* ilog)
{
	LOGGER_WARN("Not implemented");
	FT_UNUSED(filter);
	FT_UNUSED(ilog);
	return true;
}


static ILogCursor* cursor_create(ILogList* ilog_list, ILogFilter* filter)
{
	ILogCursor* cursor = NULL;

	if (FT_ALLOC(cursor) == NULL) {
		LOGGER_ERROR("Out of memory");
		return NULL;
	}
	cursor->ilog_list = ilog_list;
	cursor->current_ilog = ilog_list->head;
	cursor->filter = filter;

	return cursor;
}


static void cursor_destroy(ILogCursor* cursor) { safe_free(cursor); }


static ILog* next_or_null(ILog* ilog) { return ilog ? ilog->next : NULL; }


static void cursor_increment_position(ILogCursor* cursor)
{
	cursor->current_ilog = next_or_null(cursor->current_ilog);
}


static void cursor_set_position(ILogCursor* cursor, ILogID to)
{
	ILog* head = cursor->ilog_list->head;
	ILog* current = cursor->current_ilog;
	ILog* ilog = (current && current->id >= to) ? current : head;
	while (ilog && ilog->id != to) {
		ilog = next_or_null(ilog);
	}
	cursor->current_ilog = ilog;
}


static void cursor_advance(ILogCursor* cursor)
{
	ILog* ilog = cursor->current_ilog;
	ILogFilter* filter = cursor->filter;
	while (ilog && !filter_satisfied(filter, ilog)) {
		ilog = next_or_null(ilog);
	}
	cursor->current_ilog = ilog;
}
