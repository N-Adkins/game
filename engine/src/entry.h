#pragma once

/**
 * @file
 * @brief Entrypoint
 *
 * Implements main so that the application just has to include it in order
 * to have the layers start.
 */

#include <core/memory.h>
#include <core/logger.h>
#include <platform/platform.h>

int main(void)
{
    struct platform_state platform;

    platform_startup(&platform, "App", 640, 360, 1280, 720);
    memory_startup();
    
    int *ptr = engine_allocate(sizeof(int), MEMORY_TAG_UNKNOWN);
    engine_free(ptr, sizeof(int), MEMORY_TAG_UNKNOWN);

    while (platform_poll_events(&platform)) {}

    memory_shutdown();
    platform_shutdown(&platform);

    return 0; 
}
