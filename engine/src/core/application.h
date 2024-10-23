#pragma once

/**
 * @file
 * @brief Application layer
 *
 * Manages the game state provided by the linked executable and holds most
 * of the "global" state that is used in the app. Think of this as the layer
 * above the platform layer.
 */

#include <platform/platform.h>
#include <core/event.h>
#include <core/memory.h>
#include <defines.h>

/**
 * @brief App settings
 *
 * These are pretty much all "starting" values, things that will be applied
 * when the application layer starts.
 */
struct application_config {
	const char *app_name; /**< The text at the top of the window */
	i32 start_x;	      /**< Window starting X position */
	i32 start_y;	      /**< Window starting Y position */
	i32 start_width;      /**< Window starting width */
	i32 start_height;     /**< Windodw starting height */
};

/**
 * @brief App state
 *
 * Holds things that are needed between frames, as well as some general
 * state of the app. Also holds a non-owning pointer to the game state so
 * that game function pointers can be called
 */
struct application {
	struct event_system event_system; /**< Manages events */
	struct platform platform_state; /**< Owning reference to platform state */
	struct game *game_state; /**< Non-owning reference to game state */
	f32 last_time; /**< Used for delta_time, is the last frame's time in ms */
	i32 width;     /**< Current window width */
	i32 height;    /**< Current window height */
	b8 is_running; /**< True if the app is running, false if not */
	b8 is_paused;  /**< True if the app is suspended, false if not */
};

/**
 * @brief Application initializer
 *
 * Begins the application, runs game init function.
 */
LAPI void application_create(struct application *app, struct game *game_state);

/**
 * @brief Application deinitializer
 *
 * Stops the application, runs game deinit function.
 */
LAPI void application_destroy(struct application *app);

/**
 * @brief Application loop
 *
 * Runs the main app event and rendering loop.
 */
LAPI void application_run(struct application *app);
