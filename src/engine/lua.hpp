#pragma once

#include "../resource/lua_source.hpp"
#include "event.hpp"
#include "sprite.hpp"
#include "keycodes.hpp"
#include <vector>
#include <unordered_map>
#include <sol/forward.hpp>
#include <glm/fwd.hpp>

namespace Engine {

class Lua {
public:
    Lua(SpriteManager& sprite_manager);

    template <typename T>
    void registerType();

    template <typename... Args>
    void registerTypes();

    void pushSource(const LuaSource& source);
    void runOnStart();
    void runOnFrame(float delta_time);
    
    void gc();

    void setKeyState(KeyCode keycode, bool state);
    
    template <typename... Args>
    void fireBuiltinEvent(const std::string& name, Args&&... args);
 
private:
    struct Script {
        sol::table table;
        std::string name;
    };

    sol::state lua;
    std::vector<Script> scripts;
    std::unordered_map<std::string, Event> builtin_events = {
        { "OnKeyPressed", Event() },
        { "OnKeyReleased", Event() },
    };
    std::unordered_map<KeyCode, bool> key_state = {
        { KeyCode::Up, false },
        { KeyCode::Down, false },
        { KeyCode::Left, false },
        { KeyCode::Right, false },
    };
};

template <typename T>
void Lua::registerType()
{
    static_assert(sizeof(T) == 0, "Type cannot be registered with Lua, needs specialization");
}

template <typename... Args>
void Lua::registerTypes()
{
    (registerType<Args>(), ...);
}

template <typename... Args>
void Lua::fireBuiltinEvent(const std::string& name, Args&&... args)
{
    if (builtin_events.contains(name)) {
        builtin_events[name].fire(std::forward<Args>(args)...);
    }
}

template <> void Lua::registerType<glm::vec2>();
template <> void Lua::registerType<glm::vec3>();
template <> void Lua::registerType<Sprite>();
template <> void Lua::registerType<Event>();
template <> void Lua::registerType<EventConnection>();

} // namespace Engine
