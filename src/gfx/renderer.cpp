#include <pch.hpp>

#include "renderer.hpp"
#include "opengl.hpp"
#include <backends/imgui_impl_opengl3.h>

namespace Engine {

Renderer::Renderer()
{
    OPENGL_CALL(glEnable(GL_DEBUG_OUTPUT));
    OPENGL_CALL(glDebugMessageCallback(debugCallback, nullptr));
    OPENGL_CALL(glDisable(GL_DEPTH_TEST));
    ImGui_ImplOpenGL3_Init("#version 410");
}

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
}

void Renderer::setViewport(size_t width, size_t height)
{
    OPENGL_CALL(glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height)));
}

void Renderer::setBackgroundColor(const glm::vec3& color)
{
    background_color = color;
}

void Renderer::clearBackground()
{
    OPENGL_CALL(glClearColor(background_color.x, background_color.y, background_color.z, 1.f));
    OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) 
{
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

} // namespace Engine
