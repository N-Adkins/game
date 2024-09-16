#ifndef GAME_MATH_VEC2_HPP
#define GAME_MATH_VEC2_HPP

namespace Engine {
    
class Vec2 {
public:
    Vec2(float x, float y)
        : x(x), y(y) {}

    float getX();
    float getY();
    void setX(float x);
    void setY(float y);

    Vec2 operator+(const Vec2& rhs);
    void operator+=(const Vec2& rhs);
    Vec2 operator-(const Vec2& rhs);
    void operator-=(const Vec2& rhs);
    Vec2 operator*(float rhs);
    void operator*=(float rhs);
    Vec2 operator/(float rhs);
    void operator/=(float rhs);
    void operator,(float rhs);

private:
    float x = 0.f;
    float y = 0.f;
};

} // namespace Engine

#endif
