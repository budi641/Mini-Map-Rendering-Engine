#pragma once

#include "GPU/GlHeaders.h"
#include "Math/Mat4.h"

#include <string>

namespace minimap::gpu {

class Shader {
public:
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Load(const std::string& vertexPath, const std::string& fragmentPath);
    void Use() const;
    void SetMat4(const char* name, const minimap::math::Mat4& matrix) const;
    void SetFloat(const char* name, float value) const;
    void SetInt(const char* name, int value) const;
    void SetVec4(const char* name, float x, float y, float z, float w) const;

private:
    static unsigned int Compile(unsigned int type, const std::string& source);
    static std::string ReadTextFile(const std::string& path);
    unsigned int program_ {0};
};

}  // namespace minimap::gpu
