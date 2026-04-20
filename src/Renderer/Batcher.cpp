#include "Renderer/Batcher.h"

namespace minimap::renderer {

void Batcher::Clear() {
    mesh_.vertices.clear();
    mesh_.indices.clear();
}

void Batcher::Append(const minimap::geometry::Mesh2D& mesh) {
    const std::uint32_t base = static_cast<std::uint32_t>(mesh_.vertices.size());
    mesh_.vertices.insert(mesh_.vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
    mesh_.indices.reserve(mesh_.indices.size() + mesh.indices.size());
    for (const std::uint32_t idx : mesh.indices) {
        mesh_.indices.push_back(base + idx);
    }
}

}  // namespace minimap::renderer
