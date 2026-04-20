#include "GPU/Texture2D.h"

namespace minimap::gpu {

Texture2D::Texture2D() {
    glGenTextures(1, &id_);
}

Texture2D::~Texture2D() {
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
}

Texture2D::Texture2D(Texture2D&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (id_ != 0) {
        glDeleteTextures(1, &id_);
    }
    id_ = other.id_;
    other.id_ = 0;
    return *this;
}

void Texture2D::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture2D::UploadRgba(unsigned int width, unsigned int height, const unsigned char* rgba) {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
}

}  // namespace minimap::gpu
