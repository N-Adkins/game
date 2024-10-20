#include "application.h"

#include <core/game.h>

LAPI void application_create(struct application *app, struct game *game_state)
{
	app->is_running = true;
	app->is_paused = false;
	app->game_state = game_state;
	app->width = game_state->config.start_width;
	app->height = game_state->config.start_height;
	app->last_time = (f32)0;

	platform_startup(&app->platform_state, game_state->config.app_name,
			 game_state->config.start_x, game_state->config.start_y,
			 game_state->config.start_width,
			 game_state->config.start_height);

	app->allocator = allocator_create();
	game_state->allocator = &app->allocator;

	if (game_state->init_func != NULL) {
		game_state->init_func(game_state);
	}
}

LAPI void application_destroy(struct application *app)
{
	if (app->game_state->deinit_func != NULL) {
		app->game_state->deinit_func(app->game_state);
	}
	allocator_destroy(&app->allocator);
	platform_shutdown(app->platform_state);
}

LAPI void application_run(struct application *app)
{
	while (app->is_running) {
		if (app->is_paused) {
			// TODO: Platform specific thread wait to stop
			// 100% usage here
			continue;
		}
		app->is_running = platform_poll_events(app->platform_state);

		if (app->game_state->render_step_func != NULL) {
			// TODO: Deltatime
			app->game_state->render_step_func(app->game_state,
							  (f32)0);
		}
	}
}
