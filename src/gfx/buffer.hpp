#pragma once

#include "../constructors.hpp"
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include <vector>
#include <glad/glad.h>

namespace Engine {

class VertexBuffer {
public:
    VertexBuffer(const void* data, size_t size);
    ~VertexBuffer();
    DELETE_COPY(VertexBuffer);
    DEFAULT_MOVE(VertexBuffer);

    void bind() const;
    void unbind() const;

private:
    unsigned int vbo;
};

enum class AttributeType {
    Float = GL_FLOAT,
};

struct AttributeDescriptor {
    AttributeType type;
    size_t count;

    inline static size_t getTypeSize(AttributeType type)
    {
        switch (type) {
            case AttributeType::Float: return sizeof(float);
        }
        return 0;
    }
};

struct VertexBufferLayout {
public:
    const std::vector<AttributeDescriptor>& getAttributes() const;
    size_t getStride() const;

    template <typename T>
    void push(size_t count);

private:
    std::vector<AttributeDescriptor> attributes;
    size_t stride = 0;
};

template <>
inline void VertexBufferLayout::push<float>(size_t count) {
    attributes.push_back({ .type = AttributeType::Float, .count = count });
    stride += sizeof(float) * count;
}

template <>
inline void VertexBufferLayout::push<Vec2>(size_t count) {
    attributes.push_back({ .type = AttributeType::Float, .count = count * 2 });
    stride += sizeof(float) * 2 * count;
}

template <>
inline void VertexBufferLayout::push<Vec3>(size_t count) {
    attributes.push_back({ .type = AttributeType::Float, .count = count * 3 });
    stride += sizeof(float) * 3 * count;
}

} // namespace Engine
