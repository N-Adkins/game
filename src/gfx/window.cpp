#include "window.hpp"
#include "logging.hpp"
#include <string_view>

#if defined (GAME_RENDER_BACKEND_OPENGL)
#include <SDL_opengl.h>
#endif
#include <SDL_syswm.h>
#include <SDL_video.h>

constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;

namespace Engine {

int eventHandler(void *user_data, SDL_Event *event);

Window::Window()
{
    Log::debug("Attempting to create SDL window");
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);

#if defined (GAME_RENDER_BACKEND_OPENGL)
    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#endif

    handle = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        DEFAULT_WIDTH, 
        DEFAULT_HEIGHT, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
#if defined (GAME_RENDER_BACKEND_OPENGL)
        | SDL_WINDOW_OPENGL
#endif
    );

#if defined (GAME_RENDER_BACKEND_OPENGL)
    gl_context = SDL_GL_CreateContext(handle);
    assert(gl_context != nullptr);
    assert(gladLoadGLLoader(SDL_GL_GetProcAddress));
    Log::info("Loaded OpenGL");
    Log::info("Vendor: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_VENDOR))});
    Log::info("Renderer: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_RENDERER))});
    Log::info("Version: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_VERSION))});
#endif

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

Renderer Window::createRenderer() const
{
    return Renderer();
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
