#pragma once

#include "buffer.hpp"
#include "../constructors.hpp"

namespace Engine {

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    DELETE_COPY(VertexArray);
    DEFAULT_MOVE(VertexArray);

    void addBuffer(const VertexBuffer& buffer, const VertexBufferLayout& layout);
    void bind() const;
    void unbind() const;

private:
    unsigned int vao;
};

} // namespace Engine
