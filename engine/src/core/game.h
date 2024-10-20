#pragma once

#include <core/application.h>
#include <defines.h>

struct game;

typedef void (*pfn_game_init)(struct game *game);
typedef void (*pfn_game_deinit)(struct game *game);
typedef void (*pfn_game_fixed_step)(struct game *game,
				    f32 delta_time); // Fixed update, 60 hz
typedef void (*pfn_game_render_step)(
	struct game *game, f32 delta_time); // Dynamic update, runs each frame

struct game {
	struct allocator *allocator;
	struct application_config config;
	pfn_game_init init_func;
	pfn_game_deinit deinit_func;
	pfn_game_fixed_step fixed_step_func;
	pfn_game_render_step render_step_func;
	void *state; // Internal game state
};
