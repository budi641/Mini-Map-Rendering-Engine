#pragma once

#include "Core/InputState.h"
#include "Map/Camera.h"
#include "Map/RoadNetwork.h"
#include "Map/TileManager.h"
#include "Math/Mat4.h"
#include "Renderer/Batcher.h"
#include "Renderer/TileRenderer.h"
#include "Renderer/UiRenderer.h"

#include <unordered_map>

namespace minimap::renderer {

class RenderEngine {
public:
    void Initialize(int width, int height);
    void OnResize(int width, int height);
    void HandleInput(const core::InputState& input, float dtSeconds);
    void Update(float dtSeconds);
    void Render();

    [[nodiscard]] bool HasDistanceMeasurement() const { return hasDistance_; }
    [[nodiscard]] double DistanceMeters() const { return distanceMeters_; }
    [[nodiscard]] double BearingDegrees() const { return bearingDegrees_; }
    [[nodiscard]] int LastVisibleTileCount() const { return lastVisibleTileCount_; }
    [[nodiscard]] std::size_t LastVertexCount() const { return lastVertexCount_; }
    [[nodiscard]] std::size_t LastIndexCount() const { return lastIndexCount_; }
    [[nodiscard]] std::size_t LastPoiInstanceCount() const { return lastPoiInstanceCount_; }

private:
    void RebuildScene();
    minimap::math::Mat4 BuildWorldToClip() const;
    minimap::math::Mat4 BuildScreenToClip() const;

    int viewportWidth_ {1280};
    int viewportHeight_ {720};
    map::Camera camera_ {};
    map::TileManager tiles_ {};
    map::RoadNetwork roads_ {};
    Batcher batcher_ {};
    TileRenderer tileRenderer_ {};
    UiRenderer uiRenderer_ {};

    bool hasDistance_ {false};
    map::LatLng distanceA_ {};
    map::LatLng distanceB_ {};
    math::Vec2 distanceScreenA_ {};
    math::Vec2 distanceScreenB_ {};
    double distanceMeters_ {0.0};
    double bearingDegrees_ {0.0};
    bool hasFirstPoint_ {false};
    bool rightMouseLatch_ {false};
    int lastVisibleTileCount_ {0};
    std::size_t lastVertexCount_ {0};
    std::size_t lastIndexCount_ {0};
    std::size_t lastPoiInstanceCount_ {0};
    int startupPrewarmFramesRemaining_ {0};
    int localStableZoom_ {-1};
    std::unordered_map<std::size_t, geometry::Mesh2D> localTileMeshCache_ {};
};

}  // namespace minimap::renderer
