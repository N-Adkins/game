#pragma once

#include "../constructors.hpp"
#include <SDL.h>
#include <glm/fwd.hpp>

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

    glm::vec2 getSize() const;
    void setRenderer(Renderer* renderer);
    PlatformDisplayData getPlatformData() const;
    void swapBuffers() const;
    
private:
    friend int eventHandler(void *user_data, SDL_Event *event);
    
    Renderer* renderer = nullptr;
    SDL_Window* handle = nullptr;
    glm::vec2 size = { 0.f, 0.f };

    SDL_GLContext gl_context;
};

} // namespace Engine
