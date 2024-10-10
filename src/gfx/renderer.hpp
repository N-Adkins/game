#pragma once

#include <cstddef>
#include <glm/fwd.hpp>

namespace Engine {

class Renderer {
public:
    Renderer();
    ~Renderer();
    void setViewport(size_t width, size_t height);
    void setBackgroundColor(const glm::vec3& color);
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

    glm::vec3 background_color = { 0.2f, 0.3f, 0.3f };
};

} // namespace Engine
