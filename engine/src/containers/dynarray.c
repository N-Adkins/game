#include "dynarray.h"

#include <core/assert.h>
#include <core/memory.h>

LAPI struct dynarray dynarray_create(struct allocator *allocator, u64 stride)
{
	LASSERT(allocator != NULL);

	return dynarray_create_capacity(allocator, DYNARRAY_DEFAULT_CAPACITY,
					stride);
}

LAPI struct dynarray dynarray_create_capacity(struct allocator *allocator,
					      u64 capacity, u64 stride)
{
	LASSERT(allocator != NULL);

	void *values = allocator_alloc(allocator, stride * capacity,
				       MEMORY_TAG_DYNARRAY);
	struct dynarray array = {
		.allocator = allocator,
		.capacity = capacity,
		.length = 0,
		.stride = stride,
		.values = values,
	};
	return array;
}

LAPI void dynarray_destroy(struct dynarray *array)
{
	LASSERT(array != NULL);

	allocator_free(array->allocator, array->values,
		       array->capacity * array->stride, MEMORY_TAG_DYNARRAY);
}

LAPI void dynarray_resize(struct dynarray *array, u64 capacity)
{
	LASSERT(array != NULL);

	// Effectively a realloc here
	void *values = allocator_alloc(array->allocator,
				       array->stride * capacity,
				       MEMORY_TAG_DYNARRAY);
	allocator_copy_memory(array->allocator, values, array->values,
			      array->length * array->stride);
	allocator_free(array->allocator, array->values,
		       array->capacity * array->stride, MEMORY_TAG_DYNARRAY);

	array->capacity = capacity;
	array->values = values;
}

LAPI b8 dynarray_get(struct dynarray *array, u64 index, void *result)
{
	LASSERT(array != NULL);
	LASSERT(result != NULL);

	if (index >= array->length) {
		LERROR("Dynarray out of bounds access at index %llu with length %llu",
		       index, array->length);
		return false;
	}

	// Must cast to char pointer to do arithmetic
	char *char_array = array->values;
	allocator_copy_memory(array->allocator, result,
			      char_array + (index * array->stride),
			      array->stride);

	return true;
}

LAPI void dynarray_push_ptr(struct dynarray *array, const void *value)
{
	LASSERT(array != NULL);
	LASSERT(value != NULL);

	// Resize if too large
	if (array->length >= array->capacity) {
		dynarray_resize(array,
				array->capacity * DYNARRAY_RESIZE_FACTOR);
	}

	// Must cast to char pointer to do arithmetic
	char *char_array = array->values;
	allocator_copy_memory(array->allocator,
			      char_array + (array->stride * array->length),
			      value, array->stride);

	array->length++;
}
