#pragma once

/**
 * @file
 * @brief Entrypoint
 *
 * Implements main so that the application just has to include it in order
 * to have the layers start.
 */

#include <core/application.h>
#include <core/game.h>
#include <defines.h>

extern void configure_game(struct game *game);

int main(void)
{
	struct game game;
	configure_game(&game);

	struct application app;
	application_create(&app, &game);
	application_run(&app);
	application_destroy(&app);

	return 0;
}
