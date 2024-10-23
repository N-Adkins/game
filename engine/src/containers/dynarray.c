#include "dynarray.h"

#include <core/assert.h>
#include <core/memory.h>

LAPI struct dynarray dynarray_create(u64 stride)
{
	return dynarray_create_capacity(DYNARRAY_DEFAULT_CAPACITY, stride);
}

LAPI struct dynarray dynarray_create_capacity(u64 capacity, u64 stride)
{
	void *values = engine_alloc(stride * capacity, MEMORY_TAG_DYNARRAY);
	struct dynarray array = {
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

	engine_free(array->values, array->capacity * array->stride,
		    MEMORY_TAG_DYNARRAY);
}

LAPI void dynarray_resize(struct dynarray *array, u64 capacity)
{
	LASSERT(array != NULL);

	// Effectively a realloc here
	void *values =
		engine_alloc(array->stride * capacity, MEMORY_TAG_DYNARRAY);
	engine_copy_memory(values, array->values,
			   array->length * array->stride);
	engine_free(array->values, array->capacity * array->stride,
		    MEMORY_TAG_DYNARRAY);

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
	engine_copy_memory(result, char_array + (index * array->stride),
			   array->stride);

	return true;
}

LAPI void dynarray_remove(struct dynarray *array, u64 index)
{
	LASSERT(array != NULL);

	if (index >= array->length) {
		LERROR("Dynarray out of bounds removal at index %llu with length %llu",
		       index, array->length);
	}

	// If last element
	if (index == array->length - 1) {
		array->length--;
		return;
	}

	const u64 size_after = array->length - index - 1;
	char *dest = ((char *)array->values) + index;
	const char *source = ((char *)array->values) + index + 1;
    array->length--;
    engine_move_memory(dest, source, size_after);
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
	engine_copy_memory(char_array + (array->stride * array->length), value,
			   array->stride);

	array->length++;
}
