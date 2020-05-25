#error CSV-based logger not implemented

#include <stdbool.h>
#include <stddef.h>


/* TODO(phymod0): preproc guards for typedefs */

typedef enum {
	CSV_SUCCESS,
	CSV_NO_MEMORY,
	CSV_CURSOR_END,
	CSV_FILE_ERROR,
} CsvError;

typedef struct csv_ctx CsvCtx;
typedef struct csv_filter CsvFilter;
typedef struct csv_cursor CsvCursor;
typedef struct csv_log CsvLog;


/**
 * Creates and loads an empty csv log at the path specified in `filename`.
 *
 * The context will be allocated and initialized and it's pointer will be
 * written to `ctx`. If `CSV_SUCCESS` is returned, `ctx` must be freed with
 * `csv_destroy` once no longer needed. Otherwise `ctx` will remain unchanged.
 * @see csv_destroy
 *
 * @param filename C-string containing the path for the new log file
 * @param fields Array of the desired CSV field names
 * @param n_fields Number of the desired CSV field names
 * @param ctx Destination address to write the new context pointer to
 * @return CSV_NO_MEMORY, CSV_FILE_ERROR or CSV_SUCCESS
 */
CsvError csv_create_file(const char* filename, const char* fields[],
			 size_t n_fields, CsvCtx** ctx);


/**
 * Loads the csv log from the file specified in `filename`.
 *
 * The context will be allocated and initialized and it's pointer will be
 * written to `ctx`. If `CSV_SUCCESS` is returned, `ctx` must be freed with
 * `csv_destroy` once no longer needed. Otherwise `ctx` will remain unchanged.
 * @see csv_destroy
 *
 * @param filename C-string containing the path to the log file
 * @param ctx Destination address to write the new context pointer to
 * @return CSV_NO_MEMORY, CSV_FILE_ERROR or CSV_SUCCESS
 */
CsvError csv_load_from_file(const char* filename, CsvCtx** ctx);


/**
 * Frees a context.
 *
 * Frees the allocated memory and closes all files owned by `ctx`.
 *
 * @param ctx Context created by `csv_create_file` or `csv_load_from_file`
 */
void csv_destroy(CsvCtx* ctx);


/**
 * Creates a cursor for retrieving specific logs.
 *
 * Generates a cursor to iterate over logs in order of increasing age, starting
 * from the `start`th oldest log and satisfying the constraints imposed by
 * `filter`. If `CSV_SUCCESS` is returned, `cursor` must be freed with
 * `csv_cursor_destroy` once no longer needed. Otherwise, `cursor` will remain
 * unchanged.
 * @see csv_cursor_read
 * @see csv_cursor_step
 * @see csv_cursor_destroy
 *
 * @param ctx CSV logger context
 * @param start The log number to start at, or 0 to start at the most recent log
 * @param filter Pointer to a csv filter, or NULL to disable filtering
 * @param cursor Destination address to write the new cursor pointer to
 * @return CSV_NO_MEMORY or CSV_SUCCESS
 */
CsvError csv_get_logs(CsvCtx* ctx, size_t start, CsvFilter* filter,
		      CsvCursor** cursor);


/**
 * Frees a cursor.
 *
 * @param cursor Cursor created by `csv_get_logs`
 */
void csv_cursor_destroy(CsvCursor* cursor);


/**
 * Reads the log pointed to by a cursor.
 *
 * If `CSV_SUCCESS` is returned, `log` must be freed with `csv_log_destroy` once
 * no longer needed. Otherwise, `log` will remain unchanged. `CSV_CURSOR_END` is
 * returned if the cursor was flagged as ended by `csv_cursor_step`.
 * @see csv_log_destroy
 * @see csv_cursor_step
 *
 * @param cursor Pointer to a cursor
 * @param log Destination address to write the new log data to
 * @return CSV_NO_MEMORY, CSV_FILE_ERROR, CSV_CURSOR_END or CSV_SUCCESS
 */
CsvError csv_cursor_read(CsvCursor* cursor, CsvLog** log);


/**
 * Advances a cursor to the next relevant log.
 *
 * Updates `cursor` to point to the log immediately older that satisfies the
 * filters applied in `csv_get_logs`. If no such log exists, the cursor is
 * flagged as ended without any other modifications. `CSV_CURSOR_END` is
 * returned if the cursor is or was already flagged as ended.
 * @see csv_get_logs
 *
 * @param cursor Pointer to a cursor
 * @return CSV_NO_MEMORY, CSV_FILE_ERROR, CSV_CURSOR_END or CSV_SUCCESS
 */
CsvError csv_cursor_step(CsvCursor* cursor);


/**
 * Checks whether a cursor has been flagged as ended.
 *
 * @see csv_cursor_step
 * @param cursor Pointer to a cursor
 * @return `bool` indicating whether `cursor` has been flagged as ended
 */
bool csv_cursor_ended(CsvCursor* cursor);


/**
 * Creates and initializes an empty log entry.
 *
 * @see csv_log_set
 * @param log Destination address to write the new log pointer to
 * @return CSV_NO_MEMORY or CSV_SUCCESS
 */
CsvError csv_log_create(CsvLog** log);


/**
 * Gets a field value from a log.
 *
 * The return value will point to borrowed memory. The behavior may be undefined
 * if this pointer is freed, if the underlying data is modified or if the
 * underlying data is accessed after modifying the log. `NULL` is returned if
 * the required field name does not exist in the log.
 *
 * @param log Log data allocated by `csv_cursor_read` or `csv_log_create`
 * @param field_name Field name corresponding to the required value
 * @return Pointer to the borrowed value string
 */
char* csv_log_get(CsvLog* log, const char* field_name);


/**
 * Sets a field value on a log.
 *
 * The data pointed to by `value` will be copied rather than borrowed.
 *
 * @param log Log data allocated by `csv_cursor_read` or `csv_log_create`
 * @param field_name Field name corresponding to the required value
 * @param value C-string containing the value to copy from
 * @return CSV_NO_MEMORY or CSV_SUCCESS
 */
CsvError csv_log_set(CsvLog* log, const char* field_name, const char* value);


/**
 * Free allocated log data.
 *
 * Successful calls will render the data returned by `csv_log_get` invalid.
 * @see csv_log_get
 *
 * @param log Log data allocated by `csv_cursor_read` or `csv_log_create`
 */
void csv_log_destroy(CsvLog* log);


/**
 * Writes a log to the file handled by ctx.
 *
 * Field names that are not present in both `log` and the log file handled by
 * `ctx` will not be dealt with. Empty strings will be written as values of
 * field names not in `log` but present in the log file.
 *
 * @param ctx Context created by `csv_create_file` or `csv_load_from_file`
 * @param log Log data allocated by `csv_cursor_read` or `csv_log_create`
 * @return CSV_NO_MEMORY, CSV_FILE_ERROR or CSV_SUCCESS
 */
CsvError csv_write_log(CsvCtx* ctx, CsvLog* log);


/* TODO(phymod0): Write interfaces for filters */
/*
 * (Have each function return a local variable)
 * csv_filter_equal
 * csv_filter_str_greater
 * csv_filter_str_less
 * csv_filter_int_greater
 * csv_filter_int_less
 * csv_filter_inversion
 * csv_filter_conjuction
 * csv_filter_disjuction
 */
