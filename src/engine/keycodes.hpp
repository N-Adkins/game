#pragma once

#include <SDL_keycode.h>

namespace Engine {

enum class KeyCode : SDL_Keycode {
    Up = SDLK_UP,
    Down = SDLK_DOWN,
    Left = SDLK_LEFT,
    Right = SDLK_RIGHT,
};

} // namespace KeyCodes
