#pragma once

#include "../constructors.hpp"
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
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
    Float,
    Vec2,
    Vec3,
    Mat4,
};

struct AttributeDescriptor {
    AttributeType type;
    size_t count;

    inline static unsigned int getTypeGLValue(AttributeType type)
    {
        switch (type) {
        case AttributeType::Float: return GL_FLOAT;
        case AttributeType::Vec2: return GL_FLOAT * 2;
        case AttributeType::Vec3: return GL_FLOAT * 3;
        case AttributeType::Mat4: return GL_FLOAT * 16;
        default: return 0;
        }
    }

    inline static size_t getTypeSize(AttributeType type)
    {
        switch (type) {
        case AttributeType::Float: return sizeof(float);
        case AttributeType::Vec2: return sizeof(float) * 2;
        case AttributeType::Vec3: return sizeof(float) * 3;
        case AttributeType::Mat4: return sizeof(float) * 16;
        default: return 0;
        }
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
    attributes.push_back({ .type = AttributeType::Vec2, .count = count });
    stride += sizeof(float) * 2 * count;
}

template <>
inline void VertexBufferLayout::push<Vec3>(size_t count) {
    attributes.push_back({ .type = AttributeType::Vec3, .count = count });
    stride += sizeof(float) * 3 * count;
}

template <>
inline void VertexBufferLayout::push<Mat4>(size_t count) {
    attributes.push_back({ .type = AttributeType::Mat4, .count = count });
    stride += sizeof(float) * 16 * count;
}

} // namespace Engine
