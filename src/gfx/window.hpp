#ifndef GAME_GFX_WINDOW_HPP
#define GAME_GFX_WINDOW_HPP

#include <SDL.h>
#include "renderer.hpp"
#include "render_backend.hpp"
#include "../math/vec2.hpp"
#include "../platform.hpp"

#if defined(GAME_PLATFORM_LINUX)
#define SDL_VIDEO_DRIVER_X11
#elif defined(GAME_PLATFORM_WINDOWS)
#define SDL_VIDEO_DRIVER_WINDOWS
#elif defined(GAME_PLATFORM_OSX)
#define SDL_VIDEO_DRIVER_COCOA
#endif
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
    void setRenderer(Renderer* renderer);
    PlatformDisplayData getPlatformData() const;
    void endFrame() const;
    
private:
    friend int eventHandler(void *user_data, SDL_Event *event);
    
    Renderer* renderer = nullptr;
    SDL_Window* handle = nullptr;
    Vec2 size = Vec2(0.f, 0.f);

#if defined(GAME_RENDER_BACKEND_OPENGL)
    SDL_GLContext gl_context;
#endif
};

} // namespace Engine

#endif
