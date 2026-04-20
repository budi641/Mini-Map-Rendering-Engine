#include "Renderer/RenderEngine.h"

#include "Geometry/PolygonMeshBuilder.h"
#include "Geometry/PolylineMeshBuilder.h"
#include "Geometry/Simplifier.h"
#include "Map/Distance.h"
#include "Map/Projection.h"
#include "Map/TileSystem.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace minimap::renderer {

namespace {
struct BlendWeights {
    float low {1.0F};
    float high {0.0F};
};

std::size_t HashTileId(const map::TileId& id) {
    const std::size_t z = static_cast<std::size_t>(id.z) & 0x3F;
    const std::size_t x = static_cast<std::size_t>(id.x) & 0x1FFFFFFF;
    const std::size_t y = static_cast<std::size_t>(id.y) & 0x1FFFFFFF;
    return (z << 58) | (x << 29) | y;
}

void AppendMesh(minimap::geometry::Mesh2D& dst, const minimap::geometry::Mesh2D& src) {
    const std::uint32_t indexOffset = static_cast<std::uint32_t>(dst.vertices.size());
    dst.vertices.insert(dst.vertices.end(), src.vertices.begin(), src.vertices.end());
    dst.indices.reserve(dst.indices.size() + src.indices.size());
    for (const auto idx : src.indices) {
        dst.indices.push_back(idx + indexOffset);
    }
}

BlendWeights ComputeZoomBlend(float zoomFraction) {
    constexpr float kBlendStart = 0.45F;
    constexpr float kBlendEnd = 0.55F;
    const float t = std::clamp(zoomFraction, 0.0F, 1.0F);
    if (t <= kBlendStart) {
        return {1.0F, 0.0F};
    }
    if (t >= kBlendEnd) {
        return {0.0F, 1.0F};
    }
    float u = (t - kBlendStart) / (kBlendEnd - kBlendStart);
    // Smoothstep for softer transition while keeping narrow overlap window.
    u = u * u * (3.0F - 2.0F * u);
    return {1.0F - u, u};
}

geometry::Mesh2D BuildLocalTileMesh(const map::TileData& tile, float ppm, float roadSimplifyEpsilon) {
    geometry::Mesh2D mesh;
    for (const auto& poly : tile.landuse) {
        auto polyMesh = geometry::BuildPolygonMesh(poly);
        for (auto& v : polyMesh.vertices) {
            v.a *= 0.55F;
        }
        AppendMesh(mesh, polyMesh);
    }
    for (const auto& road : tile.roads) {
        auto simplified = geometry::DouglasPeucker(road.points, roadSimplifyEpsilon);
        std::vector<math::Vec2> worldPts;
        worldPts.reserve(simplified.size());
        for (const auto& p : simplified) {
            worldPts.push_back({static_cast<float>(p.x), static_cast<float>(p.y)});
        }
        const float roadWidthWorld = road.widthPixels / ppm;
        auto roadMesh = geometry::BuildPolylineMesh(worldPts, roadWidthWorld, road.colorRg.x, road.colorRg.y, road.colorB);
        AppendMesh(mesh, roadMesh);
    }
    return mesh;
}
}  // namespace

void RenderEngine::Initialize(int width, int height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
    if (tiles_.HasSuggestedCenter()) {
        camera_.SetCenter(tiles_.SuggestedCenter());
    } else {
        camera_.SetCenter({37.7749, -122.4194});
    }
    camera_.SetZoom(14.0F);
    startupPrewarmFramesRemaining_ = 90;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderEngine::OnResize(int width, int height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
}

void RenderEngine::HandleInput(const core::InputState& input, float) {
    const float ppm = static_cast<float>(1.0 / map::TileSystem::ResolutionMetersPerPixel(camera_.Zoom()));
    if (input.leftMouseDown) {
        const math::Vec2 delta = input.MouseDelta();
        camera_.AddPanPixels(delta.x, delta.y, ppm);
    }
    if (std::abs(input.scrollDelta) > 0.0F) {
        camera_.AddZoomDelta(input.scrollDelta * 0.15F);
    }
    const bool rightPressed = input.rightMouseDown && !rightMouseLatch_;
    rightMouseLatch_ = input.rightMouseDown;
    if (rightPressed) {
        const map::WorldPoint clickWorld = map::Projection::ScreenToWorld(input.mousePosition, camera_.CenterWorld(), ppm, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_));
        double snapDist = 0.0;
        const auto snapped = roads_.SnapToNearest(clickWorld, &snapDist);
        const auto snappedLatLng = map::Projection::WorldToLatLng(snapped);
        if (!hasFirstPoint_) {
            hasFirstPoint_ = true;
            distanceA_ = snappedLatLng;
            distanceScreenA_ = map::Projection::WorldToScreen(snapped, camera_.CenterWorld(), ppm, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_));
        } else {
            distanceB_ = snappedLatLng;
            distanceScreenB_ = map::Projection::WorldToScreen(snapped, camera_.CenterWorld(), ppm, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_));
            distanceMeters_ = map::HaversineMeters(distanceA_, distanceB_);
            bearingDegrees_ = map::BearingDegrees(distanceA_, distanceB_);
            hasDistance_ = true;
            hasFirstPoint_ = false;
        }
    }
}

