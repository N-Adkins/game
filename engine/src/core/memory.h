#pragma once

/**
 * @file
 * @brief memory subsystem
 *
 * all memory allocation should go through these facilities - they will log
 * and keep track of the memory usage of different parts of the program, and this
 * log can be dumped to the logger at any time.
 *
 * this is not a vtable interface like lots of other things use the term "allocator" for!
 * that would be super slow here. i could have made these global, and they used to be, but 
 * for the sake of correctness and so that hot-reloading is possible in the future (as i 
 * understand it, statics are reset when a dll is re-linked) it was changed to this api.
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
	MEMORY_TAG_UNKNOWN, /**< Avoid using this, mostly for debugging.  */
	MEMORY_TAG_ARRAY,
	MEMORY_TAG_DYNARRAY,
	MEMORY_TAG_VULKAN,

	MEMORY_TAG_MAX_TAGS,
};

/**
 * Initialize global memory state
 */
void memory_system_startup(void);

/**
 * Deinitialize global memory state
 */
void memory_system_shutdown(void);

/**
 * @brief General allocation function
 *
 * This should be used pretty much anywhere memory is allocated.
 *
 * @param size Allocation size in bytes
 * @param tag Allocation type
 */
LAPI void *engine_alloc(u64 size, enum memory_tag tag);

/**
 * @brief General free function
 *
 * This must be called once for every engine_allocate function.
 * The tag parameter must be the same as engine_allocate.
 *
 * @param ptr Pointer to free
 * @param size Allocation size in bytes, must be same as equivalent 
 * engine_alloc call.
 * @param tag Allocation type
 */
LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag);

/**
 * @brief Zeroes memory
 *
 * Sets "size" bytes from ptr to zero
 *
 * @param ptr Pointer to zero out
 * @param size Byte count that is to be zeroed
 */
LAPI void *engine_zero_memory(void *ptr, u64 size);

/**
 * @brief Copies memory
 *
 * Copies "size" bytes from source to dest. Source and dest memory blocks
 * **must not** overlap.
 */
LAPI void *engine_copy_memory(void *restrict dest, const void *restrict source,
			      u64 size);

/**
 * @brief Moves memory
 *
 * Copies "size" bytes from source to dest. Source and dest memory blocks
 * **may** overlap. There is overhead compared to engine_copy_memory due to
 * the allowance of overlapping blocks.
 */
LAPI void *engine_move_memory(void *dest, const void *source, u64 size);

/**
 * @brief Sets memory to value
 *
 * Sets "size" bytes from dest to the passed value.
 *
 * @param value Value for each byte
 */
LAPI void *engine_set_memory(void *dest, u8 value, u64 size);

/**
 * @brief Prints memory usage in logs
 *
 * Dumps a log at the debug level showing how much memory is allocated
 * with each memory_tag.
 */
LAPI void engine_dump_memory_usage(void);
