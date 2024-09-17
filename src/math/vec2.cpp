#include "vec2.hpp"

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

} // namespace Engine
