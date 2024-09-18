#ifndef GAME_GFX_SDL_HPP
#define GAME_GFX_SDL_HPP

#include <platform.hpp>

#if defined(GAME_PLATFORM_LINUX)
#define SDL_VIDEO_DRIVER_X11
#elif defined(GAME_PLATFORM_WINDOWS)
#define SDL_VIDEO_DRIVER_WINDOWS
#elif defined(GAME_PLATFORM_OSX)
#define SDL_VIDEO_DRIVER_COCOA
#endif
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#endif
