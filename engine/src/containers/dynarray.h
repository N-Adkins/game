#pragma once

/**
 * @file
 * @brief Dynamic list container
 *
 * A dynamic list structure (similar to std::vector) that resizes when it is full.
 * This container provides a couple of convenience macros to lighten up the syntax
 * since it is fully type erased.
 */

#include <defines.h>

#define DYNARRAY_DEFAULT_CAPACITY 16
#define DYNARRAY_RESIZE_FACTOR 2

/**
 * @brief Dynamic array container
 */
struct dynarray {
    u64 capacity;
    u64 length;
    u64 stride; // type size in bytes
    void *values; // values[0] guaranteed to be first element in array
};

/**
  * @brief Initialize dynamic array
  *
  * This initializes a dynamic array with the passed stride and with
  * DYNARRAY_DEFUALT_CAPACITY capacity, it is just a wrapper to a call
  * to dynarray_create_capacity with that value.
  *
  * This **does** allocate.
  */
LAPI struct dynarray dynarray_create(u64 stride);

/**
  * @brief Initialize dynamic array with passed capacity
  *
  * This **does** allocate.
  */
LAPI struct dynarray dynarray_create_capacity(u64 capacity, u64 stride);

/**
  * @brief Frees all memory in a dynarray
  *
  * This does **not** attempt to do anything with the actual values, that is 
  * completely up to the caller.
  */
LAPI void dynarray_destroy(struct dynarray *array);

/**
  * @brief Change dynarray capacity to passed capacity
  *
  * This is used internally when appending to the dynarray.
  */
LAPI void dynarray_resize(struct dynarray *array, u64 capacity);

/**
  * @brief Get element from dynarray
  *
  * Will print an error if the passed index is out of bounds.
  *
  * @return true if element existed, false if out of bounds
  */
LAPI b8 dynarray_get(struct dynarray *array, u64 index, void *result);

/**
  * @brief Push element to dynarray by reference
  *
  * Under the mood this runs a memcpy to copy the value into the array.
  */
LAPI void dynarray_push_ptr(struct dynarray *array, const void *value);

/**
  * @brief Push element to dynarray by value
  *
  * This is just syntactic sugar over dynarray_push_ptr since
  * it can be a pain for literals sometimes. Prefer using
  * the ptr variation when able.
  */
#define dynarray_push(array, value)                                 \
    do {                                                            \
        LTYPEOF(value) LUNIQUE_ID(0) = (value);                     \
        dynarray_push_ptr((array), &LUNIQUE_ID(0));                 \
    } while(0)
