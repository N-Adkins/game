#include <pch.hpp>

#include "lua.hpp"
#include "keycodes.hpp"

namespace Engine {

Lua::Lua(SpriteManager& sprite_manager)
{
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string, sol::lib::package, sol::lib::math);

    lua["package"]["preload"]["Engine"] = [&](sol::this_state state) {
        sol::state_view lua(state);
        sol::table engine = lua.create_table();

        engine["CreateSprite"] = [&sprite_manager]() -> Sprite { 
            return sprite_manager.createSprite();
        };

        engine["Events"] = lua.create_table();
        for (auto& [name, event] : builtin_events) {
            engine["Events"][name] = &event;
        }

        engine["KeyCode"] = lua.create_table_with(
            "Up", KeyCode::Up,
            "Down", KeyCode::Down,
            "Left", KeyCode::Left,
            "Right", KeyCode::Right
        );

        return engine;
    };
}

void Lua::pushSource(const LuaSource& source)
{
    sol::table env = lua.safe_script(source.getSource()); 
    scripts.push_back(Script{
        .table = env,
        .name = source.getName(),
    });
}

void Lua::runOnStart()
{
    for (auto& script : scripts) {
        if (sol::protected_function func = script.table["OnStart"]) {
            if (auto result = func(script); !result.valid()) {
                sol::error err = result;
                Log::error("Lua script error at {}: {}", script.name, err.what());
            };
        }
    }
}

void Lua::runOnFrame()
{
    for (auto& script : scripts) {
        if (sol::protected_function func = script.table["OnFrame"]) {
            if (auto result = func(script); !result.valid()) {
                sol::error err = result;
                Log::error("Lua script error at {}: {}", script.name, err.what());
            };
        }
    }
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
    auto sprite = lua.new_usertype<Sprite>("Sprite");
    sprite["position"] = sol::property(&Sprite::getPosition, &Sprite::setPosition);
    sprite["scale"] = sol::property(&Sprite::getScale, &Sprite::setScale);
    sprite["Destroy"] = &Sprite::destroy;
    sprite[sol::meta_method::equal_to] = [](const Sprite& lhs, const Sprite& rhs) {
        return lhs.getId() == rhs.getId();
    };
    sprite[sol::meta_method::to_string] = [](const Sprite& self) {
        return std::format("Sprite {{ id: {} }}", self.getId()); 
    };
}

template <> 
void Lua::registerType<Event>()
{
    auto event = lua.new_usertype<Event>("Event", sol::constructors<Event()>());
    event["Connect"] = &Event::connect;
    event["Fire"] = &Event::fireVariadic;
}

template <> 
void Lua::registerType<EventConnection>()
{
    auto event_conn = lua.new_usertype<EventConnection>("EventConnection");
    event_conn["Disconnect"] = &EventConnection::disconnect;
}

} // namespace Engine
