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
    MEMORY_TAG_STRING,

    MEMORY_TAG_MAX_TAGS,
};

void memory_startup(void);
void memory_shutdown(void);

LAPI void *engine_allocate(u64 size, enum memory_tag tag);
LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag);

LAPI void *engine_zero_memory(void *ptr, u64 size);
LAPI void *engine_copy_memory(void *restrict dest, const void *restrict source, u64 size);
LAPI void *engine_set_memory(void *dest, i32 value, u64 size);

LAPI void dump_memory_usage(void);
