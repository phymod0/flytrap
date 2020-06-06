/**
 * @file vector.h
 * @brief Data type for a vector of object pointers.
 */


#ifndef VECTOR
#define VECTOR


#include <stddef.h>


typedef void* VectorElement;
typedef const void* ConstVectorElement;
typedef VectorElement* Vector;

/** Operations on vector elements */
typedef struct {
	/** Destructor for the object pointed to by `E`. */
	void (*dtor)(VectorElement E);
	/** Display the object pointed to by `E`. */
	void (*print)(VectorElement E);
	/**
	 * Return a pointer to a copy of the object pointed to by `E`.
	 *
	 * `NULL` must only be returned to indicate a memory allocation failure.
	 */
	VectorElement (*copy)(ConstVectorElement E);
	/** Return non-zero iff `*A == *B`. */
	int (*equal)(VectorElement A, VectorElement B);
	/** Return `< 0` if `*A < *B`, `> 0` if `*A > *B`, and `0` otherwise. */
	int (*order)(VectorElement A, VectorElement B);
} VectorElementOps;


/**
 * Instantiate a vector.
 *
 * `ops` will be copied rather than borrowed. Operations can safely be passed as
 * `NULL` pointers as an alternative to their respective default implementations
 * (harmless no-ops for `dtor` and `print`, the identity function for `copy` and
 * the zero function for the rest). Methods are prone to memory leaks if `copy`
 * is provided without `dtor`. `ops` itself can be `NULL` as an alternative to
 * passing all operations as `NULL` pointers. The returned vector should be
 * accessed as if it were an array of `void` pointers to vector elements.
 *
 * @param ops Pointer to operations
 * @returns Pointer to the beginning of an empty vector or NULL if out of memory
 */
Vector vector_create(const VectorElementOps* ops);

/**
 * Destroy a vector.
 *
 * `dtor`, if provided to `vector_create`, will be called exactly once on each
 * `void` pointer in the vector. Calling this function on `NULL` is a harmless
 * no-op.
 * @see vector_create
 *
 * @param vector Pointer returned by `vector_create` or `NULL`
 */
void vector_destroy(Vector vector);

/**
 * Insert an element into a vector.
 *
 * This function takes the address of a vector pointer as the new vector may be
 * relocated. If successful, `element` will be appended to the new vector as-is
 * it's underlying object will not be copied. Otherwise no change will be made.
 *
 * @param vector Pointer to a pointer returned by `vector_create`
 * @param element Pointer to the element to be appended
 * @return `0` if successful, `-1` if out of memory
 */
int vector_insert(Vector* vector_ptr, VectorElement element);

/**
 * Insert an element into a vector.
 *
 * This function takes the address of a vector pointer as the new vector may be
 * relocated. If successful, a pointer to a copy of the underlying object of
 * `element` will be appended. Otherwise no change will be made. Copies will be
 * constructed using the `copy` function pointer passed to `vector_create`. If
 * `copy` was not provided, the call is equivalent to `vector_insert`.
 *
 * @param vector Pointer to a pointer returned by `vector_create`
 * @param element Pointer to the element to be copied and appended
 * @return `0` if successful, `-1` if out of memory
 */
int vector_copy_and_insert(Vector* vector_ptr, ConstVectorElement element);

/**
 * Get the number of elements present in a vector.
 *
 * @param vector Pointer returned by `vector_create`
 * @return Vector size
 */
size_t vector_size(Vector vector);

/**
 * Print a vector.
 *
 * Prints "NULL" on a dedicated line if called with `NULL`. Otherwise the size
 * and capacity of the vector are displayed. The list of vector elements will
 * also be displayed if the `print` operation was provided to `vector_create`.
 *
 * @param vector Pointer returned by `vector_create` or `NULL`
 */
void vector_print(Vector vector);


#endif /* VECTOR */
