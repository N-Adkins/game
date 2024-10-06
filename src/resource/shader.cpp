#include <pch.hpp>

#include "shader.hpp"
#include "../gfx/render_backend.hpp"
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

    const GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    const GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert, 1, &vert_cstr, nullptr);
    glShaderSource(frag, 1, &frag_cstr, nullptr);

    glCompileShader(vert);
    glCompileShader(frag);

    int compiled = 0;
    char log[512];
    glGetShaderiv(vert, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        glGetShaderInfoLog(vert, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to compile vertex shader for \"{}\" -> \"{}\"", path.string(), safe_log);
        return;
    }
    
    glGetShaderiv(frag, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        glGetShaderInfoLog(frag, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to compile fragment shader for \"{}\" -> \"{}\"", path.string(), safe_log);
        glDeleteShader(vert);
        return;
    }
    
    const GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &compiled);
    if (!compiled) {
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::string_view safe_log = log;
        Log::error("Failed to link shader program for \"{}\" -> \"{}\"", path.string(), safe_log);
        return;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    Log::info("Successfully loaded shader \"{}\"", path.string());
}

Shader::~Shader()
{
#if defined(GAME_RENDER_BACKEND_OPENGL)
    if (program != 0) {
        glDeleteProgram(program);
    }
#endif
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

    std::string header = "#version " + std::to_string(OPENGL_MAJOR_VERSION) + std::to_string(OPENGL_MINOR_VERSION) + "0\n\n";
    data.vert = data.vert.insert(0, header);
    data.frag = data.frag.insert(0, header);

    return data;
}

VertexBufferLayout Shader::getUniformLayout() const
{
    VertexBufferLayout layout;

    int max_length;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);

    int uniform_count;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

    for (int i = 0; i < uniform_count; i++) {
        int size;
        unsigned int type;
        glGetActiveUniform(program, i, max_length, nullptr, &size, &type, nullptr);

        auto attrib_type = static_cast<AttributeType>(type);
        switch (attrib_type) {
        case AttributeType::Float: layout.push<float>(static_cast<size_t>(size)); break;
        default: Log::error("Unhandled OpenGL attribute type {}", type); break;
        }
    }

    return layout;
}

} // namespace Engine
