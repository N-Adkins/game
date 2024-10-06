#include <pch.hpp>

#include "renderer.hpp"

namespace Engine {

void Renderer::setViewport(size_t width, size_t height)
{
    glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(height));
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
    glClearColor(background_color.getX(), background_color.getY(), background_color.getZ(), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace Engine
