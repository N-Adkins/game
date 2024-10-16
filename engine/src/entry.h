#pragma once

/**
 * @file
 * @brief Entrypoint
 *
 * Implements main so that the application just has to include it in order
 * to have the layers start.
 */

#include <defines.h>

// TODO: Completely remove this and make application layer lol
// This is pretty much just here for making sure windows builds
// work fine

LAPI extern int real_main(void);

int main(void)
{
    return real_main();
}
