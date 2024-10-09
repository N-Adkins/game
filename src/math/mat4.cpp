#include <pch.hpp>

#include "mat4.hpp"
#include <cmath>

namespace Engine {

Mat4::Mat4()
{
    values = Mat4(0.f).values;
}

Mat4::Mat4(float splat)
{
    for (size_t i = 0; i < 16; i++) {
        values[i] = splat;
    }
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& at, const Vec3& up)
{
    const Vec3 zaxis = (at - eye).unit();
    const Vec3 xaxis = Vec3::cross(up, zaxis).unit();
    const Vec3 yaxis = Vec3::cross(zaxis, xaxis);

    Mat4 mat;
    mat.values[0] = xaxis.getX();
    mat.values[1] = yaxis.getX();
    mat.values[2] = zaxis.getX();
    mat.values[3] = 0;
    mat.values[4] = xaxis.getY();
    mat.values[5] = yaxis.getY();
    mat.values[6] = zaxis.getY();
    mat.values[7] = 0;
    mat.values[8] = xaxis.getZ();
    mat.values[9] = yaxis.getZ();
    mat.values[10] = zaxis.getZ();
    mat.values[11] = 0;
    mat.values[12] = -Vec3::dot(xaxis, eye);
    mat.values[13] = -Vec3::dot(yaxis, eye);
    mat.values[14] = -Vec3::dot(zaxis, eye);
    mat.values[15] = 0;

    return mat;
}

Mat4 Mat4::ortho(float left, float right, float bottom, float top, float near, float far) 
{
    Mat4 mat;

    // Orthographic projection calculation
    mat.values[0] = 2.f / (right - left);
    mat.values[5] = 2.f / (top - bottom);
    mat.values[10] = -2.f / (far - near);
    mat.values[12] = -(right + left) / (right - left);
    mat.values[13] = -(top + bottom) / (top - bottom);
    mat.values[14] = -(far + near) / (far - near);
    mat.values[15] = 1.f;

    return mat;
}

Mat4 Mat4::rotate(float angle, const Vec3& axis) const
{
    Vec3 normalized_axis = axis.unit();
    float cos_angle = std::cos(angle);
    float sin_angle = std::sin(angle);
    float one_minus_cos = 1.0f - cos_angle;

    // Extract the axis components
    float x = normalized_axis.getX();
    float y = normalized_axis.getY();
    float z = normalized_axis.getZ();

    // Construct the rotation matrix (Rodrigues' rotation formula)
    Mat4 rotation;
    rotation.values[0] = cos_angle + x * x * one_minus_cos;
    rotation.values[1] = x * y * one_minus_cos + z * sin_angle;
    rotation.values[2] = x * z * one_minus_cos - y * sin_angle;
    rotation.values[3] = 0.0f;

    rotation.values[4] = y * x * one_minus_cos - z * sin_angle;
    rotation.values[5] = cos_angle + y * y * one_minus_cos;
    rotation.values[6] = y * z * one_minus_cos + x * sin_angle;
    rotation.values[7] = 0.0f;

    rotation.values[8] = z * x * one_minus_cos+ y * sin_angle;
    rotation.values[9] = z * y * one_minus_cos- x * sin_angle;
    rotation.values[10] = cos_angle + z * z * one_minus_cos;
    rotation.values[11] = 0.0f;

    rotation.values[12] = 0.0f;
    rotation.values[13] = 0.0f;
    rotation.values[14] = 0.0f;
    rotation.values[15] = 1.0f;

    // Multiply the input matrix with the rotation matrix
    return *this * rotation;
}

Mat4 Mat4::translate(const Vec3& translation)
{
    Mat4 result;

    // Apply the translation directly to this matrix
    result.values[12] = values[0] * translation.getX() + values[4] * translation.getY() + values[8] * translation.getZ() + values[12];
    result.values[13] = values[1] * translation.getX() + values[5] * translation.getY() + values[9] * translation.getZ() + values[13];
    result.values[14] = values[2] * translation.getX() + values[6] * translation.getY() + values[10] * translation.getZ() + values[14];
    result.values[15] = values[3] * translation.getX() + values[7] * translation.getY() + values[11] * translation.getZ() + values[15];

    return result;  // Return the updated matrix
}

Mat4 Mat4::operator*(const Mat4& rhs) const
{
    Mat4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.values[row * 4 + col] = 
                values[row * 4 + 0] * rhs.values[0 * 4 + col] + 
                values[row * 4 + 1] * rhs.values[1 * 4 + col] + 
                values[row * 4 + 2] * rhs.values[2 * 4 + col] + 
                values[row * 4 + 3] * rhs.values[3 * 4 + col];
        }
    }
    return result;
}

float Mat4::get(size_t index) const
{
    return values[index];
}

void Mat4::set(size_t index, float value)
{
    values[index] = value;
}

const float* Mat4::getValues() const
{
    return &values[0];
}

} // namespace Engine
