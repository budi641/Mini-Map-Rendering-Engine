#include "GPU/Shader.h"

#include "Utils/Logger.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace minimap::gpu {

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    Load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

Shader::Shader(Shader&& other) noexcept : program_(other.program_) {
    other.program_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
    program_ = other.program_;
    other.program_ = 0;
    return *this;
}

void Shader::Load(const std::string& vertexPath, const std::string& fragmentPath) {
    const auto vertexSource = ReadTextFile(vertexPath);
    const auto fragmentSource = ReadTextFile(fragmentPath);
    const unsigned int vert = Compile(GL_VERTEX_SHADER, vertexSource);
    const unsigned int frag = Compile(GL_FRAGMENT_SHADER, fragmentSource);

    if (program_ != 0) {
        glDeleteProgram(program_);
    }
    program_ = glCreateProgram();
    glAttachShader(program_, vert);
    glAttachShader(program_, frag);
    glLinkProgram(program_);
    glDeleteShader(vert);
    glDeleteShader(frag);

    int linked = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (linked == 0) {
        char buffer[1024] {};
        glGetProgramInfoLog(program_, static_cast<GLsizei>(sizeof(buffer)), nullptr, buffer);
        throw std::runtime_error(std::string("Shader link failed: ") + buffer);
    }
}

void Shader::Use() const { glUseProgram(program_); }

void Shader::SetMat4(const char* name, const minimap::math::Mat4& matrix) const {
    const int location = glGetUniformLocation(program_, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
}

void Shader::SetFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(program_, name), value);
}

void Shader::SetInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program_, name), value);
}

void Shader::SetVec4(const char* name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(program_, name), x, y, z, w);
}

unsigned int Shader::Compile(unsigned int type, const std::string& source) {
    const unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == 0) {
        char buffer[1024] {};
        glGetShaderInfoLog(shader, static_cast<GLsizei>(sizeof(buffer)), nullptr, buffer);
        throw std::runtime_error(std::string("Shader compile failed: ") + buffer);
    }
    return shader;
}

std::string Shader::ReadTextFile(const std::string& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        throw std::runtime_error("Could not open shader: " + path);
    }
    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

}  // namespace minimap::gpu
