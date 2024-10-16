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

    platform_startup(&platform, "App", 640, 360, 1280, 720);
    memory_startup();
    
    int *ptr = engine_allocate(sizeof(int), MEMORY_TAG_UNKNOWN);
    engine_free(ptr, sizeof(int), MEMORY_TAG_UNKNOWN);

    struct dynarray array = dynarray_create(sizeof(int));
    dynarray_push(&array, 10);

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
