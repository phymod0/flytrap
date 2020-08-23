#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	size_t size;
	size_t capacity;
	VectorElementOps* ops;
	VectorElement vector_data[];
} VectorHeader;


#define MIN_CAPACITY 16
#define VECTOR_DATA_OFFSET                                                     \
	((unsigned long long)((VectorHeader*)NULL)->vector_data)
#define UNUSED(x) ((void)(x))


/* Default VectorElementOps */
static void noop(VectorElement E);
static VectorElement identity(ConstVectorElement E);
static int binop_vanish(VectorElement A, VectorElement B);

/* Helper functions */
static VectorHeader* create_with_capacity(size_t capacity);
static VectorHeader* resize(VectorHeader* header, size_t new_capacity);
static VectorElementOps* copy_ops(const VectorElementOps* ops);
static inline VectorHeader* get_header(Vector vector);
static inline void add_element(VectorHeader* header, VectorElement element);
static inline size_t get_new_capacity(size_t new_size, size_t capacity);


Vector vector_create(const VectorElementOps* ops)
{
	VectorHeader* header = NULL;
	VectorElementOps* ops_copy = NULL;

	if ((header = create_with_capacity(MIN_CAPACITY)) == NULL) {
		goto oom;
	}
	if ((ops_copy = copy_ops(ops)) == NULL) {
		goto oom;
	}
	header->ops = ops_copy;
	return header->vector_data;

oom:
	if (header) {
		free(header);
	}
	if (ops_copy) {
		free(ops_copy);
	}
	return NULL;
}


void vector_destroy(Vector vector)
{
	if (vector == NULL) {
		return;
	}

	VectorHeader* header = get_header(vector);
	size_t size = header->size;
	VectorElementOps* ops = header->ops;
	void (*dtor)(VectorElement) = ops->dtor;

	for (size_t i = 0; i < size; ++i) {
		dtor(vector[i]);
	}
	free(ops);
	free(header);
}


int vector_insert(Vector* vector_ptr, VectorElement element)
{
	VectorHeader* header = get_header(*vector_ptr);
	size_t capacity = header->capacity;
	VectorHeader* new_header = NULL;
	size_t new_capacity = get_new_capacity(header->size + 1, capacity);
	VectorElementOps* ops = header->ops;

	if ((new_header = resize(header, new_capacity)) == NULL) {
		return -1;
	}
	new_header->ops = ops;
	add_element(new_header, element);
	*vector_ptr = new_header->vector_data;
	return 0;
}


int vector_copy_and_insert(Vector* vector_ptr, ConstVectorElement element)
{
	VectorHeader* header = get_header(*vector_ptr);
	VectorElementOps* ops = header->ops;
	void (*dtor)(VectorElement) = ops->dtor;
	void* (*copy)(ConstVectorElement) = ops->copy;

	VectorElement element_copy = copy(element);
	if (copy != NULL && element_copy == NULL) {
		return -1;
	}
	if (vector_insert(vector_ptr, element_copy) < 0) {
		dtor(element_copy);
		return -1;
	}
	return 0;
}


size_t vector_size(Vector vector) { return get_header(vector)->size; }


void vector_print(Vector vector)
{
	if (vector == NULL) {
		printf("NULL\n");
		return;
	}

	VectorHeader* header = get_header(vector);
	size_t size = header->size;
	size_t capacity = header->capacity;
	const char* header_fmt = "Size: %lu\n"
				 "Capacity: %lu\n";
	void (*print)(ConstVectorElement element) = header->ops->print;

	printf(header_fmt, size, capacity);

	if (print == NULL) {
		return;
	}
	printf("Elements: [");
	for (size_t i = 0; i < size; ++i) {
		printf("\n\t");
		print(header->vector_data[i]);
		printf(",");
	}
	printf(size > 0 ? "\n]\n" : " ]\n");
}


static void noop(VectorElement E) { UNUSED(E); }


static VectorElement identity(ConstVectorElement E) { return (VectorElement)E; }


static int binop_vanish(VectorElement A, VectorElement B)
{
	UNUSED(A);
	UNUSED(B);
	return 0;
}


static VectorHeader* create_with_capacity(size_t capacity)
{
	VectorHeader* header =
	    malloc(sizeof *header + capacity * sizeof(VectorElement));
	if (header == NULL) {
		return NULL;
	}
	header->size = 0;
	header->capacity = capacity;
	return header;
}


static VectorHeader* resize(VectorHeader* header, size_t new_capacity)
{
	VectorHeader* new_header = NULL;
	if (header->capacity == new_capacity) {
		return header;
	}
	new_header = realloc(header, sizeof *new_header +
					 new_capacity * sizeof(VectorElement));
	if (new_header != NULL) {
		new_header->capacity = new_capacity;
	}
	return new_header;
}


static VectorElementOps* copy_ops(const VectorElementOps* ops)
{
	VectorElementOps* copy = calloc(1, sizeof *copy);
	if (copy == NULL) {
		return NULL;
	}
	if (ops) {
		memcpy(copy, ops, sizeof *ops);
	}
	if (copy->dtor == NULL) {
		copy->dtor = noop;
	}
	if (copy->print == NULL) {
		copy->dtor = noop;
	}
	if (copy->copy == NULL) {
		copy->copy = identity;
	}
	if (copy->equal == NULL) {
		copy->equal = binop_vanish;
	}
	if (copy->order == NULL) {
		copy->order = binop_vanish;
	}
	return copy;
}


static inline VectorHeader* get_header(Vector vector)
{
	return (VectorHeader*)((char*)vector - VECTOR_DATA_OFFSET);
}


static inline void add_element(VectorHeader* header, VectorElement element)
{
	header->vector_data[header->size++] = element;
}


static inline size_t get_new_capacity(size_t new_size, size_t capacity)
{
	if (4 * new_size >= 3 * capacity) {
		return 2 * capacity;
	}
	if (capacity != MIN_CAPACITY && 4 * new_size < capacity) {
		return capacity / 2;
	}
	return capacity;
}


#undef UNUSED
#undef VECTOR_DATA_OFFSET
#undef MIN_CAPACITY
