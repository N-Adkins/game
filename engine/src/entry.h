#pragma once

/**
 * @file
 * @brief Entrypoint
 *
 * Implements main so that the application just has to include it in order
 * to have the layers start.
 */

#include <core/logger.h>
#include <platform/platform.h>

int main(void)
{
    struct platform_state platform;

    platform_startup(&platform, "App", 640, 360, 1280, 720);
    while (platform_poll_events(&platform)) {}
    platform_shutdown(&platform);

    return 0; 
}
