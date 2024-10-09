#pragma once

#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../resource/lua_source.hpp"
#include "event.hpp"
#include "sprite.hpp"
#include <vector>
#include <unordered_map>
#include <sol/forward.hpp>

namespace Engine {

class Lua {
public:
    Lua(SpriteManager& sprite_manager);

    template <typename T>
    void registerType();

    template <typename... Args>
    void registerTypes();

    void initSpriteManager(SpriteManager& sprite_manager);

    void pushSource(const LuaSource& source);
    void runOnStart();
    void runOnFrame();
    
    void gc();
    
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

template <> void Lua::registerType<Vec2>();
template <> void Lua::registerType<Vec3>();
template <> void Lua::registerType<Sprite>();
template <> void Lua::registerType<Event>();
template <> void Lua::registerType<EventConnection>();

} // namespace Engine
