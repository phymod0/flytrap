/**
 * @file ilog.h
 * @brief Data structures and methods for handling interaction logs.
 */


#ifndef ILOG
#define ILOG


#include <stdbool.h>
#include <stddef.h>


/*
 * TODO(phymod0):
 *    Document error message conditions for each method
 *    Write CSV/JSON exportation methods
 */


enum ILogError {
	ILOG_ESUCCESS, /**< Success */
	ILOG_ENOMEM,   /**< Out of memory */
	ILOG_ECUREND,  /**< Error due to cursor end */
	ILOG_EFILE,    /**< File error */
	ILOG_EINV,     /**< Violated invariant */
};

struct ILogCtx;
struct ILogFilter;
struct ILogCursor;
struct ILog;

#ifndef ILOG_FWD
#define ILOG_FWD
typedef enum ILogError ILogError;
typedef struct ILogCtx ILogCtx;
typedef struct ILogFilter ILogFilter;
typedef struct ILogCursor ILogCursor;
typedef struct ILog ILog;
#endif /* ILOG_FWD */


/**
 * Creates and loads an empty log at the path specified in `filename`.
 *
 * The context will be allocated and initialized and it's pointer will be
 * written to `ctx`. If `ILOG_ESUCCESS` is returned, `ctx` must be freed with
 * `ilog_destroy` once no longer needed. Otherwise `*ctx` will be set to `NULL`.
 * If a valid database file already exists at path `filename` then calling this
 * function will have the same effect of calling `ilog_load_file` and the
 * `fields` and `n_fields` parameters will be ignored. If not ignored, the
 * strings in `fields` will be copied rather than borrowed. Long field names
 * will be truncated to 256 characters in length.
 * @see ilog_destroy
 * @see ilog_load_file
 *
 * @param filename C-string containing the path for the new log file
 * @param fields Array containing desired field names
 * @param n_fields Number of desired field names
 * @param ctx Destination address to write the new context pointer to
 * @return ILOG_ENOMEM, ILOG_EFILE, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_create_or_load_file(const char* filename, const char* fields[],
				   size_t n_fields, ILogCtx** ctx);

/**
 * Loads the log from the file specified in `filename`.
 *
 * The context will be allocated and initialized and it's pointer will be
 * written to `ctx`. If `ILOG_ESUCCESS` is returned, `ctx` must be freed with
 * `ilog_destroy` once no longer needed. Otherwise `*ctx` will be set to `NULL`.
 * `ILOG_EFILE` is returned if the log file at `filename` is absent. `ILOG_EINV`
 * is returned if the log file existed but failed to load.
 * @see ilog_destroy
 *
 * @param filename C-string containing the path to the log file
 * @param ctx Destination address to write the new context pointer to
 * @return ILOG_ENOMEM, ILOG_EFILE, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_load_file(const char* filename, ILogCtx** ctx);

/**
 * Frees a context.
 *
 * Frees the allocated memory and closes all files owned by `ctx`.
 * Calling this function on `NULL` is a harmless no-op.
 *
 * @param ctx Context created by `ilog_{create|load_from}_file` or `NULL`
 */
void ilog_destroy(ILogCtx* ctx);

/**
 * Creates a cursor for retrieving specific logs.
 *
 * Generates a cursor to iterate over logs in order of increasing age,
 * starting from the `start`th oldest log and satisfying the constraints
 * imposed by `filter`. If `ILOG_ESUCCESS` is returned, `cursor` must be
 * freed with `ilog_cursor_destroy` once no longer needed. Otherwise,
 * `cursor` will remain unchanged.
 * @see ilog_cursor_read
 * @see ilog_cursor_step
 * @see ilog_cursor_destroy
 *
 * @param ctx Context created by `ilog_create_or_load_file` or `ilog_load_file`
 * @param start The log number to start at, 0 denoting the most recent log
 * @param filter Pointer to an ilog filter, or NULL to disable filtering
 * @param cursor Destination address to write the new cursor pointer to
 * @return ILOG_ENOMEM, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_get_logs(ILogCtx* ctx, size_t start, ILogFilter* filter,
			ILogCursor** cursor);

/**
 * Frees a cursor.
 *
 * @param cursor Cursor created by `ilog_get_logs`
 */
void ilog_cursor_destroy(ILogCursor* cursor);

/**
 * Reads the log pointed to by a cursor.
 *
 * If `ILOG_ESUCCESS` is returned, `log` must be freed with `ilog_destroy_log`
 * once no longer needed. Otherwise, `log` will remain unchanged. `ILOG_ECUREND`
 * is returned if the cursor was flagged as ended by `ilog_cursor_step`.
 * @see ilog_destroy_log
 * @see ilog_cursor_step
 *
 * @param cursor Pointer to a cursor
 * @param log Destination address to write the new log data to
 * @return ILOG_ENOMEM, ILOG_EFILE, ILOG_ECUREND, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_cursor_read(ILogCursor* cursor, ILog** log);

/**
 * Advances a cursor to the next relevant log.
 *
 * Updates `cursor` to point to the log immediately older that satisfies the
 * filters applied in `ilog_get_logs`. If no such log exists, the cursor is
 * flagged as ended without any other modifications. `ILOG_ECUREND` is returned
 * if the cursor is or was already flagged as ended.
 * @see ilog_get_logs
 *
 * @param cursor Pointer to a cursor
 * @return ILOG_ENOMEM, ILOG_EFILE, ILOG_ECUREND, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_cursor_step(ILogCursor* cursor);

/**
 * Checks whether a cursor has been flagged as ended.
 *
 * @see ilog_cursor_step
 *
 * @param cursor Pointer to a cursor
 * @return `bool` indicating whether `cursor` has been flagged as ended
 */
