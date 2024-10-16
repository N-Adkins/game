#include "entry.h"

#include <core/assert.h>
#include <core/memory.h>
#include <core/logger.h>
#include <containers/dynarray.h>
#include <platform/platform.h>

LAPI int real_main(void)
{
    LDEBUG("Compiler: %s, Platform: %s", LCOMPILER, LPLATFORM);

    struct platform_state platform;
    
    const i32 START_X = 640;
    const i32 START_Y = 360;
    const i32 START_WIDTH = 1280;
    const i32 START_HEIGHT = 720;
    platform_startup(&platform, "App", START_X, START_Y, START_WIDTH, START_HEIGHT);
    memory_startup();
    
    int *ptr = engine_allocate(sizeof(int), MEMORY_TAG_UNKNOWN);
    engine_free(ptr, sizeof(int), MEMORY_TAG_UNKNOWN);
    
    // Testing some mutex error stuff
    struct mutex mutex = mutex_create();
    mutex_lock(&mutex);
    mutex_lock(&mutex);
    mutex_unlock(&mutex);
    mutex_unlock(&mutex);
    mutex_destroy(&mutex);

    while (platform_poll_events(&platform)) {}

    memory_shutdown();
    platform_shutdown(&platform);

    return 0; 
}
