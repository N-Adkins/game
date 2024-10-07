#include <pch.hpp>

#include "array.hpp"
#include "opengl.hpp"
#include "gfx/buffer.hpp"

namespace Engine {

VertexArray::VertexArray()
{
    OPENGL_CALL(glGenVertexArrays(1, &vao));
}

VertexArray::~VertexArray()
{
    OPENGL_CALL(glDeleteVertexArrays(1, &vao));
}

void VertexArray::addBuffer(const VertexBuffer& buffer, const VertexBufferLayout& layout)
{
    bind();
    buffer.bind();
    const auto& attribs = layout.getAttributes();
    size_t offset = 0;
    for (size_t i = 0; i < attribs.size(); i++) {
        const auto& attrib = attribs[i];
        OPENGL_CALL(glVertexAttribPointer(
                static_cast<GLuint>(i), 
                static_cast<GLint>(attrib.getCount()),
                static_cast<GLenum>(attrib.getType()), 
                GL_FALSE, 
                static_cast<GLsizei>(layout.getStride()),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(offset))
        ));
        offset += attrib.getCount() * AttributeDescriptor::getTypeSize(attrib.getType());
    }
    OPENGL_CALL(glEnableVertexAttribArray(0));
}

void VertexArray::bind() const
{
    OPENGL_CALL(glBindVertexArray(vao));
}

void VertexArray::unbind() const
{
    OPENGL_CALL(glBindVertexArray(0));
}

} // namespace Engine
