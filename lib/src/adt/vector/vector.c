#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	size_t size;
	size_t capacity;
	VectorElementOps* ops;
	void* vector_data[];
} VectorHeader;


#define MIN_CAPACITY 16
#define VECTOR_DATA_OFFSET                                                     \
	((unsigned long long)((VectorHeader*)NULL)->vector_data)
#define UNUSED(x) ((void)(x))


/* Default VectorElementOps */
static void noop(void* E);
static void* identity(void* E);
static int binop_vanish(void* A, void* B);

/* Helper functions */
static VectorHeader* create_with_capacity(size_t capacity);
static VectorHeader* resize(VectorHeader* header, size_t new_capacity);
static VectorElementOps* copy_ops(const VectorElementOps* ops);
static inline VectorHeader* get_header(void** vector);
static inline void add_element(VectorHeader* header, void* element);
static inline size_t get_new_capacity(size_t new_size, size_t capacity);


void** vector_create(const VectorElementOps* ops)
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


void vector_destroy(void** vector)
{
	VectorHeader* header = NULL;
	if (vector == NULL) {
		return;
	}
	header = get_header(vector);
	free(header->ops);
	free(header);
}


int vector_insert(void*** vector_ptr, void* element)
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


size_t vector_size(void** vector) { return get_header(vector)->size; }


void vector_print(void** vector)
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
	void (*print)(void* element) = header->ops->print;

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


static void noop(void* E) { UNUSED(E); }


static void* identity(void* E) { return E; }


static int binop_vanish(void* A, void* B)
{
	UNUSED(A);
	UNUSED(B);
	return 0;
}


static VectorHeader* create_with_capacity(size_t capacity)
{
	VectorHeader* header =
	    malloc(sizeof *header + capacity * sizeof(void*));
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
	new_header =
	    realloc(header, sizeof *new_header + new_capacity * sizeof(void*));
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


static inline VectorHeader* get_header(void** vector)
{
	return (VectorHeader*)((char*)vector - VECTOR_DATA_OFFSET);
}


static inline void add_element(VectorHeader* header, void* element)
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
