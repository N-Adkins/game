#ifndef GAME_MATH_VEC3_HPP
#define GAME_MATH_VEC3_HPP

#include <sol/sol.hpp>

namespace Engine {
    
class Vec3 {
public:
    Vec3(float x, float y, float z)
        : x(x), y(y), z(z) {}

    float getX() const;
    float getY() const;
    float getZ() const;
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    
    static float dot(const Vec3& lhs, const Vec3& rhs);
    float magnitude() const;
    Vec3 unit() const;

    Vec3 operator+(const Vec3& rhs) const;
    void operator+=(const Vec3& rhs);
    Vec3 operator-(const Vec3& rhs) const;
    void operator-=(const Vec3& rhs);
    Vec3 operator*(float rhs) const;
    void operator*=(float rhs);
    Vec3 operator/(float rhs) const;
    void operator/=(float rhs);
    Vec3 operator-() const; // unary negation
    bool operator==(const Vec3&) const = default;

    static void registerLua(sol::state& lua);

private:
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
};

} // namespace Engine

#endif
