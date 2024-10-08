#include <pch.hpp>
#include <sol/types.hpp>

#include "lua.hpp"

namespace Engine {

// We have to do this because we use references. Might rework in the future.
struct SpriteWrapper {
    SpriteWrapper(Sprite& ref)
        : ref(ref) {}
    Sprite& ref;
};

Lua::Lua(SpriteManager& sprite_manager)
{
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string, sol::lib::package, sol::lib::math);

    lua["package"]["preload"]["Engine"] = [&](sol::this_state state) {
        sol::state_view lua(state);
        sol::table engine = lua.create_table();

        engine["CreateSprite"] = [&sprite_manager]() -> SpriteWrapper {
            return SpriteWrapper(sprite_manager.createSprite());
        };

        return engine;
    };
}

void Lua::pushSource(const LuaSource& source)
{
    sol::table env = lua.script(source.getSource()); 
    scripts.push_back(std::move(env));
}

void Lua::runOnStart()
{
    for (auto& script : scripts) {
        if (sol::function func = script["OnStart"]) {
            func(script);
        }
    }
}

void Lua::runOnFrame()
{
    for (auto& script : scripts) {
        if (sol::function func = script["OnFrame"]) {
            func(script);
        }
    }
}

void Lua::initSpriteManager(SpriteManager& sprite_manager)
{
    lua.globals()["Engine"]["CreateSprite"] = [&sprite_manager]() -> Sprite& {
        return sprite_manager.createSprite();
    };
}

void Lua::gc()
{
    lua.collect_garbage();
}

template <>
void Lua::registerType<Vec2>()
{
    auto vec2 = lua.new_usertype<Vec2>("Vec2", sol::constructors<Vec2(float, float)>());
    vec2["x"] = sol::property(&Vec2::getX, &Vec2::setX);
    vec2["y"] = sol::property(&Vec2::getY, &Vec2::setY);
    vec2["Magnitude"] = &Vec2::magnitude;
    vec2["Unit"] = &Vec2::unit;
    vec2["Dot"] = &Vec2::dot;
    vec2[sol::meta_method::addition] = &Vec2::operator+; 
    vec2[sol::meta_method::multiplication] = &Vec2::operator*;
    vec2[sol::meta_method::division] = &Vec2::operator/;
    vec2[sol::meta_method::equal_to] = &Vec2::operator==;
    vec2[sol::meta_method::to_string] = [](const Vec2& self) {
        return std::format("Vec2 {{ x: {}, y: {} }}", self.getX(), self.getY()); 
    };

    // Ambiguous operators so we have to do casts
    vec2[sol::meta_method::subtraction] = static_cast<Vec2(Vec2::*)(const Vec2&) const>(&Vec2::operator-);
    vec2[sol::meta_method::unary_minus] = static_cast<Vec2(Vec2::*)() const>(&Vec2::operator-);
}

template <>
void Lua::registerType<Vec3>()
{
    auto vec3 = lua.new_usertype<Vec3>("Vec3", sol::constructors<Vec3(float, float, float)>());
    vec3["x"] = sol::property(&Vec3::getX, &Vec3::setX);
    vec3["y"] = sol::property(&Vec3::getY, &Vec3::setY);
    vec3["z"] = sol::property(&Vec3::getZ, &Vec3::setZ);
    vec3["Magnitude"] = &Vec3::magnitude;
    vec3["Unit"] = &Vec3::unit;
    vec3["Dot"] = &Vec3::dot;
    vec3[sol::meta_method::addition] = &Vec3::operator+; 
    vec3[sol::meta_method::multiplication] = &Vec3::operator*;
    vec3[sol::meta_method::division] = &Vec3::operator/;
    vec3[sol::meta_method::equal_to] = &Vec3::operator==;
    vec3[sol::meta_method::to_string] = [](const Vec3& self) {
        return std::format("Vec3 {{ x: {}, y: {}, z: {} }}", self.getX(), self.getY(), self.getZ()); 
    };

    // Ambiguous operators so we have to do casts
    vec3[sol::meta_method::subtraction] = static_cast<Vec3(Vec3::*)(const Vec3&) const>(&Vec3::operator-);
    vec3[sol::meta_method::unary_minus] = static_cast<Vec3(Vec3::*)() const>(&Vec3::operator-);
}

template <>
void Lua::registerType<Sprite>()
{
    auto sprite = lua.new_usertype<SpriteWrapper>("Sprite");
    sprite["position"] = sol::property(
        [](const SpriteWrapper& sprite) {
            return sprite.ref.getPosition();
        },
        [](const SpriteWrapper& sprite, Vec2 position) {
            return sprite.ref.setPosition(position);
        }
    );
    sprite["scale"] = sol::property(
        [](const SpriteWrapper& sprite) {
            return sprite.ref.getScale();
        },
        [](const SpriteWrapper& sprite, float scale) {
            return sprite.ref.setScale(scale);
        }
    );
    sprite[sol::meta_method::garbage_collect] = [](SpriteWrapper& self) {
        self.ref.destroy();
    };
    sprite[sol::meta_method::equal_to] = [](const SpriteWrapper& lhs, const SpriteWrapper& rhs) {
        return lhs.ref.getId() == rhs.ref.getId();
    };
    sprite[sol::meta_method::to_string] = [](const SpriteWrapper& self) {
        return std::format("Sprite {{ id: {} }}", self.ref.getId()); 
    };
}

} // namespace Engine
