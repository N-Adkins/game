#ifndef GAME_MATH_MAT4_HPP
#define GAME_MATH_MAT4_HPP

#include <array>

class Mat4 {
public:
    Mat4();

private:
    std::array<float, 16> values;
};

#endif
