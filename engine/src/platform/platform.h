#pragma once

/**
 * @file
 * @brief Platform layer
 *
 * Handles platform-specifics through a type-erased layer,
 * this includes windowing and event handling. Nothing in this
 * program is symbol-exported (for dynamic linking) because this
 * layer should be completely abstracted away by the application
 * layer.
 */

#include <defines.h>
#include <stdio.h>

// TODO: other platforms lol
#define LPLATFORM_LINUX

/**
 * Colors to be used in platform_print_color
 */
enum terminal_color {
    TERMINAL_COLOR_PURPLE,
    TERMINAL_COLOR_RED,
    TERMINAL_COLOR_YELLOW,
    TERMINAL_COLOR_GREEN,
    TERMINAL_COLOR_GRAY,
};

/**
 * Holds a type-erased handle to the underlying
 * platform's state struct
 */
struct platform_state {
    void *inner_state;
};

/**
 * @brief Starts the platform layer
 *
 * Should be called more or less first in the application,
 * and only once. Any errors that take place here are 
 * considered unrecoverable. Opens the window and generates 
 * the platform layer.
 *
 * @param app_name The name on the top of the window
 */
void platform_startup(
    struct platform_state *state,
    const char *app_name,
    i32 start_x,
    i32 start_y,
    i32 start_width,
    i32 start_height
);

/**
 * @brief Shuts down platform-specifics
 *
 * Closes the window and fully destructs the platform layer. 
 * No platform functions can be called with the passed state 
 * after this is called.
 */
void platform_shutdown(struct platform_state *state);

/**
 * @brief Processes OS and window events
 *
 * Goes through all of the events buffered since the last call
 * and calls their respective handlers.
 *
 * @return true if the window should stay open, false if it 
 * should close
 */
b8 platform_poll_events(struct platform_state *state);

/**
 * @brief Prints a string to the passed file with a color
 *
 * Uses platform-specific escape codes to print colored text
 * to a file. This is mostly intended for use with logging,
 * avoid using this in most contexts.
 *
 * One important note is that this function may **NOT** call
 * any logging functions or assertions because an error could
 * result in an infinite loop.
 *
 * @param file Output file, can be an actual opened file
 * or stderr / stdout.
 */
void platform_print_color(
    FILE *file,
    const char *string,
    enum terminal_color color
);

/**
 * @brief Calls a breakpoint
 *
 * Attempts to use a platform-specific instruction or syscall to
 * break for a debugger. Otherwise, aborts.
 */
void platform_debug_break(void);

/*
The main justification for these memory functions is the case of a console / strange platform.
On platforms like MacOS, Windows, or Linux, these are going to be the expected libc functions.

These really should only be used by the memory subsystem under the hood. If used otherwise, document
extensively why.
*/

/**
 * @brief Platform-agnostic equivalent to malloc
 */
void *platform_allocate(u64 size, b8 aligned);

/**
 * @brief Platform-agnostic equivalent to free
 */
void platform_free(void *ptr, b8 aligned);

/**
 * @brief Platform-agnostic equivalent to memset(ptr, 0, size)
 *
 * The justfication for this function is that on some platforms there might
 * be significantly more efficient ways to zero memory than set it to
 * other values.
 */
void *platform_zero_memory(void *ptr, u64 size);

/**
 * @brief Platform-agnostic equivalent to memcpy
 *
 * The parameters are "restrict" because the memory cannot overlap between the two
 * pointers.
 */
void *platform_copy_memory(void *restrict dest, const void *restrict source, u64 size);

/**
 * @brief Platform-agnostic equivalent to memset
 */
void *platform_set_memory(void *dest, i32 value, u64 size);
