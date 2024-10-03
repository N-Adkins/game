#include "window.hpp"
#include "logging.hpp"

constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;

namespace Engine {

int eventHandler(void *user_data, SDL_Event *event);

Window::Window()
{
    Log::debug("Attempting to create SDL window");
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    handle = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        DEFAULT_WIDTH, 
        DEFAULT_HEIGHT, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    assert(handle != nullptr);
    size.setX(DEFAULT_WIDTH);
    size.setY(DEFAULT_HEIGHT);
    SDL_AddEventWatch(eventHandler, this);
}

Window::~Window()
{
    Log::debug("Destroying SDL window");
    SDL_DestroyWindow(handle);
    SDL_Quit();
}

Vec2 Window::getSize() const
{
    return size;
}

PlatformDisplayData Window::getPlatformData() const
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    assert(SDL_GetWindowWMInfo(handle, &info) != 0);

    PlatformDisplayData data;
    
#if defined(GAME_PLATFORM_LINUX)
    data.display = info.info.x11.display;
    data.window = reinterpret_cast<void*>(static_cast<uintptr_t>(info.info.x11.window));
#else
#error Unsupported platform for windowing
#endif

    return data;
}

int eventHandler(void* user_data, SDL_Event* event)
{
    Window* window = static_cast<Window*>(user_data);
    switch (event->type) {
    case SDL_WINDOWEVENT:
        if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            int width, height;
            SDL_GetWindowSize(window->handle, &width, &height);
            window->size.setX(static_cast<float>(width));
            window->size.setY(static_cast<float>(height));
            Log::debug("Window size changed: ({}, {})", width, height);
        }
        break;
    default:
        break;
    }
    return 0;
}


} // namespace Engine
