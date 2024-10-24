#include "core/memory.h"
#include <entry.h>
#include <core/logger.h>

struct game_state {

};

void game_init(struct game *game)
{
    LINFO("Game initializing");
    game->state = engine_alloc(sizeof(struct game_state), MEMORY_TAG_UNKNOWN);
}

void game_deinit(struct game *game)
{
	LINFO("Game deinitializing");
	engine_free(game->state, sizeof(struct game_state), MEMORY_TAG_UNKNOWN);
}

void game_fixed_step(struct game *game, f32 delta_time)
{
	(void)game;
	(void)delta_time;
}

void game_render_step(struct game *game, f32 delta_time)
{
	(void)game;
	(void)delta_time;
}

void game_window_resized(struct game *game, i32 width, i32 height)
{
	(void)game;
	LINFO("Window resized with size (%d, %d)", width, height);
}

void game_key_pressed(struct game *game, enum keycode keycode)
{
	(void)game;
	(void)keycode;
	LINFO("Pressed key");
}

void game_key_released(struct game *game, enum keycode keycode)
{
	(void)game;
	(void)keycode;
	LINFO("Released key");
}

void configure_game(struct game *game)
{
	game->config = (struct application_config){
		.app_name = "Editor",
		.start_x = 0,
		.start_y = 0,
		.start_width = 1280,
		.start_height = 720,
	};
	game->vtable.init = &game_init;
	game->vtable.deinit = &game_deinit;
	game->vtable.fixed_step = &game_fixed_step;
	game->vtable.render_step = &game_render_step;
	game->vtable.window_resized = &game_window_resized;
	game->vtable.key_pressed = &game_key_pressed;
	game->vtable.key_released = &game_key_released;
}
