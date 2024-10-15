#pragma once

/**
 * @file
 * @brief Memory subsystem
 *
 * All memory allocation should go through these facilities - they will log
 * and keep track of the memory usage of different parts of the program, and this
 * log can be dumped to the logger at any time.
 */

#include <defines.h>

/**
 * @brief Type of memory allocation
 *
 * The different categories of allocation that are used in the engine, the type
 * MEMORY_TAG_UNKNOWN should be avoided and a warning will occur every time
 * it is used
 */
enum memory_tag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DYNARRAY,
    MEMORY_TAG_STRING,

    MEMORY_TAG_MAX_TAGS,
};

/**
 * @brief Initializes the global memory state
 */
void memory_startup(void);

/**
 * @brief Shuts down the global memory state
 *
 * Will print diagnostics if leaked memory is detected.
 */
void memory_shutdown(void);

/**
 * @brief General allocation function
 *
 * This should be used pretty much anywhere memory is allocated.
 *
 * @param tag Allocation type
 */
LAPI void *engine_allocate(u64 size, enum memory_tag tag);

/**
 * @brief General free function
 *
 * This must be called once for every engine_allocate function.
 * The tag parameter must be the same as engine_allocate.
 *
 * @param tag Allocation type
 */
LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag);

/**
 * @brief Zeroes memory
 *
 * Sets "size" bytes from ptr to zero
 */
LAPI void *engine_zero_memory(void *ptr, u64 size);

/**
 * @brief Copies memory
 *
 * Copies "size" bytes from source to dest. Source and dest memory blocks
 * must not overlap.
 */
LAPI void *engine_copy_memory(void *restrict dest, const void *restrict source, u64 size);

/**
 * @brief Sets memory to value
 *
 * Sets "size" bytes from dest to the passed value.
 *
 * @param value Value for each byte
 */
LAPI void *engine_set_memory(void *dest, i32 value, u64 size);

/**
 * @brief Prints memory usage in logs
 *
 * Dumps a log at the debug level showing how much memory is allocated
 * with each memory_tag.
 */
LAPI void dump_memory_usage(void);
