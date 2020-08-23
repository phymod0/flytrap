#ifndef FT_UTILS_MACROS
#define FT_UTILS_MACROS


/** Use to suppress unused variable warnings */
#define FT_UNUSED(x) ((void)(x))

/** Use to allocate a single instance of an object type */
#define FT_ALLOC(x) ((x) = malloc(sizeof *(x)))

/** Use to allocate multiple instances of an object type */
#define FT_VALLOC(x, n) ((x) = malloc((sizeof *(x)) * (n)))


#endif /* FT_UTILS_MACROS */
