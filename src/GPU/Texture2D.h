#pragma once

#include "GPU/GlHeaders.h"

namespace minimap::gpu {

class Texture2D {
public:
    Texture2D();
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void Bind(unsigned int slot = 0) const;
    void UploadRgba(unsigned int width, unsigned int height, const unsigned char* rgba);
    [[nodiscard]] unsigned int Id() const { return id_; }

private:
    unsigned int id_ {0};
};

}  // namespace minimap::gpu
