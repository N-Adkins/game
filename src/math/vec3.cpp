#include <pch.hpp>

#include "vec3.hpp"
#include "floats.hpp"
#include <cmath>

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


Vec3 Vec3::cross(const Vec3& lhs, const Vec3& rhs)
{
    return Vec3(
        lhs.getY() * rhs.getZ() - lhs.getZ() * rhs.getY(),
        lhs.getZ() * rhs.getX() - lhs.getX() * rhs.getZ(),
        lhs.getX() * rhs.getY() - lhs.getY() * rhs.getX()
    );
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

void Vec3::operator+=(const Vec3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
}

Vec3 Vec3::operator-(const Vec3& rhs) const
{
    return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

void Vec3::operator-=(const Vec3& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
}

Vec3 Vec3::operator*(float rhs) const
{
    return Vec3(x * rhs, y * rhs, z * rhs);
}

void Vec3::operator*=(float rhs)
{
    x *= rhs;
    y *= rhs;
    z *= rhs;
}

Vec3 Vec3::operator/(float rhs) const
{
    return Vec3(x / rhs, y / rhs, z / rhs);
}

void Vec3::operator/=(float rhs)
{
    x /= rhs;
    y /= rhs;
    z /= rhs;
}

Vec3 Vec3::operator-() const
{
    return Vec3(-x, -y, -z);
}

bool Vec3::operator==(const Vec3& rhs) const
{
    return approxEqual(x, rhs.getX()) 
        && approxEqual(y, rhs.getY()) 
        && approxEqual(z, rhs.getZ());
}

} // namespace Engine
