#include <pch.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "gfx/window.hpp"
#include "gfx/renderer.hpp"
#include "resource/resource_manager.hpp"
#include "resource/shader.hpp"
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
    Engine::Renderer renderer;
    renderer.setViewport(
        static_cast<size_t>(window.getSize().getX()), 
        static_cast<size_t>(window.getSize().getY())
    );
    window.setRenderer(&renderer);

    Engine::ResourceManager resource_manager;
    resource_manager.load<Engine::Shader>("test.shader");
    const Engine::Shader& shader = resource_manager.get<Engine::Shader>("test.shader");
    
    bool loop = true;
    while (loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                loop = false;
            }
        }
        
        /*
        const Engine::Vec2 window_size = window.getSize();
        const Engine::Vec3 at(0.f, 0.f, 0.f);
        const Engine::Vec3 eye(0.f, 0.f, -5.f);
        const Engine::Mat4 view = Engine::Mat4::lookAt(eye, at);
        const Engine::Mat4 projection = Engine::Mat4::projection(60.f, window_size.getX() / window_size.getY(), 0.1f, 100.f, 0);
        */ 

        renderer.startFrame();
        {

        }
        renderer.endFrame();
        window.endFrame();
    }
    
    return 0;
}
