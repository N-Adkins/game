#include <sol/sol.hpp>
#include "math/vec2.hpp"
#include <format>

using namespace Engine;

int main()
{
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    auto vec2 = lua.new_usertype<Vec2>(
        "Vec2",
        "getX",
        &Vec2::getX,
        "getY",
        &Vec2::getY,
        "setX",
        &Vec2::setX,
        "setY",
        &Vec2::setY
    );
    vec2["new"] = [](float x, float y) {
        return Vec2(x, y);
    };
    vec2["__tostring"] = [](const Vec2& self){
        return std::format("Vec2 {{ x: {}, y: {} }}", self.getX(), self.getY()); 
    };

    const auto& code = R"(
        local vec = Vec2.new(10, 3)
        print(vec)
    )";

    lua.script(code);

    return 0;
}
