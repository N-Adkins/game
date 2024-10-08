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
        Log::debug("Attrib pointer call, index {} count {} type {} stride {} offset {}",
            i, attrib.getCount(), static_cast<GLenum>(attrib.getType()), layout.getStride(), offset
        );
        if (attrib.getType() == AttributeType::UInt || attrib.getType() == AttributeType::Int) {
            OPENGL_CALL(glVertexAttribIPointer(
                static_cast<GLuint>(i), 
                static_cast<GLint>(attrib.getCount()),
                static_cast<GLenum>(attrib.getType()), 
                static_cast<GLsizei>(layout.getStride()),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(offset))
            ));
        } else {
            OPENGL_CALL(glVertexAttribPointer(
                static_cast<GLuint>(i), 
                static_cast<GLint>(attrib.getCount()),
                static_cast<GLenum>(attrib.getType()), 
                GL_FALSE, 
                static_cast<GLsizei>(layout.getStride()),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(offset))
            ));
        }
        OPENGL_CALL(glEnableVertexAttribArray(static_cast<GLuint>(i)));
        offset += attrib.getCount() * AttributeDescriptor::getTypeSize(attrib.getType());
    }
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
