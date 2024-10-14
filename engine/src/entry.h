#pragma once

#include <core/logger.h>
#include <platform/platform.h>

int main(void)
{
    struct platform_state platform;
    if (!platform_startup(&platform, "App", 0, 0, 1280, 720)) {
        LFATAL("Failed to initialize platform");
    }
    platform_shutdown(&platform);

    return 0; 
}
