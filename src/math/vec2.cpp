#include <pch.hpp>

#include "vec2.hpp"
#include "floats.hpp"
#include <cmath>

namespace Engine {

float Vec2::getX() const
{
    return x;
}

float Vec2::getY() const
{
    return y;
}

void Vec2::setX(float x)
{
    this->x = x;
}

void Vec2::setY(float y)
{
    this->y = y;
}

float Vec2::dot(const Vec2& lhs, const Vec2& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

float Vec2::magnitude() const
{
    return std::sqrt(x * x + y * y);
}

Vec2 Vec2::unit() const
{
    const float mag = magnitude();
    return *this / mag;
}

Vec2 Vec2::operator+(const Vec2& rhs) const
{
    return Vec2(x + rhs.x, y + rhs.y);
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

bool Vec2::operator==(const Vec2& rhs) const
{
    return approxEqual(x, rhs.getX()) && approxEqual(y, rhs.getY());
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
