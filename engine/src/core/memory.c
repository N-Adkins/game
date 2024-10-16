#include "memory.h"

#include <core/assert.h>
#include <core/logger.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Global memory state
 */
static struct {
    u64 total_bytes;
    u64 tag_bytes[MEMORY_TAG_MAX_TAGS];
} memory_state;

// Used for asserting if memory was started up
b8 initialized = false;

void memory_startup(void)
{
    LINFO("Starting up memory subsystem");

    platform_zero_memory(
        &memory_state,
        sizeof(memory_state)
    );

    initialized = true;
}

void memory_shutdown(void)
{
    LASSERT(initialized);
    LINFO("Shutting down memory subsystem");

    if (memory_state.total_bytes > 0) {
        LWARN("Memory has leaked, 0x%04llX bytes in use at shutdown", memory_state.total_bytes);
        dump_memory_usage();
    }
    
    // Consistency check
    u64 tag_total = 0;
    for (u64 i = 0; i < LARRAY_LENGTH(memory_state.tag_bytes); i++) {
        tag_total += memory_state.tag_bytes[i];
    }
    LASSERT(memory_state.total_bytes == tag_total);
}

LAPI void *engine_allocate(u64 size, enum memory_tag tag)
{
    LASSERT(initialized);

    if (tag == MEMORY_TAG_UNKNOWN) {
        LWARN("Allocating 0x%04llX bytes using MEMORY_TAG_UNKNOWN, change this tag", size);
    }

    memory_state.total_bytes += size;
    memory_state.tag_bytes[tag] += size;
    
    void *ptr = platform_allocate(size, false);
    platform_zero_memory(ptr, size);
    return ptr;
}

LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag)
{
    LASSERT(initialized);

    if (tag == MEMORY_TAG_UNKNOWN) {
        LWARN("Freeing 0x%04llX bytes using MEMORY_TAG_UNKNOWN, change this tag", size);
    }

    memory_state.total_bytes -= size;
    memory_state.tag_bytes[tag] -= size;
    platform_free(ptr, false);
}

LAPI void *engine_zero_memory(void *ptr, u64 size)
{
    LASSERT(initialized);
    return platform_zero_memory(ptr, size);
}

LAPI void *engine_copy_memory(void *restrict dest, const void *restrict source, u64 size)
{
    LASSERT(initialized);
    return platform_copy_memory(dest, source, size);
}

LAPI void *engine_set_memory(void *dest, i32 value, u64 size)
{
    LASSERT(initialized);
    return platform_set_memory(dest, value, size);
}

LAPI void dump_memory_usage(void)
{
    LASSERT(initialized);
    
    const char *tag_strings[] = {
        "UNKNOWN  ",
        "ARRAY    ",
        "DYNARRAY ",
        "STRING   ",
    };

    const u64 gib = 1024LU * 1024LU * 1024LU;
    const u64 mib = 1024LU * 1024LU;
    const u64 kib = 1024LU;

    const u64 array_len = LARRAY_LENGTH(memory_state.tag_bytes);
    
    char buffer[30000] = "Memory subsystem usage:\n";
    u64 offset = strlen(buffer);

    for (u64 i = 0; i < array_len; i++) {
        const char *unit = "GiB";
        f64 amount = 1.0;

        if (memory_state.tag_bytes[i] >= gib) {
            amount = (f64)memory_state.tag_bytes[i] / (f64)gib;
        } else if (memory_state.tag_bytes[i] >= mib) {
            unit = "MiB";
            amount = (f64)memory_state.tag_bytes[i] / (f64)mib;
        } else if (memory_state.tag_bytes[i] >= kib) {
            unit = "KiB";
            amount = (f64)memory_state.tag_bytes[i] / (f64)kib;
        } else {
            unit = "B";
            amount = (f64)memory_state.tag_bytes[i];
        }
        
        // This part is just for cleaner printing in the logger.
        if (i != array_len - 1) {
            offset += snprintf(
                buffer + offset, 
                LARRAY_LENGTH(buffer),
                "  %s: %.2lf%s\n",
                tag_strings[i], 
                amount,
                unit
            );
        } else {
            offset += snprintf(
                buffer + offset, 
                LARRAY_LENGTH(buffer),
                "  %s: %.2lf%s",
                tag_strings[i], 
                amount,
                unit
            );
        }
    }

    LDEBUG("%s", buffer);
}
