#include "bgfx/bgfx.h"
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "gfx/bgfx.hpp"
#include "gfx/window.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4.hpp"

#include <cassert>

int main()
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string);
    Engine::Vec2::registerLua(lua);
    Engine::Vec3::registerLua(lua);

    Engine::Window window;
    auto window_platform = window.getPlatformData();
    
    bgfx::PlatformData platform_data;
    platform_data.ndt = window_platform.display;
    platform_data.nwh = window_platform.window;
    platform_data.context = nullptr;
    platform_data.backBuffer = nullptr;
    platform_data.backBufferDS = nullptr;

    bgfx::Init init_data;
    init_data.type = bgfx::RendererType::Vulkan;
    init_data.resolution.width = static_cast<uint32_t>(window.getSize().getX());
    init_data.resolution.height = static_cast<uint32_t>(window.getSize().getY());
    init_data.resolution.reset = BGFX_RESET_VSYNC;
    init_data.debug = BGFX_DEBUG_TEXT;
    init_data.platformData = platform_data;

    assert(bgfx::init(init_data));

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xAAAAFFFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 
        static_cast<uint16_t>(window.getSize().getX()),
        static_cast<uint16_t>(window.getSize().getY())
    );
    
    bool loop = true;
    while (loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                loop = false;
            }
        }

        Engine::Vec2 window_size = window.getSize();

        const Engine::Vec3 at(0.f, 0.f, 0.f);
        const Engine::Vec3 eye(0.f, 0.f, -5.f);
        Engine::Mat4 view = Engine::Mat4::lookAt(eye, at);
        Engine::Mat4 projection = Engine::Mat4::projection(60.f, window_size.getX() / window_size.getY(), 0.1f, 100.f, bgfx::getCaps()->homogeneousDepth);
        bgfx::setViewTransform(0, view.getValues(), projection.getValues());
        
        bgfx::dbgTextClear();
        bgfx::setDebug(BGFX_DEBUG_STATS);
        bgfx::frame();
    }
    
    bgfx::shutdown();

    SDL_Quit();

    return 0;
}
