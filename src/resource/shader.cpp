#include <pch.hpp>

#include "shader.hpp"
#include "../gfx/render_backend.hpp"
#include "../gfx/opengl.hpp"
#include "gfx/buffer.hpp"
#include <fstream>
#include <sstream>

namespace Engine {

Shader::Shader(const std::filesystem::path& path)
{
    Log::info("Attempting to load shader \"{}\"", path.string());

    std::ifstream file(path);
    if (!file) {
        Log::error("Shader \"{}\" could not be found or opened", path.string());
        return;
    }
    
    std::stringstream buf;
    buf << file.rdbuf();

    std::string full_shader = buf.str();

    Shader::ShaderData data;
    if (auto maybe_data = preProcessShader(full_shader)) {
        data = *maybe_data;
    } else {
        Log::error("Failed to process shader \"{}\"", path.string());
        return;
    }

    const char* vert_cstr = data.vert.c_str();
    const char* frag_cstr = data.frag.c_str();

    Log::debug("Vert for \"{}\" -> \n{}", path.string(), vert_cstr);
    Log::debug("Frag for \"{}\" -> \n{}", path.string(), frag_cstr);

    const GLuint vert = OPENGL_CALL(glCreateShader(GL_VERTEX_SHADER));
    const GLuint frag = OPENGL_CALL(glCreateShader(GL_FRAGMENT_SHADER));

    OPENGL_CALL(glShaderSource(vert, 1, &vert_cstr, nullptr));
    OPENGL_CALL(glShaderSource(frag, 1, &frag_cstr, nullptr));

    OPENGL_CALL(glCompileShader(vert));
    OPENGL_CALL(glCompileShader(frag));

    int compiled = 0;
    char log[512];
    OPENGL_CALL(glGetShaderiv(vert, GL_COMPILE_STATUS, &compiled));
    if (!compiled) {
        OPENGL_CALL(glGetShaderInfoLog(vert, 512, nullptr, log));
        std::string_view safe_log = log;
        Log::error("Failed to compile vertex shader for \"{}\" -> \"{}\"", path.string(), safe_log);
        return;
    }
    
    OPENGL_CALL(glGetShaderiv(frag, GL_COMPILE_STATUS, &compiled));
    if (!compiled) {
        OPENGL_CALL(glGetShaderInfoLog(frag, 512, nullptr, log));
        std::string_view safe_log = log;
        Log::error("Failed to compile fragment shader for \"{}\" -> \"{}\"", path.string(), safe_log);
        OPENGL_CALL(glDeleteShader(vert));
        return;
    }
    
    program = OPENGL_CALL(glCreateProgram());
    OPENGL_CALL(glAttachShader(program, vert));
    OPENGL_CALL(glAttachShader(program, frag));
    OPENGL_CALL(glLinkProgram(program));

    OPENGL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &compiled));
    if (!compiled) {
        OPENGL_CALL(glGetProgramInfoLog(program, 512, nullptr, log));
        std::string_view safe_log = log;
        Log::error("Failed to link shader program for \"{}\" -> \"{}\"", path.string(), safe_log);
        return;
    }

    OPENGL_CALL(glDetachShader(program, vert));
    OPENGL_CALL(glDetachShader(program, frag));

    OPENGL_CALL(glDeleteShader(vert));
    OPENGL_CALL(glDeleteShader(frag));

    Log::info("Successfully loaded shader \"{}\"", path.string());
}

Shader::~Shader()
{
    if (program != 0) {
        OPENGL_CALL(glDeleteProgram(program));
    }
}

std::optional<Shader::ShaderData> Shader::preProcessShader(const std::string& file)
{
    constexpr auto vert_pattern = ctll::fixed_string{"#section\\s+vert\\s*\n((?:(?!#section).)*)"};
    constexpr auto frag_pattern = ctll::fixed_string{"#section\\s+frag\\s*\n((?:(?!#section).)*)"};

    std::optional<std::string> vert = std::nullopt;
    std::optional<std::string> frag = std::nullopt;

    // Match vertex shader section
    if (auto [whole, content] = ctre::search<vert_pattern>(file); whole) {
        vert = content;
    }

    // Match fragment shader section
    if (auto [whole, content] = ctre::search<frag_pattern>(file); whole) {
        frag = content;
    }

    if (!vert || !frag) {
        return std::nullopt;
    }
    
    auto data = Shader::ShaderData {
        .vert = *vert,
        .frag = *frag,
    };

    std::string header = "#version " + std::to_string(OPENGL_MAJOR_VERSION) + std::to_string(OPENGL_MINOR_VERSION) + "0 core\n\n";
    data.vert = data.vert.insert(0, header);
    data.frag = data.frag.insert(0, header);

    return data;
}

void Shader::use() const
{
    OPENGL_CALL(glUseProgram(program));
}

void Shader::setUniform(const std::string& name, float value) const
{
    GLint location = OPENGL_CALL(glGetUniformLocation(program, name.c_str()));
    OPENGL_CALL(glUniform1f(location, value)); 
}

void Shader::setUniform(const std::string& name, Vec2 value) const
{
    GLint location = OPENGL_CALL(glGetUniformLocation(program, name.c_str()));
    OPENGL_CALL(glUniform2f(location, value.getX(), value.getY())); 

}

void Shader::setUniform(const std::string& name, const Vec3& value) const
{
    GLint location = OPENGL_CALL(glGetUniformLocation(program, name.c_str()));
    OPENGL_CALL(glUniform3f(location, value.getX(), value.getY(), value.getZ())); 
}

void Shader::setUniform(const std::string& name, const Mat4& value) const
{
    GLint location = OPENGL_CALL(glGetUniformLocation(program, name.c_str()));
    OPENGL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, value.getValues())); 
}

VertexBufferLayout Shader::getUniformLayout() const
{
    VertexBufferLayout layout;

    int attrib_count;
    OPENGL_CALL(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attrib_count));
    Log::debug("Attrib count {}", attrib_count);

    for (int i = 0; i < attrib_count; i++) {
        char name[256];
        GLsizei length;
        int size;
        unsigned int type;
        OPENGL_CALL(glGetActiveAttrib(program, i, sizeof(name), &length, &size, &type, name));
        Log::debug("Attrib {} of size {}", name, size);

        auto attrib_type = static_cast<AttributeType>(type);
        switch (attrib_type) {
        case AttributeType::UInt: layout.push<GLuint>(static_cast<size_t>(size)); break;
        case AttributeType::Int: layout.push<GLint>(static_cast<size_t>(size)); break;
        case AttributeType::Float: layout.push<GLfloat>(static_cast<size_t>(size)); break;
        case AttributeType::Vec2: layout.push<Vec2>(static_cast<size_t>(size)); break;
        case AttributeType::Vec3: layout.push<Vec3>(static_cast<size_t>(size)); break;
        case AttributeType::Mat4: layout.push<Mat4>(static_cast<size_t>(size)); break;
        default: Log::error("Unhandled OpenGL attribute type {}", type); break;
        }
    }

    return layout;
}

} // namespace Engine
