/**
 * @file vector.h
 * @brief Data type for a vector of object pointers.
 */


#ifndef VECTOR
#define VECTOR


/** Operations on vector elements */
typedef struct {
	/** Destructor for the object pointed to by `E` */
	void (*dtor)(void* E);
	/** Return a pointer to a copy of the object pointed to by `E` */
	void* (*copy)(void* E);
	/** Return non-zero iff `*A == *B` */
	int (*equal)(void* A, void* B);
	/** Return `< 0` if `*A < *B`, `> 0` if `*A > *B`, and `0` otherwise */
	int (*order)(void* A, void* B);
} VectorElementOps;


/**
 * Instantiate a vector.
 *
 * `ops` will be copied rather than borrowed. Operations can safely be passed as
 * `NULL` pointers as an alternative to their respective stub implementations
 * (harmless no-op for `dtor`, the identity function for `copy` and the zero
 * function for the rest). `ops` itself can be `NULL` as an alternative to
 * passing all operations as `NULL` pointers. The returned vector should be
 * accessed as if it were an array of `void` pointers to vector elements.
 *
 * @param ops Pointer to operations
 * @returns Pointer to the beginning of an empty vector or NULL if out of memory
 */
void** vector_create(const VectorElementOps* ops);

/**
 * Destroy a vector.
 *
 * `dtor`, if provided to `vector_create`, will be called exactly once on each
 * `void` pointer in the vector.
 * @see vector_create
 *
 * @param vector A pointer returned by `vector_create`
 */
void vector_destroy(void* vector);

/**
 * Insert an element into a vector.
 *
 * TODO(phymod0)
 */

/* TODO(phymod0): Implement only the necessary parts */


#endif /* VECTOR */
