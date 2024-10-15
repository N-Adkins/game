#include "memory.h"

#include <core/assert.h>
#include <core/logger.h>

static struct {
    u64 total_bytes;
    u64 tag_bytes[MEMORY_TAG_MAX_TAGS];
} memory_state;

void memory_startup(void)
{
    LINFO("Starting up memory subsystem");

    memory_state.total_bytes = 0;
    platform_zero_memory(
        memory_state.tag_bytes, 
        ARRAY_LENGTH(memory_state.tag_bytes) * sizeof(memory_state.tag_bytes[0])
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
    return platform_allocate(size, true);
}

LAPI void engine_free(void *ptr, u64 size, enum memory_tag tag)
{
    memory_state.total_bytes -= size;
    memory_state.tag_bytes[tag] -= size;
    platform_free(ptr, true);
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
    LWARN("TODO - Dump memory usage");
}
