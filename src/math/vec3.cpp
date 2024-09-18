#include "vec3.hpp"
#include <cmath>
#include <format>

namespace Engine {

float Vec3::getX() const
{
    return x;
}

float Vec3::getY() const
{
    return y;
}

float Vec3::getZ() const
{
    return z;
}

void Vec3::setX(float x)
{
    this->x = x;
}

void Vec3::setY(float y)
{
    this->y = y;
}

void Vec3::setZ(float z)
{
    this->z = z;
}

float Vec3::dot(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

float Vec3::magnitude() const
{
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::unit() const
{
    const float mag = magnitude();
    return *this / mag;
}

Vec3 Vec3::operator+(const Vec3& rhs) const
{
    return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

void Vec2::operator+=(const Vec2& rhs)
{
    x += rhs.x;
    y += rhs.y;
}

Vec2 Vec2::operator-(const Vec2& rhs) const
{
    return Vec2(x - rhs.x, y - rhs.y);
}

void Vec2::operator-=(const Vec2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
}

Vec2 Vec2::operator*(float rhs) const
{
    return Vec2(x * rhs, y * rhs);
}

void Vec2::operator*=(float rhs)
{
    x *= rhs;
    y *= rhs;
}

Vec2 Vec2::operator/(float rhs) const
{
    return Vec2(x / rhs, y / rhs);
}

void Vec2::operator/=(float rhs)
{
    x /= rhs;
    y /= rhs;
}

Vec2 Vec2::operator-() const
{
    return Vec2(-x, -y);
}

void Vec2::registerLua(sol::state &lua)
{
    auto vec2 = lua.new_usertype<Vec2>("Vec2", sol::constructors<Vec2(float, float)>());
    vec2["x"] = sol::property(&Vec2::getX, &Vec2::setX);
    vec2["y"] = sol::property(&Vec2::getY, &Vec2::setY);
    vec2["magnitude"] = &Vec2::magnitude;
    vec2["unit"] = &Vec2::unit;
    vec2["dot"] = &Vec2::dot;
    vec2["__add"] = &Vec2::operator+; 
    vec2["__mul"] = &Vec2::operator*;
    vec2["__div"] = &Vec2::operator/;
    vec2["__eq"] = &Vec2::operator==;
    vec2["__tostring"] = [](const Vec2& self) {
        return std::format("Vec2 {{ x: {}, y: {} }}", self.getX(), self.getY()); 
    };

    // Ambiguous operators so we have to do casts
    vec2["__sub"] = static_cast<Vec2(Vec2::*)(const Vec2&) const>(&Vec2::operator-);
    vec2["__unm"] = static_cast<Vec2(Vec2::*)() const>(&Vec2::operator-);
}

} // namespace Engine