bool ilog_cursor_ended(ILogCursor* cursor);

/**
 * Creates a blank log object.
 *
 * Use `ilog_set_field_value` to initialize.
 * @see ilog_set_field_value
 *
 * @return Pointer to a newly allocated log object or `NULL` if out of memory
 */
ILog* ilog_create_log();

/**
 * Gets a field value from a log.
 *
 * The return value will point to borrowed memory. The behavior may be
 * undefined if this pointer is freed, if the underlying data is
 * modified or if the underlying data is accessed after modifying the
 * log. `NULL` is returned if the required field name does not exist in
 * the log.
 *
 * @param log Log data allocated by `ilog_cursor_read` or `ilog_create_log`
 * @param field_name Field name corresponding to the required value
 * @return Pointer to the borrowed value string
 */
char* ilog_get_field_value(ILog* log, const char* field_name);

/**
 * Sets a field value on a log.
 *
 * The data pointed to by `value` will be copied rather than borrowed.
 *
 * @param log Log data allocated by `ilog_cursor_read` or `ilog_create_log`
 * @param field_name Field name corresponding to the required value
 * @param value C-string containing the value to copy from
 * @return ILOG_ENOMEM or ILOG_ESUCCESS
 */
ILogError ilog_set_field_value(ILog* log, const char* field_name,
			       const char* value);

/**
 * Free allocated log data.
 *
 * Successful calls will render the data returned by `ilog_get_field_value`
 * invalid.
 * @see ilog_get_field_value
 *
 * @param log Log data allocated by `ilog_cursor_read` or `ilog_create_log`
 */
void ilog_destroy_log(ILog* log);

/**
 * Writes a log to the file handled by ctx.
 *
 * Field names that are not present in both `log` and the log file
 * handled by `ctx` will not be dealt with. Empty strings will be
 * written as values of field names not in `log` but present in the log
 * file.
 *
 * @param ctx Context created by `ilog_create_or_load_file` or `ilog_load_file`
 * @param log Log data allocated by `ilog_cursor_read` or `ilog_create_log`
 * @return ILOG_ENOMEM, ILOG_EFILE, ILOG_EINV or ILOG_ESUCCESS
 */
ILogError ilog_write_log(ILogCtx* ctx, ILog* log);

/**
 * Creates an equality filter.
 *
 * Returns an ILogFilter object that selects logs with value V for field F
 * given F and V. `field_name` and `value` are copied rather than borrowed.
 *
 * @param field_name C-string representing F
 * @param value C-string representing V
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_equal(const char* field_name, const char* value);

/**
 * Creates a lexicographic lower bound filter.
 *
 * Returns an ILogFilter object that selects logs with value lexicographically
 * strictly greater than V for field F given F and V. `field_name` and `value`
 * are copied rather than borrowed.
 *
 * @param field_name C-string representing F
 * @param value C-string representing V
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_str_greater(const char* field_name, const char* value);

/**
 * Creates a lexicographic upper bound filter.
 *
 * Returns an ILogFilter object that selects logs with value lexicographically
 * strictly less than V for field F given F and V. `field_name` and `value` are
 * copied rather than borrowed.
 *
 * @param field_name C-string representing F
 * @param value C-string representing V
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_str_less(const char* field_name, const char* value);

/**
 * Creates a numeric lower bound filter.
 *
 * Returns an ILogFilter object that selects logs with value numerically
 * strictly greater than V for field F given F and V. `field_name` is copied
 * rather than borrowed.
 *
 * @param field_name C-string representing F
 * @param value Integer representing V
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_int_greater(const char* field_name,
				    long long int value);

/**
 * Creates a numeric upper bound filter.
 *
 * Returns an ILogFilter object that selects logs with value numerically
 * strictly less than V for field F given F and V. `field_name` is copied
 * rather than borrowed.
 *
 * @param field_name C-string representing F
 * @param value Integer representing V
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_int_less(const char* field_name, long long int value);

/**
 * Creates a filter conjunction.
 *
 * Returns an ILogFilter object that selects logs selected by both filter A and
 * filter B given A and B. `filter_A` and `filter_B` are borrowed rather than
 * copied.
 *
 * @param filter_A Pointer to a filter object representing A
 * @param filter_B Pointer to a filter object representing B
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_conjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B);

/**
 * Creates a filter disjunction.
 *
 * Returns an ILogFilter object that selects logs selected by either filter A
 * or filter B given A and B. `filter_A` and `filter_B` are borrowed rather
 * than copied.
 *
 * @param filter_A Pointer to a filter object representing A
 * @param filter_B Pointer to a filter object representing B
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_disjuction(const ILogFilter* filter_A,
				   const ILogFilter* filter_B);

/**
 * Inverts and returns a filter.
 *
 * Returns an ILogFilter object that selects all and only the logs not selected
 * by the given filter. `filter` will be borrowed rather than copied.
 *
 * @param filter Pointer to the filter object to invert
 * @return Pointer to allocated ILogFilter object or `NULL` if out of memory
 */
ILogFilter* ilog_filter_inversion(const ILogFilter* filter);

/**
 * Destroys a filter object.
 *
 * Frees memory for an ILogFilter object. Destroying a conjunction, disjunction
 * or inversion of a filter will also destroy the filter. The behavior is
 * undefined when a filter is destroyed more than once.
 *
 * @param filter Pointer to the filter object to destroy
 */
void ilog_filter_destroy(ILogFilter* filter);

/**
 * Returns a description of the most recent error.
 *
 * Returns a C-string describing the last error encountered by any of the
 * `ilog_*` functions. Returned strings should not be freed.
 *
 * @return C-string
 */
const char* ilog_last_error_str(void);


#endif /* ILOG */
