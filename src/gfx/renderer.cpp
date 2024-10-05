#include <pch.hpp>

#include "renderer.hpp"

namespace Engine {

void Renderer::setViewport(size_t width, size_t height)
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
#endif
}

void Renderer::setBackgroundColor(const Vec3& color)
{
    background_color = color;
}

void Renderer::startFrame()
{

}

void Renderer::endFrame()
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    glClearColor(background_color.getX(), background_color.getY(), background_color.getZ(), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
}

} // namespace Engine
