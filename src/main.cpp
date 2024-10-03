#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

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
        Engine::Mat4 projection = Engine::Mat4::projection(60.f, window_size.getX() / window_size.getY(), 0.1f, 100.f, 0);
    }
    
    SDL_Quit();

    return 0;
}
