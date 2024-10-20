#pragma once

#include <platform/platform.h>
#include <core/memory.h>
#include <defines.h>

struct application_config {
	const char *app_name;
	i32 start_x;
	i32 start_y;
	i32 start_width;
	i32 start_height;
};

struct application {
	struct allocator allocator;
	struct game *game_state;
	struct platform platform_state;
	f32 last_time; // for deltatime
	i32 width;
	i32 height;
	b8 is_running;
	b8 is_paused;
};

LAPI void application_create(struct application *app, struct game *game_state);
LAPI void application_destroy(struct application *app);
LAPI void application_run(struct application *app);
