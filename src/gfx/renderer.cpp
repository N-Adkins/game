#include <pch.hpp>

#include "renderer.hpp"
#include "opengl.hpp"

namespace Engine {

Renderer::Renderer()
{
    const auto callback = [](
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
    ) {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: {
            switch(severity) {
            case GL_DEBUG_SEVERITY_HIGH: Log::error("{}", message); break;
            case GL_DEBUG_SEVERITY_MEDIUM: Log::warn("{}", message); break;
            case GL_DEBUG_SEVERITY_LOW: Log::info("{}", message); break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: Log::debug("{}", message); break;
            default: break;
            }
        }
        default: break;
        }
    };

    OPENGL_CALL(glEnable(GL_DEBUG_OUTPUT));
    OPENGL_CALL(glDebugMessageCallback(callback, nullptr));
    OPENGL_CALL(glDisable(GL_DEPTH_TEST));
}

void Renderer::setViewport(size_t width, size_t height)
{
    OPENGL_CALL(glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height)));
}

void Renderer::setBackgroundColor(const Vec3& color)
{
    background_color = color;
}

void Renderer::clearBackground()
{
    OPENGL_CALL(glClearColor(background_color.getX(), background_color.getY(), background_color.getZ(), 1.f));
    OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

} // namespace Engine
