#include "application.h"

#include "core/event.h"
#include <core/assert.h>
#include <core/game.h>

static void application_window_resized(union event_payload payload,
				       void *user_data)
{
	LASSERT(user_data != NULL);
	struct application *app = user_data;

	if (app->game_state->vtable.window_resized != NULL) {
		app->game_state->vtable.window_resized(
			app->game_state, payload.window_resized.width,
			payload.window_resized.height);
	}

	event_system_unregister(&app->event_system, EVENT_TAG_WINDOW_RESIZED,
				application_window_resized);
}

LAPI void application_create(struct application *app, struct game *game_state)
{
	app->is_running = true;
	app->is_paused = false;
	app->game_state = game_state;
	app->width = game_state->config.start_width;
	app->height = game_state->config.start_height;
	app->last_time = (f32)0;

	platform_startup(&app->platform_state, &app->event_system,
			 game_state->config.app_name,
			 game_state->config.start_x, game_state->config.start_y,
			 game_state->config.start_width,
			 game_state->config.start_height);

	app->allocator = allocator_create();
	game_state->allocator = &app->allocator;

	event_system_startup(&app->event_system, &app->allocator);
	event_system_register(&app->event_system, EVENT_TAG_WINDOW_RESIZED,
			      application_window_resized, app);

	if (game_state->vtable.init != NULL) {
		game_state->vtable.init(game_state);
	}
}

LAPI void application_destroy(struct application *app)
{
	if (app->game_state->vtable.deinit != NULL) {
		app->game_state->vtable.deinit(app->game_state);
	}
	event_system_shutdown(&app->event_system);
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

		if (app->game_state->vtable.render_step != NULL) {
			// TODO: Deltatime
			app->game_state->vtable.render_step(app->game_state,
							    (f32)0);
		}
	}
}
