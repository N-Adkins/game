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

void Vec3::registerLua(sol::state &lua)
{
    auto vec3 = lua.new_usertype<Vec3>("Vec3", sol::constructors<Vec3(float, float, float)>());
    vec3["x"] = sol::property(&Vec3::getX, &Vec3::setX);
    vec3["y"] = sol::property(&Vec3::getY, &Vec3::setY);
    vec3["z"] = sol::property(&Vec3::getZ, &Vec3::setZ);
    vec3["magnitude"] = &Vec3::magnitude;
    vec3["unit"] = &Vec3::unit;
    vec3["dot"] = &Vec3::dot;
    vec3["__add"] = &Vec3::operator+; 
    vec3["__mul"] = &Vec3::operator*;
    vec3["__div"] = &Vec3::operator/;
    vec3["__eq"] = &Vec3::operator==;
    vec3["__tostring"] = [](const Vec3& self) {
        return std::format("Vec3 {{ x: {}, y: {}, z: {} }}", self.getX(), self.getY(), self.getZ()); 
    };

    // Ambiguous operators so we have to do casts
    vec3["__sub"] = static_cast<Vec3(Vec3::*)(const Vec3&) const>(&Vec3::operator-);
    vec3["__unm"] = static_cast<Vec3(Vec3::*)() const>(&Vec3::operator-);
}

} // namespace Engine
