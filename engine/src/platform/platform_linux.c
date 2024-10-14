#include "platform.h"

#ifdef LPLATFORM_LINUX

#include <core/assert.h>
#include <core/logger.h>
#include <stdlib.h>

#include <X11/Xlib.h>

struct linux_platform_state {
    Display *display;
    Window window;
};

b8 platform_startup(
    struct platform_state *state,
    const char *app_name,
    i32 start_x,
    i32 start_y,
    i32 start_width,
    i32 start_height
)
{
    (void)app_name;

    LINFO("Initializing the Linux platform state");

    struct linux_platform_state *linux_state = malloc(sizeof(struct linux_platform_state));
    state->inner_state = linux_state;

    linux_state->display = XOpenDisplay(NULL); // fetch default display
    if (linux_state->display == NULL) {
        LFATAL("Failed to open X11 display");
        return false;
    }

    Window root_window = XDefaultRootWindow(linux_state->display);
    if (root_window <= 0) {
        LFATAL("Failed to fetch default X11 window");
        return false;
    }

    linux_state->window = XCreateSimpleWindow(
        linux_state->display,
        root_window,
        start_x,
        start_y,
        start_width,
        start_height,
        0, // no border
        BlackPixel(linux_state->display, root_window),
        BlackPixel(linux_state->display, root_window)
    );
    if (linux_state->window <= 0) {
        LFATAL("Failed to create X11 window");
        return false;
    }

    return true;
}

void platform_shutdown(struct platform_state *state)
{
    LINFO("Shutting down the Linux platform state");

    struct linux_platform_state *linux_state = state->inner_state;

    XCloseDisplay(linux_state->display);

    free(state->inner_state);
}

#endif
