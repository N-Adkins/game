#include <pch.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

#include "engine/sprite.hpp"
#include "gfx/window.hpp"
#include "gfx/renderer.hpp"
#include "resource/resource_manager.hpp"
#include "resource/shader.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/mat4.hpp"
#include "logging.hpp"

#include <cassert>

void renderLoop()
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

    Engine::SpriteManager sprite_manager(shader);
    auto& sprite = sprite_manager.createSprite();

    bool loop = true;
    while (loop) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                loop = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        
        /*
        const Engine::Vec2 window_size = window.getSize();
        const Engine::Vec3 at(0.f, 0.f, 0.f);
        const Engine::Vec3 eye(0.f, 0.f, -5.f);
        const Engine::Mat4 view = Engine::Mat4::lookAt(eye, at);
        const Engine::Mat4 projection = Engine::Mat4::projection(60.f, window_size.getX() / window_size.getY(), 0.1f, 100.f, 0);
        */

        sprite.setScale(sprite.getScale() - 0.01f);
        
        renderer.clearBackground();
        sprite_manager.render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.swapBuffers();
    }
}

int main()
{
    sol::state lua;
    lua.create_table("Engine");
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string);
    Engine::Vec2::registerLua(lua);
    Engine::Vec3::registerLua(lua);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    renderLoop();

    ImGui::DestroyContext();
    
    return 0;
}
