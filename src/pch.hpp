#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "platform.hpp"
#if defined(GAME_PLATFORM_LINUX)
#define SDL_VIDEO_DRIVER_X11
#elif defined(GAME_PLATFORM_WINDOWS)
#define SDL_VIDEO_DRIVER_WINDOWS
#elif defined(GAME_PLATFORM_OSX)
#define SDL_VIDEO_DRIVER_COCOA
#endif

#include "gfx/render_backend.hpp"
#if defined(GAME_RENDER_BACKEND_OPENGL)
#include <glad/glad.h>
#include <SDL_opengl.h>
#endif

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#include <sol/sol.hpp>

#include "logging.hpp"
