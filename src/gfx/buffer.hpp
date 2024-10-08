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
    VertexBuffer();
    ~VertexBuffer();
    DELETE_COPY(VertexBuffer);
    DEFAULT_MOVE(VertexBuffer);
    
    void buffer(const void* data, size_t size);
    void bind() const;
    void unbind() const;
    bool isEmpty() const;

private:
    unsigned int vbo;
    bool empty = true;
};

enum class AttributeType : GLenum {
    UInt = GL_UNSIGNED_INT,
    Int = GL_INT,
    Float = GL_FLOAT,
    Vec2 = GL_FLOAT_VEC2,
    Vec3 = GL_FLOAT_VEC3,
    Mat4 = GL_FLOAT_MAT4,
};

struct AttributeDescriptor { 
    AttributeDescriptor(AttributeType type, size_t count)
        : type(type), count(count) {}

    inline AttributeType getType() const 
    { 
        switch (type) {
        case AttributeType::UInt: return AttributeType::UInt;
        case AttributeType::Int: return AttributeType::Int;
        case AttributeType::Float:
        case AttributeType::Vec2: 
        case AttributeType::Vec3: 
        case AttributeType::Mat4: return AttributeType::Float;
        default: return AttributeType::Float;
        }
    }

    inline size_t getCount() const 
    {
        switch (type) {
        case AttributeType::UInt: return count;
        case AttributeType::Int: return count;
        case AttributeType::Float: return count;
        case AttributeType::Vec2: return count * 2;
        case AttributeType::Vec3: return count * 3;
        case AttributeType::Mat4: return count * 16;
        default: return 0;
        }
    }

    inline static size_t getTypeSize(AttributeType type)
    {
        switch (type) {
        case AttributeType::UInt: return sizeof(GLuint);
        case AttributeType::Int: return sizeof(GLint);
        case AttributeType::Float: return sizeof(GLfloat);
        case AttributeType::Vec2: return sizeof(GLfloat) * 2;
        case AttributeType::Vec3: return sizeof(GLfloat) * 3;
        case AttributeType::Mat4: return sizeof(GLfloat) * 16;
        default: return 0;
        }
    } 
 
private:
    AttributeType type;
    size_t count; 
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

template<>
inline void VertexBufferLayout::push<GLuint>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::UInt, count));
    stride += sizeof(GLuint) * count;
}

template<>
inline void VertexBufferLayout::push<GLint>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::Int, count));
    stride += sizeof(GLint) * count;
}

template <>
inline void VertexBufferLayout::push<GLfloat>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::Float, count));
    stride += sizeof(GLfloat) * count;
}

template <>
inline void VertexBufferLayout::push<Vec2>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::Vec2, count));
    stride += sizeof(GLfloat) * 2 * count;
}

template <>
inline void VertexBufferLayout::push<Vec3>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::Vec3, count));
    stride += sizeof(GLfloat) * 3 * count;
}

template <>
inline void VertexBufferLayout::push<Mat4>(size_t count) {
    attributes.push_back(AttributeDescriptor(AttributeType::Mat4, count));
    stride += sizeof(GLfloat) * 16 * count;
}

} // namespace Engine
