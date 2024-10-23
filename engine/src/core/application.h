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
	const char *app_name;
	i32 start_x;
	i32 start_y;
	i32 start_width;
	i32 start_height;
};

/**
 * @brief App state
 *
 * Holds things that are needed between frames, as well as some general
 * state of the app. Also holds a non-owning pointer to the game state so
 * that game function pointers can be called
 */
struct application {
	struct event_system event_system;
	struct platform platform_state;
	struct game *game_state;
	f32 last_time; // for deltatime
	i32 width;
	i32 height;
	b8 is_running;
	b8 is_paused;
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
