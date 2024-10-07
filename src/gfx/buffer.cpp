#include <pch.hpp>

#include "buffer.hpp"

namespace Engine {

VertexBuffer::VertexBuffer()
{
    glGenBuffers(1, &vbo);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &vbo);
}

void VertexBuffer::buffer(const void* data, size_t size)
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
