#ifdef ILOG_SQL


#include "../../../adt/trie/trie.h"
#include "../../../utils/logger.h"
#include "ilog.h"

#include <sqlite3.h>


#define ALLOC(x) ((x) = malloc(sizeof *(x)))
#define VALLOC(x, n) ((x) = malloc((sizeof *(x)) * (n)))


static const char* __last_error = "No error";

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

struct ILogCtx {
	sqlite3* db;
	char** fields;
	size_t n_fields;
};

struct ILogFilter {
	ILogFilterType filter_type;
	struct ILogFilter* children[2];
	long long int int_value;
	char* str_value;
	ILogRelationType relation_type;
};

struct ILogCursor {
	sqlite3_stmt* cursor;
	bool is_ended;
	struct ILogCtx* ctx;
};

struct ILog {
	Trie* field_value_map;
};


/* TODO(phymod0): Split static functions into declarations + definitions */
/* TODO(phymod0): Remove unneccessary functions in the end */
/* TODO(phymod0): Move definitions to bottom of file preserving order */
void set_last_error(const char* error);
static char* str_dup(const char* str);
void safe_free(void* mem);
void safe_free_str_vector(char** vec, size_t sz);
char** copy_str_vector(const char** src, size_t sz);
ILogError db_fetch_fields(const sqlite3* db, char*** fields, size_t* n_fields,
			  int* err);
ILogError db_set_fields(const sqlite3* db, const char** fields, size_t n_fields,
			int* err);
ILogError try_db_open(const char* filename, sqlite3** db_dst,
		      char*** fields_dst, size_t* n_fields_dst, int* err_dst);
ILogError try_db_create_and_open(const char* filename, sqlite3** db_dst,
				 const char** fields, size_t n_fields,
				 int* err_dst);


ILogError ilog_create_file(const char* filename, const char* fields[],
			   size_t n_fields, ILogCtx** ctx)
{
	int err = 0;
	int ilog_error = 0;
	ILogCtx* result = NULL;

	LOGGER_DEBUG("Attempting to create SQL database at %s with %lu fields",
		     filename, n_fields);

	if (ALLOC(result) == NULL) {
		LOGGER_ERROR("Insufficient memory for context creation");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}
	result->db = NULL;
	result->fields = NULL;
	result->n_fields = 0;

	ilog_error = try_db_open(filename, &result->db, &result->fields,
				 &result->n_fields, &err);
	if (ilog_error == ILOG_ESUCCESS && err == SQLITE_OK &&
	    result->db != NULL) {
		goto success;
	}

	if ((result->fields = copy_str_vector(fields, n_fields)) == NULL) {
		LOGGER_ERROR("Insufficient memory for copying field names");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}
	result->n_fields = n_fields;

	LOGGER_DEBUG("No database found at %s, trying to create", filename);
	ilog_error = try_db_create_and_open(filename, &result->db, fields,
					    n_fields, &err);
	if (ilog_error == ILOG_ESUCCESS && err == SQLITE_OK &&
	    result->db != NULL) {
		goto success;
	}

error:
	*ctx = NULL;
	if (result != NULL) {
		sqlite3_close(result->db);
		safe_free_str_vector(result->fields, result->n_fields);
		free(result);
	}
	return ilog_error;

success:
	*ctx = result;
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


const char* ilog_last_error_str(void) { return __last_error; }


void set_last_error(const char* error) { __last_error = error; }


static char* str_dup(const char* str)
{
	size_t len = strlen(str);
	char* result = NULL;
	if (VALLOC(result, len + 1)) {
		memcpy(result, str, len + 1);
	}
	return result;
}


void safe_free(void* mem)
{
	if (mem) {
		free(mem);
	}
}


void safe_free_str_vector(char** vec, size_t sz)
{
	if (vec == NULL) {
		return;
	}
	for (size_t i = 0; i < sz; ++i) {
		if (vec[i]) {
			free(vec[i]);
		} else {
			break;
		}
	}
	free(vec);
}


char** copy_str_vector(const char** src, size_t sz)
{
	char** copy = NULL;
	if (VALLOC(copy, sz) == NULL) {
		goto oom;
	}
	for (size_t i = 0; i < sz; ++i) {
		if ((copy[i] = str_dup(src[i])) == NULL) {
			goto oom;
		}
	}
	return copy;

oom:
	safe_free_str_vector(copy, sz);
	return NULL;
}


ILogError db_fetch_fields(const sqlite3* db, char*** fields, size_t* n_fields,
			  int* err)
{
}


ILogError db_set_fields(const sqlite3* db, const char** fields, size_t n_fields,
			int* err)
{
}


ILogError try_db_open(const char* filename, sqlite3** db_dst,
		      char*** fields_dst, size_t* n_fields_dst, int* err_dst)
{
	int err = 0;
	ILogError ilog_error = 0;
	sqlite3* db = NULL;
	char** fields = NULL;
	size_t n_fields = 0;

	err = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if (err != SQLITE_OK || db == NULL) {
		LOGGER_DEBUG("Failed to open database at %s", filename);
		set_last_error("Failed to open database");
		ilog_error = ILOG_EFILE;
		goto error;
	}

	LOGGER_DEBUG("Opened database file at %s", filename);
	ilog_error = db_fetch_fields(db, &fields, &n_fields, &err);
	if (ilog_error == ILOG_ENOMEM) {
		LOGGER_ERROR("Insufficient memory for copying database fields");
		set_last_error("Out of memory");
		goto error;
	}
	if (err != SQLITE_OK) {
		ilog_error = ILOG_EINV;
		LOGGER_ERROR("Cannot read database: %s", sqlite3_errmsg(db));
		set_last_error("Failed to read from database");
		goto error;
	}

	*db_dst = db;
	*fields_dst = fields;
	*n_fields_dst = n_fields;
	*err_dst = err;
	LOGGER_INFO("Opened database at %s", filename);
	set_last_error("Successful");
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	safe_free_str_vector(fields, n_fields);
	*db_dst = NULL;
	*fields_dst = NULL;
	*n_fields_dst = 0;
	*err_dst = err;
	return ilog_error;
}


ILogError try_db_create_and_open(const char* filename, sqlite3** db_dst,
				 const char** fields, size_t n_fields,
				 int* err_dst)
{
	int err = 0;
	ILogError ilog_error = 0;
	sqlite3* db = NULL;

	err = sqlite3_open_v2(filename, &db,
			      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (err != SQLITE_OK || db == NULL) {
		LOGGER_DEBUG("Failed to create database at %s", filename);
		set_last_error("Failed to create database");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	LOGGER_DEBUG("Created database file at %s", filename);
	ilog_error = db_set_fields(db, fields, n_fields, &err);
	if (ilog_error == ILOG_ENOMEM) {
		LOGGER_ERROR("Insufficient memory for setting database fields");
		set_last_error("Out of memory");
		goto error;
	}
	if (err != SQLITE_OK) {
		ilog_error = ILOG_EINV;
		LOGGER_ERROR("Can't write to database: %s", sqlite3_errmsg(db));
		set_last_error("Failed to write to database");
		goto error;
	}

	*db_dst = db;
	*err_dst = err;
	LOGGER_INFO("Created database at %s", filename);
	set_last_error("Successful");
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	*db_dst = NULL;
	*err_dst = err;
	return ilog_error;
}


#undef VALLOC
#undef ALLOC


#endif /* ILOG_SQL */
