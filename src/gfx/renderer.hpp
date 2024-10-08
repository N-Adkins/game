#pragma once

#include "../math/vec3.hpp"
#include <cstddef>

namespace Engine {

class Renderer {
public:
    Renderer();
    ~Renderer();
    void setViewport(size_t width, size_t height);
    void setBackgroundColor(const Vec3& color);
    void clearBackground();

private:
    static void debugCallback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    );

    Vec3 background_color = Vec3(0.2f, 0.3f, 0.3f);
};

} // namespace Engine
