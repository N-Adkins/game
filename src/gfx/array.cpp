#include <pch.hpp>

#include "array.hpp"

namespace Engine {

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &vao);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &vao);
}

void VertexArray::addBuffer(const VertexBuffer& buffer, const VertexBufferLayout& layout)
{
    buffer.bind();
    const auto& attribs = layout.getAttributes();
    size_t offset = 0;
    for (size_t i = 0; i < attribs.size(); i++) {
        const auto& attrib = attribs[i];
        glEnableVertexAttribArray(static_cast<GLuint>(i));
        glVertexAttribPointer(static_cast<GLuint>(i), static_cast<GLint>(attrib.count), 
                static_cast<size_t>(attrib.type), GL_FALSE, static_cast<GLsizei>(layout.getStride()),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)));
        offset += attrib.count * AttributeDescriptor::getTypeSize(attrib.type);
    }
}

void VertexArray::bind() const
{
    glBindVertexArray(vao);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

} // namespace Engine
