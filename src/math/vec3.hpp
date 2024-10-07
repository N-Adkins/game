#pragma once

#include "../platform.hpp"

namespace sol {
    class state;
}

namespace Engine {
    
GAME_PACKED_CLASS(Vec3, {
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
    static Vec3 cross(const Vec3& lhs, const Vec3& rhs);
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
    bool operator==(const Vec3& rhs) const;

    static void registerLua(sol::state& lua);

private:
    union {
        struct {
            float x;
            float y;
            float z;
        };
        float raw[3];
    };
});

} // namespace Engine
