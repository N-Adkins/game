#pragma once

#include "../math/vec3.hpp"
#include <cstddef>

namespace Engine {

class Renderer {
public:
    Renderer();
    void setViewport(size_t width, size_t height);
    void setBackgroundColor(const Vec3& color);
    void clearBackground();

private:
    Vec3 background_color = Vec3(0.2f, 0.3f, 0.3f);
};

} // namespace Engine
