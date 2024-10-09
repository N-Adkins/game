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

#include <glm/ext.hpp>

#include <cassert>

void handleEvent(SDL_Event& e, Engine::Lua& lua) {
    if (e.key.repeat != 0) { // ignore repeat signals, OS dependent
        return;
    }
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_DOWN: 
        case SDLK_LEFT: 
        case SDLK_RIGHT: 
            lua.fireBuiltinEvent("OnKeyPressed", static_cast<Engine::KeyCode>(e.key.keysym.sym));
            lua.setKeyState(static_cast<Engine::KeyCode>(e.key.keysym.sym), true);
            break;
        default: break;
        }
    } else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_DOWN: 
        case SDLK_LEFT: 
        case SDLK_RIGHT: 
            lua.fireBuiltinEvent("OnKeyReleased", static_cast<Engine::KeyCode>(e.key.keysym.sym));
            lua.setKeyState(static_cast<Engine::KeyCode>(e.key.keysym.sym), false);
            break;
        default: break;
        }
    }
}

void renderLoop(
    Engine::Lua& lua,
    Engine::Window& window,
    Engine::Renderer& renderer,
    Engine::ResourceManager& resource_manager,
    Engine::SpriteManager& sprite_manager,
    const Engine::Shader& shader
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
            handleEvent(e, lua);
        }

        const glm::vec2 window_size = window.getSize();
        const glm::mat4 projection = glm::ortho(
            -(window_size.x / 2),
            window_size.x / 2,
            -(window_size.y / 2),
            window_size.y / 2,
            -1.f, 1.f
        );
        shader.setUniform("projection", projection);

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
            static_cast<size_t>(window.getSize().x), 
            static_cast<size_t>(window.getSize().y)
        );
        window.setRenderer(&renderer);

        Engine::ResourceManager resource_manager;
        const auto& shader = resource_manager.load<Engine::Shader>("test.shader");
        const auto& script = resource_manager.load<Engine::LuaSource>("test.lua");

        Engine::SpriteManager sprite_manager(shader);

        Engine::Lua lua(sprite_manager);
        lua.registerTypes<
            glm::vec2,
            glm::vec3,
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
            sprite_manager,
            shader
        );
    }

    ImGui::DestroyContext();
    
    return 0;
}
