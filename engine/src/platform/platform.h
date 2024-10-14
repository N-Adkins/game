#pragma once

/**
 * @file
 * @brief Platform layer
 *
 * Handles platform-specifics through a type-erased layer,
 * this includes windowing and event handling.
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
 * @param file Output file, can be an actual opened file
 * or stderr / stdout.
 */
void platform_print_color(
    FILE *file,
    const char *string,
    enum terminal_color color
);
