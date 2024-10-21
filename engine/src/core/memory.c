#include "memory.h"

#include <core/assert.h>
#include <core/logger.h>
#include <stdio.h>
#include <string.h>

LAPI struct allocator allocator_create(void)
{
	struct allocator allocator;

	platform_zero_memory(&allocator, sizeof(struct allocator));

	return allocator;
}

LAPI void allocator_destroy(struct allocator *allocator)
{
	LASSERT(allocator != NULL);

	if (allocator->total_bytes > 0) {
		LWARN("Memory has leaked, 0x%04llX bytes in use at allocator destruction, dumping usage",
		      allocator->total_bytes);
		allocator_dump_usage(allocator);
	}

	// Consistency check
	u64 tag_total = 0;
	for (u64 i = 0; i < LARRAY_LENGTH(allocator->tag_bytes); i++) {
		tag_total += allocator->tag_bytes[i];
	}
	LASSERT(allocator->total_bytes == tag_total);
	(void)tag_total;
}

LAPI void *allocator_alloc(struct allocator *allocator, u64 size,
			   enum memory_tag tag)
{
	LASSERT(allocator != NULL);

	if (tag == MEMORY_TAG_UNKNOWN) {
		LWARN("Allocating 0x%04llX bytes using MEMORY_TAG_UNKNOWN, change this tag",
		      size);
	}

	allocator->total_bytes += size;
	allocator->tag_bytes[tag] += size;

	void *ptr = platform_allocate(size, false);
	platform_zero_memory(ptr, size);
	return ptr;
}

LAPI void allocator_free(struct allocator *allocator, void *ptr, u64 size,
			 enum memory_tag tag)
{
	LASSERT(allocator != NULL);

	if (tag == MEMORY_TAG_UNKNOWN) {
		LWARN("Freeing 0x%04llX bytes using MEMORY_TAG_UNKNOWN, change this tag",
		      size);
	}

	allocator->total_bytes -= size;
	allocator->tag_bytes[tag] -= size;
	platform_free(ptr, false);
}

LAPI void *allocator_zero_memory(struct allocator *allocator, void *ptr,
				 u64 size)
{
	LASSERT(allocator != NULL);
	(void)allocator;

	return platform_zero_memory(ptr, size);
}

LAPI void *allocator_copy_memory(struct allocator *allocator,
				 void *restrict dest,
				 const void *restrict source, u64 size)
{
	LASSERT(allocator != NULL);
	(void)allocator;

	return platform_copy_memory(dest, source, size);
}

LAPI void *allocator_move_memory(struct allocator *allocator, void *dest,
				 const void *source, u64 size)
{
	LASSERT(allocator != NULL);
	(void)allocator;

	return platform_move_memory(dest, source, size);
}

LAPI void *allocator_set_memory(struct allocator *allocator, void *dest,
				i32 value, u64 size)
{
	LASSERT(allocator != NULL);
	(void)allocator;

	return platform_set_memory(dest, value, size);
}

LAPI void allocator_dump_usage(struct allocator *allocator)
{
	LASSERT(allocator != NULL);
	(void)allocator;

	const char *tag_strings[] = {
		"UNKNOWN  ",
		"ARRAY    ",
		"DYNARRAY ",
		"STRING   ",
	};

	const u64 gib = 1024LU * 1024LU * 1024LU;
	const u64 mib = 1024LU * 1024LU;
	const u64 kib = 1024LU;

	const u64 array_len = LARRAY_LENGTH(allocator->tag_bytes);

	char buffer[LOG_MAX_LENGTH] = "Memory subsystem usage:\n";
	u64 offset = strlen(buffer);

	for (u64 i = 0; i < array_len; i++) {
		const char *unit = "GiB";
		f64 amount = 1.0;

		if (allocator->tag_bytes[i] >= gib) {
			amount = (f64)allocator->tag_bytes[i] / (f64)gib;
		} else if (allocator->tag_bytes[i] >= mib) {
			unit = "MiB";
			amount = (f64)allocator->tag_bytes[i] / (f64)mib;
		} else if (allocator->tag_bytes[i] >= kib) {
			unit = "KiB";
			amount = (f64)allocator->tag_bytes[i] / (f64)kib;
		} else {
			unit = "B";
			amount = (f64)allocator->tag_bytes[i];
		}

		// This part is just for cleaner printing in the logger.
		if (i != array_len - 1) {
			offset += (u64)snprintf(buffer + offset,
						LARRAY_LENGTH(buffer),
						"  %s: %.2lf%s\n",
						tag_strings[i], amount, unit);
		} else {
			offset += (u64)snprintf(buffer + offset,
						LARRAY_LENGTH(buffer),
						"  %s: %.2lf%s", tag_strings[i],
						amount, unit);
		}
	}

	LDEBUG("%s", buffer);
}
