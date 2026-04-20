#pragma once

#include "GPU/GlHeaders.h"

namespace minimap::gpu {

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void Bind() const;
    static void Unbind();
    [[nodiscard]] unsigned int Id() const { return id_; }

private:
    unsigned int id_ {0};
};

}  // namespace minimap::gpu
