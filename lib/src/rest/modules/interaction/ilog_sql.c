#ifdef ILOG_SQL


#include "../../../adt/trie/trie.h"
#include "../../../adt/vector/vector.h"
#include "../../../config.h"
#include "../../../utils/logger.h"
#include "ilog.h"

#include <sqlite3.h>


#define ALLOC(x) ((x) = malloc(sizeof *(x)))
#define VALLOC(x, n) ((x) = malloc((sizeof *(x)) * (n)))
#define STRLEN(str) (sizeof(str) - 1)


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


/*
 * TODO(phymod0):
 *    - Bijection between ILogError and error description, remove set_last_error
 */


static void set_last_error(const char* error);
static char* str_dup(const char* str);
static void* str_dup_void(const void* str);
static Vector str_vector_from(const char** strings, size_t sz);
static ILogError db_fetch_fields(sqlite3* db, Vector* fields);
static char* get_create_stmt(Vector fields);
static ILogError db_set_fields(sqlite3* db, Vector fields);
static ILogCtx* ctx_create(void);
static void ctx_destroy(ILogCtx* ctx);
static ILogError try_db_open(ILogCtx* ctx, const char* filename);
static ILogError try_db_create_and_open(ILogCtx* ctx, const char* filename);


ILogError ilog_create_or_load_file(const char* filename, const char* fields[],
				   size_t n_fields, ILogCtx** ctx)
{
	int ilog_error = 0;
	ILogCtx* result = NULL;

	LOGGER_DEBUG("Attempting to open SQL database at %s with %lu fields",
		     filename, n_fields);

	if ((result = ctx_create()) == NULL) {
		LOGGER_ERROR("Context creation failed");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	LOGGER_DEBUG("Attempting to open DB at %s", filename);
	ilog_error = try_db_open(result, filename);
	if (ilog_error == ILOG_ESUCCESS && result->db != NULL) {
		LOGGER_INFO("Opened database at %s", filename);
		goto success;
	}
	LOGGER_INFO("Could not open database at %s, trying to create",
		    filename);

	if ((result->fields = str_vector_from(fields, n_fields)) == NULL) {
		LOGGER_ERROR("Insufficient memory for copying field names");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	ilog_error = try_db_create_and_open(result, filename);
	if (ilog_error == ILOG_ESUCCESS && result->db != NULL) {
		LOGGER_INFO("Created and opened database at %s", filename);
		goto success;
	}

error:
	LOGGER_ERROR("Failed to create/load database at %s", filename);
	ctx_destroy(result);
	*ctx = NULL;
	return ilog_error;

success:
	set_last_error("Successful");
	*ctx = result;
	return ILOG_ESUCCESS;
}


ILogError ilog_load_file(const char* filename, ILogCtx** ctx)
{
	int ilog_error = 0;
	ILogCtx* result = NULL;

	LOGGER_DEBUG("Attempting to load SQL database at %s", filename);

	if ((result = ctx_create()) == NULL) {
		LOGGER_ERROR("Context creation failed");
		ilog_error = ILOG_ENOMEM;
		goto error;
	}

	ilog_error = try_db_open(result, filename);
	if (ilog_error != ILOG_ESUCCESS || result->db == NULL) {
		LOGGER_ERROR("Could not open database at %s", filename);
		goto error;
	}

	LOGGER_INFO("Opened database at %s", filename);
	set_last_error("Successful");
	*ctx = result;
	return ILOG_ESUCCESS;

error:
	LOGGER_ERROR("Failed to load database at %s", filename);
	ctx_destroy(result);
	*ctx = NULL;
	return ilog_error;
}


void ilog_destroy(ILogCtx* ctx)
{
	LOGGER_INFO("Freeing up ILog context at %p", (void*)ctx);
	ctx_destroy(ctx);
}


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


static ILogError db_fetch_fields(sqlite3* db, Vector* fields)
{
	int err = SQLITE_OK;
	sqlite3_stmt* stmt = NULL;
	ILogError ilog_error = ILOG_ESUCCESS;
	Vector result = NULL;
	const char* sql_statement =
	    "SELECT name FROM PRAGMA_TABLE_INFO('" ILOG_SQL_TABLENAME "');";

	result = vector_create(&(VectorElementOps){
	    .dtor = free,
	    .copy = str_dup_void,
	});
	if (result == NULL) {
		LOGGER_ERROR("Insufficient memory for vector creation");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto err;
	}

	LOGGER_DEBUG("Loading column names with query: %s", sql_statement);
	err = sqlite3_prepare_v2(db, sql_statement, -1, &stmt, NULL);
	if (err != SQLITE_OK || stmt == NULL) {
		LOGGER_ERROR("Failed to prepare SQL statement: %s",
			     sqlite3_errmsg(db));
		set_last_error("Database query failed");
		ilog_error = ILOG_EINV;
		goto err;
	}

	LOGGER_DEBUG("Found column names, reading...");
	while (1) {
		int step_rc = sqlite3_step(stmt);
		if (step_rc == SQLITE_DONE) {
			LOGGER_DEBUG("Read all column names");
			break;
		}
		if (step_rc != SQLITE_ROW) {
			LOGGER_ERROR("Failed to read row from DB: %s",
				     sqlite3_errmsg(db));
			set_last_error("Database query failed");
			sqlite3_finalize(stmt);
			ilog_error = ILOG_EINV;
			goto err;
		}
		const unsigned char* column_name = sqlite3_column_text(stmt, 0);
		if (vector_copy_and_insert(&result, column_name) < 0) {
			LOGGER_ERROR("Failed to initialize vector");
			set_last_error("Out of memory");
			sqlite3_finalize(stmt);
			ilog_error = ILOG_ENOMEM;
			goto err;
		}
		LOGGER_DEBUG("Read column name: %s", column_name);
	}

	LOGGER_DEBUG("Fetched %lu column names", vector_size(result));
	*fields = result;
	err = sqlite3_finalize(stmt);
	return err == SQLITE_OK ? ILOG_ESUCCESS : ILOG_EINV;

err:
	LOGGER_DEBUG("Failed to read column names from table %s",
		     ILOG_SQL_TABLENAME);
	vector_destroy(result);
	return ilog_error;
}


static char* get_create_stmt(Vector fields)
{
#define START                                                                  \
	"DROP TABLE IF EXISTS " ILOG_SQL_TABLENAME ";"                         \
	"CREATE TABLE " ILOG_SQL_TABLENAME "("
#define DELIM " VARCHAR(255), "
#define END ");"

	size_t n = vector_size(fields);
	size_t result_sz = 0;
	char* result = NULL;

	result_sz += STRLEN(START);
	for (size_t i = 0; i < n; ++i) {
		result_sz += strlen(fields[i]) + STRLEN(DELIM);
	}
	result_sz += STRLEN(DELIM);
	++result_sz; // NULL-terminator

	if (VALLOC(result, result_sz) == NULL) {
		return NULL;
	}
	result[0] = '\0';
	strncat(result, START, STRLEN(START));
	for (size_t i = 0; i < n; ++i) {
		strncat(result, fields[i], MAX_FIELDNAME_STRLEN);
		strncat(result, DELIM, STRLEN(DELIM));
	}
	strncat(result, END, STRLEN(END));

	return result;

#undef END
#undef DELIM
#undef START
}


static ILogError db_set_fields(sqlite3* db, Vector fields)
{
	int err = SQLITE_OK;
	ILogError ilog_error = ILOG_ESUCCESS;
	char* sql_statement = NULL;
	char* errmsg = NULL;

	if ((sql_statement = get_create_stmt(fields)) == NULL) {
		LOGGER_ERROR("Insufficient memory for creating SQL statement");
		set_last_error("Out of memory");
		ilog_error = ILOG_ENOMEM;
		goto err;
	}

	LOGGER_DEBUG("Creating table with statement: %s", sql_statement);
	err = sqlite3_exec(db, sql_statement, NULL, NULL, &errmsg);
	if (err != SQLITE_OK || errmsg != NULL) {
		LOGGER_ERROR("Failed to execute SQL statement: %s", errmsg);
		set_last_error("Database transaction failed");
		ilog_error = ILOG_EINV;
		goto err;
	}

	LOGGER_DEBUG("Field names successfully set");
	sqlite3_free(errmsg);
	free(sql_statement);
	return ILOG_ESUCCESS;

err:
	LOGGER_DEBUG("Failed to set column names for table %s",
		     ILOG_SQL_TABLENAME);
	if (errmsg != NULL) {
		sqlite3_free(errmsg);
	}
	if (sql_statement != NULL) {
		free(sql_statement);
	}
	return ilog_error;
}


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


static ILogError try_db_open(ILogCtx* ctx, const char* filename)
{
	int err = 0;
	ILogError ilog_error = 0;
	sqlite3* db = NULL;
	Vector fields = NULL;

	LOGGER_DEBUG("Trying to open database");
	err = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if (err != SQLITE_OK || db == NULL) {
		LOGGER_DEBUG("Failed to open database at %s", filename);
		set_last_error("Failed to open database");
		ilog_error = ILOG_EFILE;
		goto error;
	}

	LOGGER_DEBUG("Opened database file at %s", filename);
	ilog_error = db_fetch_fields(db, &fields);
	if (ilog_error != ILOG_ESUCCESS) {
		LOGGER_ERROR("Failed to fetch database field names");
		goto error;
	}

	ctx->db = db;
	ctx->fields = fields;
	LOGGER_INFO("Opened database at %s", filename);
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	vector_destroy(fields);
	ctx->db = NULL;
	ctx->fields = NULL;
	return ilog_error;
}


static ILogError try_db_create_and_open(ILogCtx* ctx, const char* filename)
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
	ilog_error = db_set_fields(db, ctx->fields);
	if (ilog_error != ILOG_ESUCCESS) {
		LOGGER_ERROR("Failed to set database fields");
		goto error;
	}

	ctx->db = db;
	LOGGER_INFO("Created database at %s", filename);
	return ILOG_ESUCCESS;

error:
	sqlite3_close(db);
	ctx->db = NULL;
	return ilog_error;
}


#undef STRLEN
#undef VALLOC
#undef ALLOC


#endif /* ILOG_SQL */
