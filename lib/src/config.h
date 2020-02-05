#ifndef FT_CONFIG
#define FT_CONFIG


#ifndef REST_PATH_MAXSZ
#define REST_PATH_MAXSZ 256
#endif /* REST_PATH_MAXSZ */


#ifndef REST_PATH_SEGMENT_WILDCARD
#define REST_PATH_SEGMENT_WILDCARD "<?>"
#endif /* REST_PATH_SEGMENT_WILDCARD */


#ifndef LOGGING_ENABLE
#ifdef NDEBUG
#define LOGGING_ENABLE 0
#else /* NDEBUG */
#define LOGGING_ENABLE 1
#endif /* NDEBUG */
#endif /* LOGGING_ENABLE */


#endif /* FT_CONFIG */
