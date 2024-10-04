#pragma once

#include <SDL.h>
#include "render_backend.hpp"
#include "../math/vec2.hpp"
#include "../constructors.hpp"

namespace Engine {

class Renderer;

struct PlatformDisplayData {
    void* display;
    void* window;
};

class Window {
public:
    Window();
    ~Window();
    DELETE_COPY(Window);
    DEFAULT_MOVE(Window);

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
