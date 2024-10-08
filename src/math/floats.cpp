#include <pch.hpp>

#include "floats.hpp"
#include <cmath>
#include <limits>

namespace Engine {

bool approxEqual(float a, float b)
{
    return std::fabs(a - b) <= ( 
        (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) 
        * std::numeric_limits<float>::epsilon()
    );
}

float asRadians(float deg)
{
    return deg * M_PIf * 180.f;
}

} // namespace Engine
