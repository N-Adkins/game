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

} // namespace Engine
