#include "mat4.hpp"

#include <cmath>

namespace Engine {

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& at, const Vec3& up)
{
    const Vec3 forward = eye.unit();
    const Vec3 right = Vec3::cross(up, forward).unit();
    const Vec3 true_up = Vec3::cross(forward, right);
    Mat4 mat;
    mat.values[0] = right.getX();
    mat.values[1] = up.getX();
    mat.values[2] = -forward.getX();
    mat.values[3] = 0.f;
    mat.values[4] = right.getY();
    mat.values[5] = up.getY();
    mat.values[6] = -forward.getY();
    mat.values[7] = 0.f;
    mat.values[8] = right.getZ();
    mat.values[9] = up.getZ();
    mat.values[10] = -forward.getZ();
    mat.values[11] = 0.f;
    mat.values[12] = -Vec3::dot(right, eye);
    mat.values[13] = -Vec3::dot(up, eye);
    mat.values[14] = Vec3::dot(forward, eye);
    mat.values[15] = 1.f;
    return mat;
}

Mat4 Mat4::projection(float fov, float aspect, float near, float far, bool depth)
{
    const float tanHalfFov = std::tanf(fov * 0.5f);

    Mat4 mat;
    mat.values[0] = 1.f / (aspect * tanHalfFov);
    mat.values[1] = 0.f;
    mat.values[2] = 0.f;
    mat.values[3] = 0.f;
    mat.values[4] = 0.f;
    mat.values[5] = 1.f / tanHalfFov;
    mat.values[6] = 0.f;
    mat.values[7] = 0.f;
    mat.values[8] = 0.f;
    mat.values[9] = 0.f;
    mat.values[10] = depth ? (far + near) / (near - far) : far / (far - near);
    mat.values[11] = -1.f;
    mat.values[12] = 0.f;
    mat.values[13] = 0.f;
    mat.values[14] = depth ? (2.f * far * near) / (near - far) : (near * far) / (near - far);
    mat.values[15] = 0.f;
    return mat;
}

} // namespace Engine
