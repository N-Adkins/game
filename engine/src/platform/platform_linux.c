#include "platform.h"

/**
 * @file
 * @brief Linux platform layer
 *
 * This layer uses X11 for windowing (for now). The only other thing of note
 * is that the allocation functions are simply glibc wrappers.
 */

#ifdef LPLATFORM_LINUX

#include <core/assert.h>
#include <core/event.h>
#include <core/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <pthread.h>
#include <signal.h>

/**
 * Linux-specific platform state, contains X11 handles.
 */
struct platform_impl {
	struct event_system *event_system;
	Display *display; // X11 display handle
	Window window; // X11 window handle
	Atom wm_delete; // Handle to the window deletion event
};

/**
 * Linux-specific mutex, in this case a pthread mutex
 */
struct mutex_impl {
	pthread_mutex_t mutex;
};

void platform_startup(struct platform *platform,
		      struct event_system *event_system, const char *app_name,
		      i32 start_x, i32 start_y, i32 start_width,
		      i32 start_height)
{
	LASSERT(platform != NULL);
	LASSERT(app_name != NULL);

	LINFO("Initializing the Linux platform state");

	struct platform_impl *linux_impl = malloc(sizeof(struct platform_impl));
	platform->impl = linux_impl;

	linux_impl->event_system = event_system;

	linux_impl->display = XOpenDisplay(NULL); // fetch default display
	if (linux_impl->display == NULL) {
		LFATAL("Failed to open X11 display");
		return;
	}

	Window root_window = XDefaultRootWindow(linux_impl->display);
	if (root_window <= 0) {
		LFATAL("Failed to fetch default X11 window");
		return;
	}

	linux_impl->window = XCreateSimpleWindow(
		linux_impl->display, root_window, start_x, start_y,
		(u32)start_width, (u32)start_height,
		0, // no border
		BlackPixel(linux_impl->display, 0),
		BlackPixel(linux_impl->display, 0));
	if (linux_impl->window <= 0) {
		LFATAL("Failed to create X11 window");
		return;
	}

	if (!XMapWindow(linux_impl->display, linux_impl->window)) {
		LFATAL("Failed to map X11 window to display");
		return;
	}

	const long event_mask = KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask |
				PointerMotionMask | ResizeRedirectMask;

	if (!XSelectInput(linux_impl->display, linux_impl->window,
			  event_mask)) {
		LFATAL("Failed to enable X11 event mask");
		return;
	}

	linux_impl->wm_delete =
		XInternAtom(linux_impl->display, "WM_DELETE_WINDOW", false);

	if (!XSetWMProtocols(linux_impl->display, linux_impl->window,
			     &linux_impl->wm_delete, 1)) {
		LFATAL("Failed to set X11 WM protocols");
		return;
	}

	// Set window name to app_name
	const i32 BYTE_SIZE = 8;
	if (!XChangeProperty(
		    linux_impl->display, linux_impl->window,
		    XInternAtom(linux_impl->display, "_NET_WM_NAME", False),
		    XInternAtom(linux_impl->display, "UTF8_STRING", False),
		    BYTE_SIZE, PropModeReplace, (unsigned char *)app_name,
		    (i32)strlen(app_name))) {
		LFATAL("Failed to change X11 window name");
		return;
	}

	if (XFlush(linux_impl->display) <= 0) {
		LFATAL("Failed to flush X11");
		return;
	}
}

void platform_shutdown(struct platform platform)
{
	LASSERT(platform.impl != NULL);

	LINFO("Shutting down the Linux platform state");

	XDestroyWindow(platform.impl->display, platform.impl->window);
	XCloseDisplay(platform.impl->display);

	free(platform.impl);
}

b8 platform_poll_events(struct platform platform)
{
	LASSERT(platform.impl != NULL);

	b8 stay_open = true;
	XEvent event;
	union event_payload payload;

	while (XPending(platform.impl->display)) {
		XNextEvent(platform.impl->display, &event);
		switch (event.type) {
		case ResizeRequest:
			payload.window_resized.width =
				event.xresizerequest.width;
			payload.window_resized.height =
				event.xresizerequest.height;
			event_system_fire(platform.impl->event_system,
					  EVENT_TAG_WINDOW_RESIZED, payload);
			break;
		case ClientMessage:
			if (event.xclient.data.l[0] ==
			    (long)platform.impl->wm_delete) {
				stay_open = false;
			}
			break;
		default:
			break;
		}
	}

	return stay_open;
}

