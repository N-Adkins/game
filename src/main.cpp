#include "resource/lua_source.hpp"
#include <pch.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include "engine/sprite.hpp"
#include "engine/lua.hpp"
#include "gfx/window.hpp"
#include "gfx/renderer.hpp"
#include "resource/resource_manager.hpp"
#include "resource/shader.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4.hpp"

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
            }
        }

        lua.runOnFrame();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        /*
        const Engine::Vec2 window_size = window.getSize();
        const Engine::Vec3 at(0.f, 0.f, 0.f);
        const Engine::Vec3 eye(0.f, 0.f, -5.f);
        const Engine::Mat4 view = Engine::Mat4::lookAt(eye, at);
        const Engine::Mat4 projection = Engine::Mat4::projection(60.f, window_size.getX() / window_size.getY(), 0.1f, 100.f, 0);
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
        
        Engine::SpriteManager sprite_manager(shader);

        Engine::Lua lua(sprite_manager);
        lua.registerTypes<
            Engine::Vec2,
            Engine::Vec3,
            Engine::Sprite
        >();
        lua.pushSource(script);

        lua.runOnStart();
        //auto& sprite = sprite_manager.createSprite();

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
