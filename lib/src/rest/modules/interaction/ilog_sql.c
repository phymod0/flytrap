#ifdef ILOG_SQL


#include "../../../adt/trie/trie.h"
#include "../../../adt/vector/vector.h"
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
	Vector fields;
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


/* TODO(phymod0): Remove unneccessary functions/macros in the end */
static void set_last_error(const char* error);
static char* str_dup(const char* str);
static void* str_dup_void(const void* str);
static Vector str_vector_from(const char** strings, size_t sz);
static ILogError db_fetch_fields(const sqlite3* db, Vector* fields, int* err);
static ILogError db_set_fields(const sqlite3* db, Vector fields, int* err);
static ILogCtx* ctx_create(void);
static void ctx_destroy(ILogCtx* ctx);
static ILogError try_db_open(ILogCtx* ctx, const char* filename, int* err_dst);
static ILogError try_db_create_and_open(ILogCtx* ctx, const char* filename,
					int* err_dst);


ILogError ilog_create_file(const char* filename, const char* fields[],
			   size_t n_fields, ILogCtx** ctx)
{
	int err = 0;
	int ilog_error = 0;
	ILogCtx* result = NULL;

	LOGGER_DEBUG("Attempting to open SQL database at %s with %lu fields",
		     filename, n_fields);

	if ((result = ctx_create()) == NULL) {
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	ilog_error = try_db_open(result, filename, &err);
	if (ilog_error == ILOG_ESUCCESS && err == SQLITE_OK &&
	    result->db != NULL) {
		goto success;
	}

	if ((result->fields = str_vector_from(fields, n_fields)) == NULL) {
		LOGGER_ERROR("Insufficient memory for copying field names");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	LOGGER_DEBUG("Could not open database at %s, trying to create",
		     filename);
	ilog_error = try_db_create_and_open(result, filename, &err);
	if (ilog_error == ILOG_ESUCCESS && err == SQLITE_OK &&
	    result->db != NULL) {
		goto success;
	}

error:
	ctx_destroy(result);
	*ctx = NULL;
	return ilog_error;

success:
	*ctx = result;
	return ILOG_ESUCCESS;
}


ILogError ilog_load_from_file(const char* filename, ILogCtx** ctx)
{
	int err = 0;
	int ilog_error = 0;
	ILogCtx* result = NULL;

	LOGGER_DEBUG("Attempting to load SQL database at %s", filename);

	if ((result = ctx_create()) == NULL) {
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	ilog_error = try_db_open(result, filename, &err);
	if (ilog_error != ILOG_ESUCCESS || err != SQLITE_OK ||
	    result->db == NULL) {
		LOGGER_ERROR("Could not open database at %s", filename);
		set_last_error("Failed to open database");
		goto error;
	}

	*ctx = result;
	return ILOG_ESUCCESS;

error:
	ctx_destroy(result);
	*ctx = NULL;
	return ilog_error;
}


void ilog_destroy(ILogCtx* ctx) { ctx_destroy(ctx); }


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


static void set_last_error(const char* error) { __last_error = error; }


static char* str_dup(const char* str)
{
	size_t len = strlen(str);
	char* result = NULL;
	if (VALLOC(result, len + 1)) {
		memcpy(result, str, len + 1);
	}
	return result;
}


static void* str_dup_void(const void* str) { return str_dup(str); }


static Vector str_vector_from(const char** strings, size_t sz)
{
	Vector vector = vector_create(&(VectorElementOps){
	    .dtor = free,
	    .copy = str_dup_void,
	});
	if (vector == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < sz; ++i) {
		const void* to_insert = strings[i];
		if (vector_copy_and_insert(&vector, to_insert) < 0) {
			vector_destroy(vector);
			return NULL;
		}
	}
	return vector;
}


static ILogError db_fetch_fields(const sqlite3* db, Vector* fields, int* err) {}


static ILogError db_set_fields(const sqlite3* db, Vector fields, int* err) {}


static ILogCtx* ctx_create(void)
{
	ILogCtx* ctx = NULL;
	if (ALLOC(ctx) == NULL) {
		LOGGER_ERROR("Insufficient memory for context creation");
		set_last_error("Out of memory");
		return NULL;
	}
	ctx->db = NULL;
	ctx->fields = NULL;
	return ctx;
}


static void ctx_destroy(ILogCtx* ctx)
{
	if (ctx == NULL) {
		return;
	}
	sqlite3_close(ctx->db);
	vector_destroy(ctx->fields);
	free(ctx);
}


static ILogError try_db_open(ILogCtx* ctx, const char* filename, int* err_dst)
{
	int err = 0;
	ILogError ilog_error = 0;
	sqlite3* db = NULL;
	Vector fields = NULL;

	err = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if (err != SQLITE_OK || db == NULL) {
		LOGGER_DEBUG("Failed to open database at %s", filename);
		set_last_error("Failed to open database");
		ilog_error = ILOG_EFILE;
		goto error;
	}

	LOGGER_DEBUG("Opened database file at %s", filename);
	ilog_error = db_fetch_fields(db, &fields, &err);
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

	ctx->db = db;
	ctx->fields = fields;
	*err_dst = err;
	LOGGER_INFO("Opened database at %s", filename);
	set_last_error("Successful");
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	vector_destroy(fields);
	ctx->db = NULL;
	ctx->fields = NULL;
	*err_dst = err;
	return ilog_error;
}


static ILogError try_db_create_and_open(ILogCtx* ctx, const char* filename,
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
	ilog_error = db_set_fields(db, ctx->fields, &err);
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

	ctx->db = db;
	*err_dst = err;
	LOGGER_INFO("Created database at %s", filename);
	set_last_error("Successful");
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	ctx->db = NULL;
	*err_dst = err;
	return ilog_error;
}


#undef VALLOC
#undef ALLOC


#endif /* ILOG_SQL */
