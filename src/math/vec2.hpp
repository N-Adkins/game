#ifndef GAME_MATH_VEC2_HPP
#define GAME_MATH_VEC2_HPP

namespace Engine {
    
class Vec2 {
public:
    Vec2(float x, float y)
        : x(x), y(y) {}

private:
    float x = 0.f;
    float y = 0.f;
};

} // namespace Engine

#endif
