#include <pch.hpp>

#include "lua.hpp"
#include "keycodes.hpp"
#include "logging.hpp"
#include "../platform.hpp"
#include <sol/forward.hpp>
#include <sol/protected_function_result.hpp>
#include <sol/trampoline.hpp>
#include <sstream>

namespace Engine {

void Lua::panic(std::optional<std::string> maybe_message)
{
    if (maybe_message.has_value()) {
        Log::error("Lua panic: {}", *maybe_message);
    } else {
        Log::error("Lua has panicked without a message. :-)");
    }
}

Lua::Lua(SpriteManager& sprite_manager)
{
    lua.set_panic(sol::c_call<decltype(&Lua::panic), &Lua::panic>);

    lua.set_exception_handler([](
        lua_State* state, 
        sol::optional<const std::exception&> maybe_exception,
        std::string_view description) -> int
    {
        if (maybe_exception.has_value()) {
            Log::error("Lua exception: {}", (*maybe_exception).what());
        } else {
            Log::error("Lua error description: {}", description);
        }
        return 0;
    });

    lua.open_libraries(
        sol::lib::base, 
        sol::lib::io, 
        sol::lib::string, 
        sol::lib::package, 
        sol::lib::math,
        sol::lib::debug
    );

    lua["package"]["path"] = lua["package"]["path"].get<std::string>() + ";" + (getExecutablePath() / "resources" / "?.lua").string();

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

        engine["IsKeyPressed"] = [this](KeyCode keycode) -> bool {
            return key_state[keycode];
        };

        engine["SetVSync"] = [](bool enable) {
            SDL_GL_SetSwapInterval(enable ? 1 : 0);
        };

        return engine;
    };

    // Overriding the builtin print function to use the engine logging
    lua["print"] = [](sol::variadic_args va, sol::this_state ts) {
        std::stringstream buffer;
        sol::state_view lua(ts);

        std::optional<sol::table> debug_info_maybe = lua["debug"]["getinfo"](2, "Sl");
        std::string prefix = "[unknown]";
        
        if (debug_info_maybe) {
            auto debug_info = *debug_info_maybe;
            int line_num = debug_info["currentline"].get_or(-1);
            std::string script_name;
            if (std::optional<std::string> name = debug_info["source"]) {
                // Attempt to remove everything before the resources folder to keep logs
                // cleaner
                size_t folder_pos = (*name).find("resources/");
                if (folder_pos != std::string::npos) {
                    script_name = (*name).substr(folder_pos);
                } else {
                    script_name = *name;
                }
            } else {
                script_name = "[unknown_script]";
            }
            prefix = std::format(
                "[{}:{}]", 
                script_name,
                line_num
            );
        }

        for (auto arg : va) {
            buffer << lua["tostring"](arg).get<std::string>();
            buffer << "\t";
        }

        Log::GLOBAL_LOGGER.logCustomPrefix(Log::Logger::Severity::Info, buffer.str(), prefix);
    };
}

void Lua::runEntryPoint(const LuaSource& source)
{
    try {
        lua.script_file(source.getSource()); 
    } catch (const sol::error& e) {
        Log::error("Lua compilation error: {}", e.what());
    }
}

void Lua::gc()
{
    lua.collect_garbage();
}

void Lua::setKeyState(KeyCode keycode, bool state)
{
    key_state[keycode] = state;
}

template <>
void Lua::registerType<glm::vec2>()
{
    auto vec2 = lua.new_usertype<glm::vec2>("Vec2", sol::constructors<glm::vec3(float, float)>());
    vec2["x"] = &glm::vec2::x;
    vec2["y"] = &glm::vec2::y;
    vec2["Length"] = &glm::vec2::length;
    vec2["Normalize"] = [](const glm::vec2& self) -> glm::vec2 {
        return glm::normalize(self);
    };
    vec2["Dot"] = [](const glm::vec2& lhs, const glm::vec2& rhs) {
        return glm::dot(lhs, rhs); 
    };
    vec2[sol::meta_method::addition] = [](const glm::vec2& lhs, const glm::vec2& rhs) {
        return lhs + rhs;
    };
    vec2[sol::meta_method::subtraction] = [](const glm::vec2& lhs, const glm::vec2& rhs) {
        return lhs - rhs;
    };
    vec2[sol::meta_method::multiplication] = [](const glm::vec2& lhs, float rhs) {
        return lhs * rhs;
    };
    vec2[sol::meta_method::division] = [](const glm::vec2& lhs, float rhs) {
        return lhs / rhs;
    };
    vec2[sol::meta_method::unary_minus] = [](const glm::vec2& self) {
        return -self;
    };
    vec2[sol::meta_method::to_string] = [](const glm::vec2& self) {
        return std::format("Vec2 {{ x: {}, y: {} }}", self.x, self.y); 
    };
}

template <>
void Lua::registerType<glm::vec3>()
{
    auto vec3 = lua.new_usertype<glm::vec3>("Vec3", sol::constructors<glm::vec3(float, float, float)>());
    vec3["x"] = &glm::vec3::x;
    vec3["y"] = &glm::vec3::y;
    vec3["z"] = &glm::vec3::z;
    vec3["Length"] = &glm::vec3::length;
    vec3["Normalize"] = [](const glm::vec3& self) -> glm::vec3 {
        return glm::normalize(self);
    };
    vec3["Dot"] = [](const glm::vec3& lhs, const glm::vec3& rhs) {
        return glm::dot(lhs, rhs); 
    };
    vec3[sol::meta_method::addition] = [](const glm::vec3& lhs, const glm::vec3& rhs) {
        return lhs + rhs;
    };
    vec3[sol::meta_method::subtraction] = [](const glm::vec3& lhs, const glm::vec3& rhs) {
        return lhs - rhs;
    };
    vec3[sol::meta_method::multiplication] = [](const glm::vec3& lhs, float rhs) {
        return lhs * rhs;
    };
    vec3[sol::meta_method::division] = [](const glm::vec3& lhs, float rhs) {
        return lhs / rhs;
    };
    vec3[sol::meta_method::unary_minus] = [](const glm::vec3& self) {
        return -self;
    };
    vec3[sol::meta_method::to_string] = [](const glm::vec3& self) {
        return std::format("Vec3 {{ x: {}, y: {}, z: {} }}", self.x, self.y, self.z); 
    };
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
