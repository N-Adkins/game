#include <cstdlib>
#include <pch.hpp>

#include "window.hpp"
#include "renderer.hpp"
#include <backends/imgui_impl_sdl2.h>

constexpr int DEFAULT_WIDTH = 1280;
constexpr int DEFAULT_HEIGHT = 720;

namespace Engine {

int eventHandler(void *user_data, SDL_Event *event);

Window::Window()
{
    Log::debug("Attempting to create SDL window");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        Log::error("Failed to initialize SDL");
        std::exit(EXIT_FAILURE);
    }

    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    handle = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        DEFAULT_WIDTH, 
        DEFAULT_HEIGHT, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );

    if (handle == nullptr) {
        Log::error("Failed to create SDL window");
        std::exit(EXIT_FAILURE);
    }

    gl_context = SDL_GL_CreateContext(handle);
    if (gl_context == nullptr) {
        Log::error("Failed to create OpenGL context for SDL");
        std::exit(EXIT_FAILURE);
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        Log::error("Failed to load GLAD for OpenGL");
        std::exit(EXIT_FAILURE);
    }

    ImGui_ImplSDL2_InitForOpenGL(handle, gl_context);

    Log::info("Loaded OpenGL");
    Log::info("Vendor: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_VENDOR))});
    Log::info("Renderer: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_RENDERER))});
    Log::info("Version: {}", std::string_view{reinterpret_cast<const char *>(glGetString(GL_VERSION))});

    size.setX(DEFAULT_WIDTH);
    size.setY(DEFAULT_HEIGHT);

    SDL_AddEventWatch(eventHandler, this);
}

Window::~Window()
{
    Log::debug("Destroying SDL window");
    ImGui_ImplSDL2_Shutdown();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(handle);
    SDL_Quit();
}

Vec2 Window::getSize() const
{
    return size;
}

void Window::setRenderer(Renderer* renderer)
{
    this->renderer = renderer;
}

PlatformDisplayData Window::getPlatformData() const
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(handle, &info);

    PlatformDisplayData data;
    
#if defined(GAME_PLATFORM_LINUX)
    data.display = info.info.x11.display;
    data.window = reinterpret_cast<void*>(static_cast<uintptr_t>(info.info.x11.window));
#else
#error Unsupported platform for windowing
#endif

    return data;
}

void Window::swapBuffers() const
{
    SDL_GL_SwapWindow(handle);
}

int eventHandler(void* user_data, SDL_Event* event)
{
    Window* window = static_cast<Window*>(user_data);
    assert(window != nullptr);
    assert(window->renderer != nullptr);

    switch (event->type) {
    case SDL_WINDOWEVENT:
        if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            int width, height;
            SDL_GetWindowSize(window->handle, &width, &height);
            window->size.setX(static_cast<float>(width));
            window->size.setY(static_cast<float>(height));
            window->renderer->setViewport(static_cast<size_t>(width), static_cast<size_t>(height));
            Log::debug("Window size changed: ({}, {})", width, height);
        }
        break;
    default:
        break;
    }
    return 0;
}

} // namespace Engine
