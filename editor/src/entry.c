#include <entry.h>
#include <core/logger.h>

struct {
} state;

void game_init(struct game *game)
{
	LINFO("Game initialized");
	(void)game;
}

void game_deinit(struct game *game)
{
	LINFO("Game deinitialized");
	(void)game;
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

void configure_game(struct game *game)
{
	game->config = (struct application_config){
		.app_name = "Editor",
		.start_x = 0,
		.start_y = 0,
		.start_width = 1280,
		.start_height = 720,
	};
	game->state = &state;
	game->init_func = &game_init;
	game->deinit_func = &game_deinit;
	game->fixed_step_func = &game_fixed_step;
	game->render_step_func = &game_render_step;
}
