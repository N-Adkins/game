#pragma once

#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../resource/lua_source.hpp"
#include "sprite.hpp"
#include <sol/usertype_container.hpp>
#include <sol/usertype_proxy.hpp>
#include <vector>
#include <sol/sol.hpp>

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

private:
    struct Script {
        sol::table table;
        std::string name;
    };

    sol::state lua;
    std::vector<Script> scripts;
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

template <> void Lua::registerType<Vec2>();
template <> void Lua::registerType<Vec3>();
template <> void Lua::registerType<Sprite>();

} // namespace Engine
