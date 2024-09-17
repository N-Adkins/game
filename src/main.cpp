#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <cassert>
#include "math/vec2.hpp"

using namespace Engine;

int main()
{
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::io, sol::lib::string);
    
    Vec2::registerLua(lua);
        
    const auto& code = R"(
        local vec = Vec2.new(0, 2) * 3
        local other = vec + Vec2.new(4, 5)
        print(Vec2.dot(vec, other))
        print(vec)
        print(-vec);
        print(vec - other);
        print(other)
    )";

    lua.safe_script(code);

    return 0;
}
