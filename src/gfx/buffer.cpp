#include <pch.hpp>

#include "buffer.hpp"
#include "opengl.hpp"

namespace Engine {

VertexBuffer::VertexBuffer()
{
    OPENGL_CALL(glGenBuffers(1, &vbo));
}

VertexBuffer::~VertexBuffer()
{
    OPENGL_CALL(glDeleteBuffers(1, &vbo));
}

void VertexBuffer::buffer(const void* data, size_t size)
{
    empty = false;
    bind();
    OPENGL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

void VertexBuffer::bind() const
{
    OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
}

void VertexBuffer::unbind() const
{
    OPENGL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

bool VertexBuffer::isEmpty() const
{
    return empty;
}

const std::vector<AttributeDescriptor>& VertexBufferLayout::getAttributes() const
{
    return attributes;
}

size_t VertexBufferLayout::getStride() const
{
    return stride;
}

} // namespace Engine
