#include <pch.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include "engine/sprite.hpp"
#include "engine/lua.hpp"
#include "engine/keycodes.hpp"
#include "gfx/window.hpp"
#include "gfx/renderer.hpp"
#include "resource/resource_manager.hpp"
#include "resource/lua_source.hpp"
#include "resource/shader.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4.hpp"
#include "math/floats.hpp"

#include <cassert>

void renderLoop(
    Engine::Lua& lua,
    Engine::Window& window,
    Engine::Renderer& renderer,
    Engine::ResourceManager& resource_manager,
    Engine::SpriteManager& sprite_manager
)
{
    bool loop = true;
    while (loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                loop = false;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_UP: lua.fireBuiltinEvent("OnKeyPressed", Engine::KeyCode::Up); break;
                case SDLK_DOWN: lua.fireBuiltinEvent("OnKeyPressed", Engine::KeyCode::Down); break;
                case SDLK_LEFT: lua.fireBuiltinEvent("OnKeyPressed", Engine::KeyCode::Left); break;
                case SDLK_RIGHT: lua.fireBuiltinEvent("OnKeyPressed", Engine::KeyCode::Right); break;
                default: break;
                }
            }
        }

        lua.runOnFrame();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        /*
        if (ImGui::Begin("Editor")) {
            ImGui::End();
        }
        */  

        renderer.clearBackground();
        sprite_manager.render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.swapBuffers();

        lua.gc();
    }
}

int main()
{ 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    {
        Engine::Window window;
        Engine::Renderer renderer;
        renderer.setViewport(
            static_cast<size_t>(window.getSize().getX()), 
            static_cast<size_t>(window.getSize().getY())
        );
        window.setRenderer(&renderer);

        Engine::ResourceManager resource_manager;
        const auto& shader = resource_manager.load<Engine::Shader>("test.shader");
        const auto& script = resource_manager.load<Engine::LuaSource>("test.lua");

        const Engine::Vec2 window_size = window.getSize();
        const Engine::Vec3 camera_pos = Engine::Vec3(0.f, 0.f, 3.f);
        const Engine::Vec3 camera_front = Engine::Vec3(0.f, 0.f, -1.f);
        const Engine::Vec3 camera_up = Engine::Vec3(0.f, 1.f, 0.f);
        const Engine::Mat4 view = Engine::Mat4::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
        );
        const Engine::Mat4 projection = Engine::Mat4::projection(
            Engine::asRadians(45.f), 
            window_size.getX() / window_size.getY(), 
            0.1f, 
            100.f,
            false
        );
        //shader.setUniform("view", view);
        //shader.setUniform("projection", projection);
        
        Engine::SpriteManager sprite_manager(shader);

        Engine::Lua lua(sprite_manager);
        lua.registerTypes<
            Engine::Vec2,
            Engine::Vec3,
            Engine::Sprite,
            Engine::Event,
            Engine::EventConnection
        >();
        lua.pushSource(script);

        lua.runOnStart();

        renderLoop(
            lua,
            window,
            renderer,
            resource_manager,
            sprite_manager
        );
    }

    ImGui::DestroyContext();
    
    return 0;
}