void RenderEngine::Update(float dtSeconds) {
    camera_.Update(dtSeconds);
    RebuildScene();
    if (startupPrewarmFramesRemaining_ > 0) {
        --startupPrewarmFramesRemaining_;
    }
}

void RenderEngine::Render() {
    glClearColor(0.97F, 0.97F, 0.98F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto worldToClip = BuildWorldToClip();
    const auto screenToClip = BuildScreenToClip();
    tileRenderer_.Draw(worldToClip);

    if (hasDistance_) {
        uiRenderer_.DrawMeasurementLine(screenToClip, distanceScreenA_, distanceScreenB_);
    }
    uiRenderer_.DrawCompass(screenToClip, static_cast<float>(bearingDegrees_), {static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_)});
}

void RenderEngine::RebuildScene() {
    batcher_.Clear();
    roads_.Clear();

    const float zoomFloat = camera_.Zoom();
    const int zoomFloorRaw = static_cast<int>(std::floor(zoomFloat));
    const int zoomLow = std::clamp(zoomFloorRaw, 3, 18);
    const int zoomHigh = std::clamp(zoomLow + 1, 3, 18);
    const float zoomT = std::clamp(zoomFloat - static_cast<float>(zoomFloorRaw), 0.0F, 1.0F);
    const auto blend = ComputeZoomBlend(zoomT);
    const char* vectorSource = std::getenv("MINIMAP_VECTOR_SOURCE");
    const bool localOsmMode = (vectorSource != nullptr) && (std::string(vectorSource) == "local_osm");
    int layerLowZoom = zoomLow;
    int layerHighZoom = zoomHigh;
    float lowAlpha = blend.low;
    float highAlpha = (zoomHigh == zoomLow) ? 0.0F : blend.high * 0.75F;
    if (localOsmMode) {
        // Hysteresis prevents rapid zoom-layer toggling near half-step boundaries.
        if (localStableZoom_ < 0) {
            localStableZoom_ = std::clamp(static_cast<int>(std::round(zoomFloat)), 3, 18);
        }
        if (zoomFloat > static_cast<float>(localStableZoom_) + 0.65F && localStableZoom_ < 18) {
            ++localStableZoom_;
        } else if (zoomFloat < static_cast<float>(localStableZoom_) - 0.65F && localStableZoom_ > 3) {
            --localStableZoom_;
        }
        layerLowZoom = localStableZoom_;
        layerHighZoom = localStableZoom_;
        lowAlpha = 1.0F;
        highAlpha = 0.0F;
    } else {
        localStableZoom_ = -1;
    }
    // At zoom limits both layers map to the same tile level; keep one layer fully visible.
    if (layerHighZoom == layerLowZoom) {
        lowAlpha = 1.0F;
        highAlpha = 0.0F;
    }
    // Raster crossfade uses lighter secondary contribution to avoid washed-out look.
    const float lowRasterAlpha = lowAlpha;
    const float highRasterAlpha = highAlpha * 0.35F;

    const float ppm = static_cast<float>(1.0 / map::TileSystem::ResolutionMetersPerPixel(camera_.Zoom()));
    std::vector<PoiInstance> poiInstances;
    std::vector<RasterQuad> rasterQuads;
    int visibleCount = 0;
    constexpr float kLocalRoadSimplifyEpsilon = 0.75F;
    constexpr int kLocalStartupPrewarmBudgetPerFrame = 28;
    constexpr int kLocalWarmupBuildBudgetPerFrame = 16;
    constexpr int kLocalSteadyBuildBudgetPerFrame = 6;
    int localTileBuildBudgetPerFrame = kLocalSteadyBuildBudgetPerFrame;
    if (startupPrewarmFramesRemaining_ > 0) {
        localTileBuildBudgetPerFrame = kLocalStartupPrewarmBudgetPerFrame;
    } else if (localTileMeshCache_.size() < 64) {
        localTileBuildBudgetPerFrame = kLocalWarmupBuildBudgetPerFrame;
    }
    int localTilesBuiltThisFrame = 0;

    auto appendLayer = [&](int layerZoom, float layerAlpha, float rasterLayerAlpha, bool forSnapping) {
        if (layerAlpha <= 0.001F) {
            return;
        }
        const auto visible = map::TileSystem::VisibleTiles(camera_.CenterWorld(), layerZoom, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_), ppm);
        visibleCount += static_cast<int>(visible.size());
        poiInstances.reserve(poiInstances.size() + visible.size() * 50);
        rasterQuads.reserve(rasterQuads.size() + visible.size());

        for (const auto& tileId : visible) {
            const auto& tile = tiles_.GetOrCreateTile(tileId);
            if (!tile.rasterRgba.empty()) {
                rasterQuads.push_back({tileId, map::TileSystem::TileBoundsWorld(tileId), rasterLayerAlpha, &tile});
            }

            if (localOsmMode && std::abs(layerAlpha - 1.0F) < 0.001F) {
                const auto key = HashTileId(tileId);
                auto meshIt = localTileMeshCache_.find(key);
                if (meshIt == localTileMeshCache_.end()) {
                    if (localTilesBuiltThisFrame < localTileBuildBudgetPerFrame) {
                        auto cachedMesh = BuildLocalTileMesh(tile, ppm, kLocalRoadSimplifyEpsilon);
                        meshIt = localTileMeshCache_.emplace(key, std::move(cachedMesh)).first;
                        ++localTilesBuiltThisFrame;
                    } else {
                        // Fallback mesh keeps coverage stable when cache budget is exhausted this frame.
                        // Use the same simplification epsilon as cached mesh to prevent early-frame shape jitter.
                        const auto transient = BuildLocalTileMesh(tile, ppm, kLocalRoadSimplifyEpsilon);
                        batcher_.Append(transient);
                    }
                }
                if (forSnapping) {
                    for (const auto& road : tile.roads) {
                        roads_.AddRoad(road);
                    }
                }
                if (meshIt != localTileMeshCache_.end()) {
                    batcher_.Append(meshIt->second);
                }
            } else {
                for (const auto& poly : tile.landuse) {
                    auto polyMesh = geometry::BuildPolygonMesh(poly);
                    const float polygonAlphaScale = 1.0F;
                    for (auto& v : polyMesh.vertices) {
                        v.a *= (layerAlpha * polygonAlphaScale);
                    }
                    batcher_.Append(polyMesh);
                }
                for (const auto& road : tile.roads) {
                    if (forSnapping) {
                        roads_.AddRoad(road);
                    }
                    auto simplified = geometry::DouglasPeucker(road.points, map::TileSystem::ResolutionMetersPerPixel(camera_.Zoom()) * 1.5);
                    std::vector<math::Vec2> worldPts;
                    worldPts.reserve(simplified.size());
                    for (const auto& p : simplified) {
                        worldPts.push_back({static_cast<float>(p.x), static_cast<float>(p.y)});
                    }
                    const float roadWidthWorld = road.widthPixels / ppm;
                    auto roadMesh = geometry::BuildPolylineMesh(worldPts, roadWidthWorld, road.colorRg.x, road.colorRg.y, road.colorB);
                    for (auto& v : roadMesh.vertices) {
                        v.a *= layerAlpha;
                    }
                    batcher_.Append(roadMesh);
                }
            }
            for (const auto& poi : tile.pois) {
                poiInstances.push_back({
                    static_cast<float>(poi.position.x),
                    static_cast<float>(poi.position.y),
                    poi.sizePixels / ppm,
                    poi.headingDegrees,
                    layerAlpha
                });
            }
        }
    };

    appendLayer(layerLowZoom, lowAlpha, lowRasterAlpha, true);
    appendLayer(layerHighZoom, highAlpha, highRasterAlpha, false);
    lastVisibleTileCount_ = visibleCount;

    tileRenderer_.UploadGeometry(batcher_.Mesh(), poiInstances);
    tileRenderer_.UploadRasterQuads(rasterQuads);
    lastVertexCount_ = batcher_.Mesh().vertices.size();
    lastIndexCount_ = batcher_.Mesh().indices.size();
    lastPoiInstanceCount_ = poiInstances.size();
}

math::Mat4 RenderEngine::BuildWorldToClip() const {
    const float ppm = static_cast<float>(1.0 / map::TileSystem::ResolutionMetersPerPixel(camera_.Zoom()));
    const auto screenOrtho = math::Mat4::Ortho(0.0F, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_), 0.0F, -1.0F, 1.0F);
    const auto worldToScreen = math::Mat4::Translate(static_cast<float>(viewportWidth_) * 0.5F, static_cast<float>(viewportHeight_) * 0.5F, 0.0F) *
                               math::Mat4::Scale(ppm, -ppm, 1.0F) *
                               math::Mat4::Translate(-static_cast<float>(camera_.CenterWorld().x), -static_cast<float>(camera_.CenterWorld().y), 0.0F);
    return screenOrtho * worldToScreen;
}

math::Mat4 RenderEngine::BuildScreenToClip() const {
    return math::Mat4::Ortho(0.0F, static_cast<float>(viewportWidth_), static_cast<float>(viewportHeight_), 0.0F, -1.0F, 1.0F);
}

}  // namespace minimap::renderer
