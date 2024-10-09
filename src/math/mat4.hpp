#pragma once

#include "vec3.hpp"
#include <array>

namespace Engine {

class Mat4 {
public:
    Mat4();
    Mat4(float splat);
    static Mat4 lookAt(const Vec3& eye, const Vec3& at, const Vec3& up = Vec3(0.f, 1.f, 0.f));
    static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);
    Mat4 rotate(float angle, const Vec3& axis) const;
    Mat4 translate(const Vec3& translation);

    Mat4 operator*(const Mat4& rhs) const;

    float get(size_t index) const;
    void set(size_t index, float value);
    const float* getValues() const;

private:
    std::array<float, 16> values = { 0 };
};

} // namespace Engine
