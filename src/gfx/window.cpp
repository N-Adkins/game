#include "window.hpp"

constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;

namespace Engine {

Window::Window()
{
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    handle = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        DEFAULT_WIDTH, 
        DEFAULT_HEIGHT, 
        SDL_WINDOW_SHOWN
    );
    size.setX(DEFAULT_WIDTH);
    size.setY(DEFAULT_HEIGHT);
    assert(handle != nullptr);
}

Window::~Window()
{
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

} // namespace Engine
