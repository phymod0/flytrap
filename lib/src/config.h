#ifndef FT_CONFIG
#define FT_CONFIG


#ifndef REST_PATH_MAXSZ
#define REST_PATH_MAXSZ 256
#endif /* REST_PATH_MAXSZ */


#ifndef REST_PATH_MAX_WILDCARDS
#define REST_PATH_MAX_WILDCARDS 64
#endif /* REST_PATH_MAX_WILDCARDS */


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


#ifndef DEFAULT_DOCROOT
#ifdef _WIN32
#define DEFAULT_DOCROOT "%UserProfile%\Desktop\flytrap\"
#elif defined(__APPLE__) || defined(__MACH__)
#define DEFAULT_DOCROOT "~/Desktop/flytrap/"
#elif defined(__unix__) || defined(__linux__)
#define DEFAULT_DOCROOT "~/flytrap/"
#elif defined(__ANDROID__)
/* TODO(phymod0) Define the default document root for android */
#error Define the default document root for android
#endif /* DEFAULT_DOCROOT */


#endif /* FT_CONFIG */
