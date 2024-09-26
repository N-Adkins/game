#ifndef GAME_GFX_WINDOW_HPP
#define GAME_GFX_WINDOW_HPP

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

#include "../math/vec2.hpp"

namespace Engine {

struct PlatformDisplayData {
    void* display;
    void* window;
};

class Window {
public:
    Window();
    ~Window();

    Vec2 getSize() const;
    PlatformDisplayData getPlatformData() const;
    
private:
    friend int eventHandler(void *user_data, SDL_Event *event);

    SDL_Window* handle = nullptr;
    Vec2 size = Vec2(0.f, 0.f);

};

} // namespace Engine

#endif
