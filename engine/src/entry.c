#include "entry.h"

#include <core/assert.h>
#include <core/memory.h>
#include <core/logger.h>
#include <containers/dynarray.h>
#include <platform/platform.h>

LAPI int real_main(void)
{
    LDEBUG("Compiler: %s, Platform: %s", LCOMPILER, LPLATFORM);
    
    const i32 START_X = 640;
    const i32 START_Y = 360;
    const i32 START_WIDTH = 1280;
    const i32 START_HEIGHT = 720;
    struct platform platform;
    platform_startup(&platform, "App", START_X, START_Y, START_WIDTH, START_HEIGHT);

    struct allocator alloc = allocator_create();
    int *ptr = allocator_alloc(&alloc, sizeof(int), MEMORY_TAG_UNKNOWN);
    allocator_free(&alloc, ptr, sizeof(int), MEMORY_TAG_UNKNOWN);
    
    LDEBUG("Alloc: %p", &alloc);
    struct dynarray array = dynarray_create(&alloc, sizeof(int));
    dynarray_push(&array, 120312);
    dynarray_get(&array, 1, ptr);
    
    // Testing some mutex error stuff
    struct mutex mutex = mutex_create();
    mutex_lock(mutex);
    mutex_lock(mutex);
    mutex_unlock(mutex);
    mutex_unlock(mutex);
    mutex_destroy(mutex);

    while (platform_poll_events(platform)) {}

    allocator_destroy(&alloc);
    platform_shutdown(platform);
    
    return 0; 
}
