#include "memory.h"

#include <core/assert.h>
#include <core/logger.h>
#include <stdio.h>
#include <string.h>

static struct {
    u64 total_bytes;
    u64 tag_bytes[MEMORY_TAG_MAX_TAGS];
} memory_state;

void memory_startup(void)
{
    LINFO("Starting up memory subsystem");

    platform_zero_memory(
        &memory_state,
        sizeof(memory_state)
    );
}

void memory_shutdown(void)
{
    LINFO("Shutting down memory subsystem");

    if (memory_state.total_bytes > 0) {
        LWARN("Memory has leaked, 0x%04X bytes in use at shutdown", memory_state.total_bytes);
        dump_memory_usage();
    }
    
    // Consistency check
    u64 tag_total = 0;
    for (u64 i = 0; i < ARRAY_LENGTH(memory_state.tag_bytes); i++) {
        tag_total += memory_state.tag_bytes[i];
    }
    LASSERT(memory_state.total_bytes == tag_total);
}

LAPI void *engine_allocate(u64 size, enum memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LWARN("Allocating 0x%04X bytes using MEMORY_TAG_UNKNOWN, change this tag", size);
    }

    memory_state.total_bytes += size;
    memory_state.tag_bytes[tag] += size;
    
    void *ptr = platform_allocate(size, false);
    platform_zero_memory(ptr, size);
    return ptr;
}

LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN) {
        LWARN("Freeing 0x%04X bytes using MEMORY_TAG_UNKNOWN, change this tag", size);
    }

    memory_state.total_bytes -= size;
    memory_state.tag_bytes[tag] -= size;
    platform_free(ptr, false);
}

LAPI void *engine_zero_memory(void *ptr, u64 size)
{
    return platform_zero_memory(ptr, size);
}

LAPI void *engine_copy_memory(void *restrict dest, const void *restrict source, u64 size)
{
    return platform_copy_memory(dest, source, size);
}

LAPI void *engine_set_memory(void *dest, i32 value, u64 size)
{
    return platform_set_memory(dest, value, size);
}

LAPI void dump_memory_usage(void)
{
    const char *tag_strings[] = {
        "UNKNOWN ",
        "ARRAY   ",
        "STRING  ",
    };

    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;
    
    char buffer[30000] = "Memory subsystem usage:\n";
    u64 offset = strlen(buffer);
    for (u64 i = 0; i < ARRAY_LENGTH(memory_state.tag_bytes); i++) {
        const char *unit = "GiB";
        float amount = 1.f;

        if (memory_state.tag_bytes[i] >= gib) {
            amount = memory_state.tag_bytes[i] / (float)gib;
        } else if (memory_state.tag_bytes[i] >= mib) {
            unit = "MiB";
            amount = memory_state.tag_bytes[i] / (float)mib;
        } else if (memory_state.tag_bytes[i] >= kib) {
            unit = "KiB";
            amount = memory_state.tag_bytes[i] / (float)kib;
        } else {
            unit = "B";
            amount = (float)memory_state.tag_bytes[i];
        }

        offset += snprintf(
            buffer + offset, 
            ARRAY_LENGTH(buffer),
            "  %s: %.2f%s\n", 
            tag_strings[i], 
            amount,
            unit
        );
    }

    LDEBUG("%s", buffer);
}
