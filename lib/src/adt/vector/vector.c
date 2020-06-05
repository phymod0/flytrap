#include "vector.h"
#include "../../utils/macros.h"

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
	((unsigned long long)&(((VectorHeader*)NULL)->vector_data[0]))


static VectorHeader* vector_create_with_capacity(size_t capacity);
static VectorElementOps* copy_ops(const VectorElementOps* ops);
static void default_dtor(void* E);
static void* default_copy(void* E);
static int default_equal(void* A, void* B);
static int default_order(void* A, void* B);


void** vector_create(const VectorElementOps* ops)
{
	VectorHeader* header = NULL;
	VectorElementOps* ops_copy = NULL;

	if ((header = vector_create_with_capacity(MIN_CAPACITY)) == NULL) {
		goto oom;
	}
	if ((ops_copy = copy_ops(ops)) == NULL) {
		goto oom;
	}
	header->ops = ops_copy;
	return &header->vector_data[0];

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
	header = (void*)((char*)vector - VECTOR_DATA_OFFSET);
	free(header->ops);
	free(header);
}


int vector_insert(void** vector, void* data)
{
	/* TODO(phymod0): Implement */
	/*
	 * TODO(phymod0): Invariant: Size must be within interquartile range of
	 * capacity if capacity is not minimum. Otherwise size must be less than
	 * 3/4th of capacity.
	 */
	(void)vector;
	(void)data;
	return -1;
}


static VectorHeader* vector_create_with_capacity(size_t capacity)
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
		copy->dtor = default_dtor;
	}
	if (copy->copy == NULL) {
		copy->copy = default_copy;
	}
	if (copy->equal == NULL) {
		copy->equal = default_equal;
	}
	if (copy->order == NULL) {
		copy->order = default_order;
	}
	return copy;
}


static void default_dtor(void* E) { FT_UNUSED(E); }


static void* default_copy(void* E) { return E; }


static int default_equal(void* A, void* B)
{
	FT_UNUSED(A);
	FT_UNUSED(B);
	return 0;
}


static int default_order(void* A, void* B)
{
	FT_UNUSED(A);
	FT_UNUSED(B);
	return 0;
}

#undef VECTOR_DATA_OFFSET
#undef MIN_CAPACITY
