#include "GPU/Buffer.h"

namespace minimap::gpu {

Buffer::Buffer(BufferType type) : type_(type) {
    glGenBuffers(1, &id_);
}

Buffer::~Buffer() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

Buffer::Buffer(Buffer&& other) noexcept : type_(other.type_), id_(other.id_) {
    other.id_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
    type_ = other.type_;
    id_ = other.id_;
    other.id_ = 0;
    return *this;
}

void Buffer::Bind() const {
    glBindBuffer(static_cast<unsigned int>(type_), id_);
}

void Buffer::Upload(const void* data, std::size_t bytes, unsigned int usage) const {
    Bind();
    glBufferData(static_cast<unsigned int>(type_), static_cast<GLsizeiptr>(bytes), data, usage);
}

void Buffer::UploadSubData(std::size_t offset, const void* data, std::size_t bytes) const {
    Bind();
    glBufferSubData(static_cast<unsigned int>(type_), static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(bytes), data);
}

}  // namespace minimap::gpu
