#include "platform.h"

#ifdef LPLATFORM_LINUX

#include <core/assert.h>
#include <core/logger.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

struct linux_platform_state {
    Display *display;
    Window window;
    Atom wm_delete;
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
        BlackPixel(linux_state->display, 0),
        BlackPixel(linux_state->display, 0)
    );
    if (linux_state->window <= 0) {
        LFATAL("Failed to create X11 window");
        return false;
    }

    if (!XMapWindow(linux_state->display, linux_state->window)) {
        LFATAL("Failed to map X11 window to display");
        return false;
    }

    const long event_mask = 
        KeyPressMask | 
        KeyReleaseMask |
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask;

    if (!XSelectInput(linux_state->display, linux_state->window, event_mask)) {
        LFATAL("Failed to enable X11 event mask");
        return false;
    }

    linux_state->wm_delete = XInternAtom(
        linux_state->display,
        "WM_DELETE_WINDOW",
        false
    );

    if (!XSetWMProtocols(
        linux_state->display,
        linux_state->window,
        &linux_state->wm_delete,
        1
    )) {
        LFATAL("Failed to set X11 WM protocols");
        return false;
    }
    
    // Set window name to app_name
    if (!XChangeProperty(
        linux_state->display,
        linux_state->window,
        XInternAtom(linux_state->display, "_NET_WM_NAME", False),
        XInternAtom(linux_state->display, "UTF8_STRING", False),
        8, 
        PropModeReplace, 
        (unsigned char *) app_name,
        strlen(app_name)
    )) {
        LFATAL("Failed to change X11 window name");
        return false;
    }

    if (XFlush(linux_state->display) <= 0) {
        LFATAL("Failed to flush X11");
        return false;
    }

    return true;
}

void platform_shutdown(struct platform_state *state)
{
    LINFO("Shutting down the Linux platform state");

    struct linux_platform_state *linux_state = state->inner_state;
    
    XDestroyWindow(linux_state->display, linux_state->window);
    XCloseDisplay(linux_state->display);

    free(state->inner_state);
}


b8 platform_poll_events(struct platform_state *state)
{ 
    struct linux_platform_state *linux_state = state->inner_state;

    b8 stay_open = true;
    XEvent event;

    while (XPending(linux_state->display)) {
        XNextEvent(linux_state->display, &event);
        switch (event.type) {
        case ClientMessage:
            if (event.xclient.data.l[0] == (long)linux_state->wm_delete) {
                stay_open = false;
            }
            break;
        default:
            break;
        }
    }

    return stay_open;
}

#endif
