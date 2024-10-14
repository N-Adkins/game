#pragma once

#include <defines.h>

// TODO: other platforms lol
#define LPLATFORM_LINUX

struct platform_state {
    void *inner_state;
};

b8 platform_startup(
    struct platform_state *state,
    const char *app_name,
    i32 start_x,
    i32 start_y,
    i32 start_width,
    i32 start_height
);

void platform_shutdown(struct platform_state *state);
void platform_poll_events(struct platform_state *state);
