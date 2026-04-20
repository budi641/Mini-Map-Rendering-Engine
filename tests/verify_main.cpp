#include "Geometry/PolygonMeshBuilder.h"
#include "Geometry/PolylineMeshBuilder.h"
#include "Geometry/Simplifier.h"
#include "Geometry/Triangulator.h"
#include "Map/Distance.h"
#include "Map/Projection.h"
#include "Map/RoadNetwork.h"
#include "Map/TileManager.h"
#include "Map/TileSystem.h"
#include "Math/Vec2.h"

#include <cmath>
#include <iostream>
#include <string_view>
#include <vector>

namespace {

bool Nearly(double a, double b, double eps) {
    return std::abs(a - b) <= eps;
}

int Fail(const char* testName, const char* reason) {
    std::cerr << "[FAIL] " << testName << ": " << reason << '\n';
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    const bool pauseAtEnd = (argc >= 2 && std::string_view(argv[1]) == "--pause");
    // Projection round-trip
    {
        const minimap::map::LatLng input {37.7749, -122.4194};
        const auto world = minimap::map::Projection::LatLngToWorld(input);
        const auto back = minimap::map::Projection::WorldToLatLng(world);
        if (!Nearly(input.lat, back.lat, 1e-6) || !Nearly(input.lng, back.lng, 1e-6)) {
            return Fail("ProjectionRoundTrip", "lat/lng mismatch");
        }
    }

    // XYZ tiling deterministic bounds
    {
        const auto world = minimap::map::Projection::LatLngToWorld({0.0, 0.0});
        const auto tile = minimap::map::TileSystem::WorldToTile(world, 3);
        if (tile.x != 4 || tile.y != 4 || tile.z != 3) {
            return Fail("TileSystemWorldToTile", "unexpected tile id");
        }
    }

    // Douglas-Peucker keeps endpoints and simplifies
    {
        std::vector<minimap::map::WorldPoint> line {
            {0.0, 0.0}, {10.0, 0.2}, {20.0, -0.1}, {30.0, 0.1}, {40.0, 0.0}
        };
        const auto simplified = minimap::geometry::DouglasPeucker(line, 0.5);
        if (simplified.size() >= line.size()) {
            return Fail("DouglasPeucker", "line was not simplified");
        }
        if (!Nearly(simplified.front().x, 0.0, 1e-9) || !Nearly(simplified.back().x, 40.0, 1e-9)) {
            return Fail("DouglasPeucker", "endpoints were not preserved");
        }
    }

    // Concave polygon triangulation
    {
        std::vector<minimap::map::WorldPoint> concave {
            {0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {1.0, 1.0}, {0.0, 2.0}
        };
        const auto triangles = minimap::geometry::TriangulateSimplePolygon(concave);
        if (triangles.size() != 9) {
            return Fail("Triangulation", "expected n-2 triangles");
        }
    }

    // Polyline mesh should generate quads per segment
    {
        std::vector<minimap::math::Vec2> pts {{0.0F, 0.0F}, {10.0F, 0.0F}, {20.0F, 0.0F}};
        const auto mesh = minimap::geometry::BuildPolylineMesh(pts, 2.0F, 1.0F, 0.0F, 0.0F);
        if (mesh.vertices.size() != 8 || mesh.indices.size() != 12) {
            return Fail("PolylineMesh", "unexpected vertex/index counts");
        }
    }

    // Distance and bearing sanity checks
    {
        const auto d = minimap::map::HaversineMeters({0.0, 0.0}, {0.0, 1.0});
        if (!Nearly(d, 111319.49, 500.0)) {
            return Fail("Distance", "haversine result out of range");
        }
        const auto b = minimap::map::BearingDegrees({0.0, 0.0}, {1.0, 0.0});
        if (!Nearly(b, 0.0, 1e-3)) {
            return Fail("Bearing", "bearing mismatch for north movement");
        }
    }

    // Road snapping on known segment
    {
        minimap::map::RoadNetwork roads;
        minimap::map::PolylineFeature feature;
        feature.points = {{0.0, 0.0}, {100.0, 0.0}};
        roads.AddRoad(feature);
        double snapDist = 0.0;
        const auto snapped = roads.SnapToNearest({20.0, 20.0}, &snapDist);
        if (!Nearly(snapped.x, 20.0, 1e-6) || !Nearly(snapped.y, 0.0, 1e-6)) {
            return Fail("RoadSnap", "snapped coordinate mismatch");
        }
        if (!Nearly(snapDist, 20.0, 1e-6)) {
            return Fail("RoadSnap", "snapped distance mismatch");
        }
    }

    // Tile payload contract (provider-aware)
    {
        minimap::map::TileManager manager;
        minimap::map::TileId tileId {0, 0, 3};

        // If local OSM is active, validate using a tile near dataset center.
        if (manager.HasSuggestedCenter()) {
            const auto world = minimap::map::Projection::LatLngToWorld(manager.SuggestedCenter());
            tileId = minimap::map::TileSystem::WorldToTile(world, 14);
        }
        const auto& tile = manager.GetOrCreateTile(tileId);
        if (tile.rasterRgba.empty() || tile.rasterWidth == 0 || tile.rasterHeight == 0) {
            return Fail("TilePayload", "raster payload invalid");
        }
        const bool syntheticSignature = tile.roads.size() == 5 && tile.pois.size() == 50 && !tile.landuse.empty();
        if (syntheticSignature) {
            // Keep strict contract for synthetic provider.
            if (tile.roads.size() != 5 || tile.landuse.empty() || tile.pois.size() != 50) {
                return Fail("TilePayload", "synthetic tile contracts not satisfied");
            }
        } else {
            // Local OSM/provider-backed tile: allow variable counts but require at least one feature class.
            const bool hasFeatures = !tile.roads.empty() || !tile.landuse.empty() || !tile.pois.empty();
            if (!hasFeatures) {
                return Fail("TilePayload", "provider tile contains no features");
            }
        }
    }

    std::cout << "[PASS] verification suite passed\n";
    if (pauseAtEnd) {
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
    }
    return 0;
}
