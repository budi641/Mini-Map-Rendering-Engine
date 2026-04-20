#pragma once

#include "GPU/GlHeaders.h"

#include <cstddef>

namespace minimap::gpu {

enum class BufferType : unsigned int {
    Vertex = GL_ARRAY_BUFFER,
    Index = GL_ELEMENT_ARRAY_BUFFER
};

class Buffer {
public:
    explicit Buffer(BufferType type);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    void Bind() const;
    void Upload(const void* data, std::size_t bytes, unsigned int usage) const;
    void UploadSubData(std::size_t offset, const void* data, std::size_t bytes) const;
    [[nodiscard]] unsigned int Id() const { return id_; }

private:
    BufferType type_;
    unsigned int id_ {0};
};

}  // namespace minimap::gpu
