#include "application.h"

#include "core/event.h"
#include "core/memory.h"
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
}

static void application_key_pressed(union event_payload payload, void *user_data)
{
    LASSERT(user_data != NULL);
    struct application *app = user_data;

    app->input_state.keyboard[payload.key_pressed.key] = true;
}

static void application_key_released(union event_payload payload, void *user_data)
{
    LASSERT(user_data != NULL);
    struct application *app = user_data;

    app->input_state.keyboard[payload.key_released.key] = false;
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

	memory_system_startup();

    input_state_create(&app->input_state);

	event_system_startup(&app->event_system);
	event_system_register(&app->event_system, EVENT_TAG_WINDOW_RESIZED,
			      application_window_resized, app);
    event_system_register(&app->event_system, EVENT_TAG_KEY_PRESSED,
                  application_key_pressed, app);
    event_system_register(&app->event_system, EVENT_TAG_KEY_RELEASED,
                  application_key_released, app);

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
	memory_system_shutdown();
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

		const f32 current_time = (f32)platform_time_ms() / 1000;
		const f32 last_time = app->last_time;
		app->last_time = current_time;
		const f32 delta_time = current_time - last_time;
		if (app->game_state->vtable.render_step != NULL) {
			// TODO: Deltatime
			app->game_state->vtable.render_step(app->game_state,
							    delta_time);
		}
	}
}
