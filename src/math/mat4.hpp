#ifndef GAME_MATH_MAT4_HPP
#define GAME_MATH_MAT4_HPP

#include "vec3.hpp"
#include <array>

namespace Engine {

class Mat4 {
public:
    static Mat4 lookAt(const Vec3& eye, const Vec3& at, const Vec3& up = Vec3(0.f, 1.f, 0.f));
    static Mat4 projection(float fov, float aspect, float near, float far, bool depth);

    const float* getValues() const;

private:
    std::array<float, 16> values;
};

} // namespace Engine

#endif
