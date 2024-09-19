#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "gfx/bgfx.hpp"
#include "gfx/sdl.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4.hpp"

#include <cassert>

using namespace Engine;

int main()
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string);
    Vec2::registerLua(lua);
    Vec3::registerLua(lua);
    
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    SDL_Window *window = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        1280, 
        720, 
        SDL_WINDOW_SHOWN
    );
    assert(window != nullptr);
        
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    assert(SDL_GetWindowWMInfo(window, &info) != 0);

    bgfx::PlatformData platform_data;
    platform_data.ndt = info.info.x11.display;
    platform_data.nwh = reinterpret_cast<void*>(static_cast<uintptr_t>(info.info.x11.window));
    platform_data.context = nullptr;
    platform_data.backBuffer = nullptr;
    platform_data.backBufferDS = nullptr;

    bgfx::Init init_data;
    init_data.type = bgfx::RendererType::Vulkan;
    init_data.resolution.width = 1280;
    init_data.resolution.height = 720;
    init_data.resolution.reset = BGFX_RESET_VSYNC;
    init_data.debug = BGFX_DEBUG_TEXT;
    init_data.platformData = platform_data;

    assert(bgfx::init(init_data));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xAAAAFFFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 1280, 720);    
    
    bool loop = true;
    while (loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                loop = false;
            }
        }

        const Vec3 at(0.f, 0.f, 0.f);
        const Vec3 eye(0.f, 0.f, -5.f);
        Mat4 view = Mat4::lookAt(eye, at);
        Mat4 projection = Mat4::projection(60.f, 1280.f / 720.f, 0.1f, 100.f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view.getValues(), projection.getValues());

        bgfx::frame();
    }
    
    bgfx::shutdown();

    SDL_Quit();

    return 0;
}
