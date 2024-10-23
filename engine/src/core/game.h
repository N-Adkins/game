#pragma once

/**
 * @file
 * @brief Game layer
 *
 * This is the layer that is exposed to the actual game executable / library -
 * they cannot see the rest of the application state, only some functions that are
 * also exposed. The game provides function pointers here that are run at certain
 * times during execution.
 */

#include <core/application.h>
#include <defines.h>

/**
 * @brief Game state
 *
 * This holds *all* state that is to be used within the game. There is an opaque
 * handle provided for the game to hold an arbitrary state struct. The function pointers
 * are called at self-explanatory points of execution.
 *
 * All function pointers may or may not be implemented - they are optional.
 */
struct game {
	struct application_config config;
	struct {
		void (*init)(struct game *game);
		void (*deinit)(struct game *game);
		void (*fixed_step)(struct game *game, f32 delta_time);
		void (*render_step)(struct game *game, f32 delta_time);
		void (*window_resized)(struct game *game, i32 width,
				       i32 height);
	} vtable;
	void *state; /**< Arbitrary state pointer */
};
