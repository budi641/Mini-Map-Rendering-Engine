#include "GPU/VertexArray.h"

namespace minimap::gpu {

VertexArray::VertexArray() { glGenVertexArrays(1, &id_); }

VertexArray::~VertexArray() {
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
    }
}

VertexArray::VertexArray(VertexArray&& other) noexcept : id_(other.id_) { other.id_ = 0; }

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
    }
    id_ = other.id_;
    other.id_ = 0;
    return *this;
}

void VertexArray::Bind() const { glBindVertexArray(id_); }

void VertexArray::Unbind() { glBindVertexArray(0); }

}  // namespace minimap::gpu