void platform_print_color(FILE *file, const char *string,
			  enum terminal_color color)
{
	LASSERT(file != NULL);
	LASSERT(string != NULL);

	const char *reset_code = "\e[0;0m";
	const char *color_code = NULL;

	switch (color) {
	case TERMINAL_COLOR_PURPLE:
		color_code = "\e[1;35m";
		break;
	case TERMINAL_COLOR_RED:
		color_code = "\e[1;31m";
		break;
	case TERMINAL_COLOR_YELLOW:
		color_code = "\e[1;33m";
		break;
	case TERMINAL_COLOR_GREEN:
		color_code = "\e[0;32m";
		break;
	case TERMINAL_COLOR_GRAY:
		color_code = "\e[0;37m";
		break;
	}

	(void)fputs(color_code, file);
	(void)fputs(string, file);
	(void)fputs(reset_code, file);
}

LAPI void platform_debug_break(void)
{
	(void)raise(SIGTRAP);
}

// On linux these are all just going to basically be glibc wrappers.

void *platform_allocate(u64 size, b8 aligned)
{
	(void)aligned; // irrelevant, always aligned
	return malloc(size);
}

void platform_free(void *ptr, b8 aligned)
{
	(void)aligned; // irrelevant, this memory is malloced so always aligned
	free(ptr);
}

void *platform_zero_memory(void *ptr, u64 size)
{
	LASSERT(ptr != NULL);

	return memset(ptr, 0, size);
}

void *platform_copy_memory(void *restrict dest, const void *restrict source,
			   u64 size)
{
	LASSERT(dest != NULL);
	LASSERT(source != NULL);

	return memcpy(dest, source, size);
}

void *platform_move_memory(void *dest, const void *source, u64 size)
{
	LASSERT(dest != NULL);
	LASSERT(source != NULL);

	return memmove(dest, source, size);
}

void *platform_set_memory(void *dest, u8 value, u64 size)
{
	LASSERT(dest != NULL);

	return memset(dest, value, size);
}

struct mutex mutex_create(void)
{
	int err = 0;

	struct mutex_impl *linux_impl =
		platform_allocate(sizeof(struct mutex_impl), true);
	struct mutex mutex;
	mutex.impl = linux_impl;

	// The main attribute we set is ERRORCHECK which we can have enabled in debug modes
	// to give us information about deadlocks and such.
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_t *attr_ptr = &mutex_attr;
	err = pthread_mutexattr_init(&mutex_attr);
	if (!err) {
		// TODO: Remove errorcheck for release modes, likely
		err = pthread_mutexattr_settype(&mutex_attr,
						PTHREAD_MUTEX_ERRORCHECK);
		if (err) {
			LERROR("Failed to set pthread mutex type in attributes: %s",
			       strerror(err));
		}
	} else {
		LERROR("Failed to initialize pthread mutex attributes, falling back to default: %s",
		       strerror(err));
		attr_ptr = NULL;
	}

	err = pthread_mutex_init(&linux_impl->mutex, attr_ptr);
	if (err) {
		LERROR("Failed to initialize pthread mutex for Linux: %s",
		       strerror(err));
	}

	if (attr_ptr != NULL) {
		err = pthread_mutexattr_destroy(&mutex_attr);
		if (err) {
			LERROR("Failed to destroy pthread mutex attributes: %s",
			       strerror(err));
		}
	}

	return mutex;
}

void mutex_destroy(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	int err = 0;

	err = pthread_mutex_destroy(&mutex.impl->mutex);
	if (err) {
		LERROR("Failed to destroy pthread mutex for Linux: %s",
		       strerror(err));
	}

	platform_free(mutex.impl, true);
}

void mutex_lock(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	int err = 0;

	err = pthread_mutex_lock(&mutex.impl->mutex);
	if (err) {
		LERROR("Failed to lock pthread mutex for Linux: %s",
		       strerror(err));
	}
}

void mutex_unlock(struct mutex mutex)
{
	LASSERT(mutex.impl != NULL);

	int err = 0;

	err = pthread_mutex_unlock(&mutex.impl->mutex);
	if (err) {
		LERROR("Failed to unlock pthread mutex for Linux: %s",
		       strerror(err));
	}
}

#endif
