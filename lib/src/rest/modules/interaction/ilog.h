/**
 * @file ilog.h
 * @brief Data structures and methods for handling interaction logs.
 */


#ifndef ILOG
#define ILOG


#include <stdbool.h>
#include <stddef.h>
#include <time.h>


/*
 * TODO(phymod0):
 *    - Implement CSV/JSON/SQL/bin partial saving, full saving and loading
 */


enum ILogError {
	ILOG_ESUCCESS, /**< Success */
	ILOG_ENOMEM,   /**< Out of memory */
	ILOG_ECUREND,  /**< Error due to cursor end */
	ILOG_ELOAD,    /**< Failed to load/create a log file */
	ILOG_EREAD,    /**< Failed to read */
	ILOG_EUPDATE,  /**< Failed to update */
	ILOG_N_ERRORS, /**< Number of error codes */
};

struct ILogList;
struct ILogFilter;
struct ILogCursor;
struct ILog;

#ifndef ILOG_FWD
#define ILOG_FWD
typedef unsigned int ILogID;
typedef enum ILogError ILogError;
typedef struct ILogList ILogList;
typedef struct ILogFilter ILogFilter;
typedef struct ILogCursor ILogCursor;
typedef struct ILog ILog;
#endif /* ILOG_FWD */


/**
 * Creates an empty log list.
 *
 * If the list is successfully allocated and initialized, it's pointer will be
 * written at `ilog_list` which must be freed with `ilog_list_destroy` once no
 * longer needed and `ILOG_ESUCCESS` will be returned. Otherwise `*ilog_list`
 * will be set to `NULL` and the function will return `ILOG_ENOMEM`.
 * @see ilog_list_destroy
 *
 * @param ilog_list Destination address to write the new ilog list pointer to
 * @return `ILOG_ENOMEM` or `ILOG_ESUCCESS`
 */
ILogError ilog_list_create(ILogList** ilog_list);

/**
 * Frees a ilog list.
 *
 * Frees the allocated memory for an ilog list. Calling this function on `NULL`
 * is a harmless no-op.
 *
 * @param ilog_list List pointer created by `ilog_list_create` or `NULL`
 */
void ilog_list_destroy(ILogList* ilog_list);

/**
 * Inserts a record into an ilog list.
 *
 * The timestamp of the ilog list is roughly set to the time of insertion.
 * `ILOG_ENOMEM` is returned if memory is insufficient for the insertion,
 * otherwise `ILOG_ESUCCESS` is returned. `json_data` and `mac_addr` will be
 * copied rather than borrowed. `json_data` may be `NULL` if not applicable.
 *
 * @param ilog_list List pointer created by `ilog_list_create`
 * @param mac_addr MAC address of the device this log is recorded from
 * @param type Interaction type
 * @param subtype Subtype of the interaction type
 * @param json_data `NULL` or a C-string containing the log's JSON data
 */
ILogError ilog_list_insert(ILogList* ilog_list, const unsigned char* mac_addr,
			   int type, int subtype, const char* json_data);

/**
 * Creates a cursor for retrieving specific logs.
 *
 * Generates a cursor to iterate over logs in order of increasing age from or
 * after the log with ID `start` satisfying `filter`. If `ILOG_ESUCCESS` is
 * returned, `cursor` must be freed with `ilog_cursor_destroy` once no longer
 * needed. Otherwise, `ILOG_ENOMEM` will be returned and `cursor` will remain
 * unchanged.
 * @see ilog_cursor_read
 * @see ilog_cursor_step
 * @see ilog_cursor_destroy
 *
 * @param ilog_list List pointer created by `ilog_list_create`
 * @param start The log number to start at, `0` denoting the youngest log
 * @param filter Pointer to an ilog filter, or `NULL` to disable filtering
 * @param cursor Destination address to write the new cursor pointer to
 * @return `ILOG_ENOMEM` or `ILOG_ESUCCESS`
 */
ILogError ilog_get_logs(ILogList* ilog_list, ILogID start, ILogFilter* filter,
			ILogCursor** cursor_dst);

/**
 * Frees a cursor.
 *
 * @param cursor Cursor created by `ilog_get_logs`
 */
void ilog_cursor_destroy(ILogCursor* cursor);

/**
 * Reads the log pointed to by a cursor.
 *
 * `NULL` is returned if the cursor was flagged as ended during a call to
 * `ilog_cursor_step` or at initialization. Otherwise, a pointer to a borrowed
 * ilog object is written at `log` which must neither be freed nor modified.
 * The behavior of reading `**log` after freeing the parent list is undefined.
 * @see ilog_cursor_step
 *
 * @param cursor Pointer to an ilog cursor
 * @return Borrowed pointer to an ilog object
 */
const ILog* ilog_cursor_read(ILogCursor* cursor);

/**
 * Advances a cursor to the next relevant log.
 *
 * Updates `cursor` to point to the immediately older log that satisfies the
 * filters applied in `ilog_get_logs`. If no such log exists, the cursor is
 * flagged as ended without any other modifications. Calling this function on an
 * ended cursor is a harmless no-op.
 * @see ilog_get_logs
 *
 * @param cursor Pointer to an ilog cursor
 */
void ilog_cursor_step(ILogCursor* cursor);

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
 * Gets the timestamp value from a log.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return Unix timestamp of the log as a `time_t` value
 */
time_t ilog_get_timestamp(ILog* log);

/**
 * Gets the ID from a log.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return ID of the log
 */
ILogID ilog_get_id(ILog* log);

/**
 * Gets the MAC address from a log.
 *
 * A pointer to borrowed memory is returned which must not be freed manually.
 * The behavior of dereferencing the returned pointer after destroying the
 * parent log list is undefined.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return 6-byte `unsigned char` array representing the MAC address
 */
const unsigned char* ilog_get_mac_addr(ILog* log);

/**
 * Gets the interaction type from a log.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return Integer representing the interaction type of the log
 */
int ilog_get_type(ILog* log);

/**
 * Gets the interaction subtype from a log.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return Integer representing the interaction subtype of the log
 */
int ilog_get_subtype(ILog* log);

/**
 * Gets the JSON data from a log.
 *
 * `NULL` is returned if the JSON provided in `ilog_list_insert` was `NULL`.
 * Otherwise a pointer to borrowed memory is returned which must not be freed
 * manually. The behavior of dereferencing the returned pointer after destroying
 * the parent log list is undefined.
 *
 * @param log Log data provided by `ilog_cursor_read`
 * @return `NULL` or the same C-string pointer provided in `ilog_list_insert`
 */
const char* ilog_get_json_data(ILog* log);

/* TODO: Redesign filtering interface */

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
 * Returns a description of an error code.
 *
 * Returns a C-string describing the error an `ILogError` value represents.
 * Returned strings must not be freed.
 *
 * @param err `ILogError` error code to translate
 *
 * @return C-string representing the error description
 */
const char* ilog_error_description(ILogError err);


#endif /* ILOG */
