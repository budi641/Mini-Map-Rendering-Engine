#include "Map/TileManager.h"

#include "Map/Projection.h"
#include "Map/TileSystem.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <limits>
#include <string>
#include <thread>
#include <utility>
#include <unordered_map>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace minimap::map {

namespace {

std::string GetEnv(const char* key, const char* defaultValue) {
    const char* value = std::getenv(key);
    if (value == nullptr || value[0] == '\0') {
        return std::string(defaultValue);
    }
    return std::string(value);
}

std::string ReplaceAll(std::string input, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while ((pos = input.find(from, pos)) != std::string::npos) {
        input.replace(pos, from.size(), to);
        pos += to.size();
    }
    return input;
}

std::string Attr(const std::string& line, const char* key) {
    const std::string needle = std::string(key) + "=\"";
    const std::size_t start = line.find(needle);
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t valueStart = start + needle.size();
    const std::size_t valueEnd = line.find('"', valueStart);
    if (valueEnd == std::string::npos) {
        return {};
    }
    return line.substr(valueStart, valueEnd - valueStart);
}

bool HasTag(const std::unordered_map<std::string, std::string>& tags, const char* key) {
    return tags.find(key) != tags.end();
}

bool IsPOITagKey(const std::string& key) {
    return key == "amenity" || key == "shop" || key == "tourism" || key == "historic";
}

bool AabbIntersects(const Aabb& a, const Aabb& b) {
    return !(a.max.x < b.min.x || a.min.x > b.max.x || a.max.y < b.min.y || a.min.y > b.max.y);
}

Aabb ComputeBounds(const std::vector<WorldPoint>& points) {
    Aabb bounds {{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
                 {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()}};
    for (const auto& p : points) {
        bounds.min.x = std::min(bounds.min.x, static_cast<float>(p.x));
        bounds.min.y = std::min(bounds.min.y, static_cast<float>(p.y));
        bounds.max.x = std::max(bounds.max.x, static_cast<float>(p.x));
        bounds.max.y = std::max(bounds.max.y, static_cast<float>(p.y));
    }
    return bounds;
}

std::vector<unsigned char> BuildCheckerRaster(unsigned int width, unsigned int height) {
    std::vector<unsigned char> rgba(width * height * 4);
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const bool checker = ((x / 4 + y / 4) % 2) == 0;
            const std::size_t idx = static_cast<std::size_t>(y * width + x) * 4;
            rgba[idx + 0] = checker ? 226 : 212;
            rgba[idx + 1] = checker ? 228 : 216;
            rgba[idx + 2] = checker ? 230 : 214;
            rgba[idx + 3] = 255;
        }
    }
    return rgba;
}

bool InExpandedBounds(const WorldPoint& p, const Aabb& b, float pad) {
    return p.x >= static_cast<double>(b.min.x - pad) && p.x <= static_cast<double>(b.max.x + pad) &&
           p.y >= static_cast<double>(b.min.y - pad) && p.y <= static_cast<double>(b.max.y + pad);
}

std::vector<WorldPoint> ClipPolylineLoose(const std::vector<WorldPoint>& points, const Aabb& tileBounds, float pad) {
    if (points.size() < 2) {
        return {};
    }
    std::vector<WorldPoint> out;
    out.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
        const bool keepCurr = InExpandedBounds(points[i], tileBounds, pad);
        const bool keepPrev = (i > 0) && InExpandedBounds(points[i - 1], tileBounds, pad);
        const bool keepNext = (i + 1 < points.size()) && InExpandedBounds(points[i + 1], tileBounds, pad);
        if (keepCurr || keepPrev || keepNext) {
            out.push_back(points[i]);
        }
    }
    return out;
}

std::vector<WorldPoint> DecimatePoints(const std::vector<WorldPoint>& points, std::size_t maxPoints, bool closed) {
    if (points.size() <= maxPoints || maxPoints < 4) {
        return points;
    }
    const std::size_t coreCount = closed ? points.size() - 1 : points.size();
    const std::size_t step = std::max<std::size_t>(1, coreCount / maxPoints);
    std::vector<WorldPoint> out;
    out.reserve(maxPoints + 2);
    for (std::size_t i = 0; i < coreCount; i += step) {
        out.push_back(points[i]);
    }
    if (out.back().x != points[coreCount - 1].x || out.back().y != points[coreCount - 1].y) {
        out.push_back(points[coreCount - 1]);
    }
    if (closed) {
        out.push_back(out.front());
    }
    return out;
}

}  // namespace

struct TileManager::OSMDataset {
    struct Road {
        std::vector<WorldPoint> points;
        Aabb bounds {};
    };
    struct Polygon {
        std::vector<WorldPoint> points;
        Aabb bounds {};
    };
    struct Poi {
        WorldPoint point {};
    };

    std::vector<Road> roads;
    std::vector<Polygon> polygons;
    std::vector<Poi> pois;
    bool hasBoundsCenter {false};
    LatLng boundsCenter {};
};

struct TileManager::DownloadState {
    std::mutex mutex;
    std::unordered_set<std::string> inFlight;
};

TileManager::TileManager() {
    const auto vectorSource = GetEnv("MINIMAP_VECTOR_SOURCE", "auto");
    osmFilePath_ = GetEnv("MINIMAP_OSM_FILE", "maps/map.osm");
    if (vectorSource == "local_osm") {
        vectorSource_ = VectorSource::LocalOSMFile;
    } else if (vectorSource == "synthetic") {
        vectorSource_ = VectorSource::Synthetic;
    } else if (std::filesystem::exists(osmFilePath_)) {
        vectorSource_ = VectorSource::LocalOSMFile;
    }

    tileCacheDir_ = GetEnv("MINIMAP_TILE_CACHE", ".tile_cache");
    osmUrlTemplate_ = GetEnv("MINIMAP_OSM_URL", "https://tile.openstreetmap.org/{z}/{x}/{y}.png");
    std::filesystem::create_directories(tileCacheDir_);
    downloadState_ = std::make_shared<DownloadState>();

    if (vectorSource_ == VectorSource::LocalOSMFile) {
        localOSMReady_ = LoadLocalOSMData(osmFilePath_);
        if (!localOSMReady_) {
            vectorSource_ = VectorSource::Synthetic;
        }
    }

    // Raster source selection supports:
    // - synthetic: always checker fallback
    // - osm: always try OSM raster
    // - auto: use OSM raster when local OSM vectors are active, synthetic otherwise
    const auto source = GetEnv("MINIMAP_RASTER_SOURCE", "auto");
    if (source == "osm") {
        rasterSource_ = RasterSource::OpenStreetMap;
    } else if (source == "synthetic") {
        rasterSource_ = RasterSource::Synthetic;
    } else {
        rasterSource_ = (vectorSource_ == VectorSource::LocalOSMFile) ? RasterSource::OpenStreetMap : RasterSource::Synthetic;
    }
}

TileManager::~TileManager() = default;

const TileData& TileManager::GetOrCreateTile(const TileId& tileId) {
    const auto key = HashTile(tileId);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        if (rasterSource_ == RasterSource::OpenStreetMap && it->second.rasterRgba.empty()) {
            LoadOpenStreetMapRaster(tileId, &it->second);
        } else if (rasterSource_ == RasterSource::Synthetic && it->second.rasterRgba.empty()) {
            it->second.rasterWidth = 32;
            it->second.rasterHeight = 32;
            it->second.rasterRgba = BuildCheckerRaster(it->second.rasterWidth, it->second.rasterHeight);
        }
        return it->second;
    }
    TileData tile = (vectorSource_ == VectorSource::LocalOSMFile) ? BuildTileFromLocalOSM(tileId) : BuildSyntheticTile(tileId);
    if (rasterSource_ == RasterSource::OpenStreetMap) {
        LoadOpenStreetMapRaster(tileId, &tile);
    } else if (tile.rasterRgba.empty()) {
        tile.rasterWidth = 32;
        tile.rasterHeight = 32;
        tile.rasterRgba = BuildCheckerRaster(tile.rasterWidth, tile.rasterHeight);
    }
    auto inserted = cache_.emplace(key, std::move(tile));
    return inserted.first->second;
}

bool TileManager::HasSuggestedCenter() const {
    return localOSMReady_ && osmDataset_ != nullptr && osmDataset_->hasBoundsCenter;
}

LatLng TileManager::SuggestedCenter() const {
    if (!HasSuggestedCenter()) {
        return {};
    }
    return osmDataset_->boundsCenter;
}

TileData TileManager::BuildSyntheticTile(const TileId& tileId) {
    TileData data;
    data.id = tileId;

    const Aabb bounds = TileSystem::TileBoundsWorld(tileId);
    const double dx = bounds.max.x - bounds.min.x;
    const double dy = bounds.max.y - bounds.min.y;

    for (int i = 1; i <= 5; ++i) {
        const double y = bounds.min.y + dy * (static_cast<double>(i) / 6.0);
        PolylineFeature road;
        road.widthPixels = 2.0F + (tileId.z % 3);
        road.colorRg = {0.2F, 0.2F};
        road.colorB = 0.25F;
        road.points.push_back({bounds.min.x, y + std::sin(i + tileId.x * 0.2) * dy * 0.05});
        road.points.push_back({bounds.max.x, y + std::cos(i + tileId.y * 0.2) * dy * 0.05});
        data.roads.push_back(road);
    }

    PolygonFeature park;
    park.colorRg = {0.2F, 0.6F};
    park.colorB = 0.25F;
    park.outer = {
        {bounds.min.x + dx * 0.25, bounds.min.y + dy * 0.25},
        {bounds.min.x + dx * 0.45, bounds.min.y + dy * 0.2},
        {bounds.min.x + dx * 0.55, bounds.min.y + dy * 0.42},
        {bounds.min.x + dx * 0.35, bounds.min.y + dy * 0.55},
        {bounds.min.x + dx * 0.2, bounds.min.y + dy * 0.4}
    };
    data.landuse.push_back(park);

    for (int i = 0; i < 50; ++i) {
        const double fx = (i % 10) / 10.0;
        const double fy = (i / 10) / 5.0;
        data.pois.push_back({
            {bounds.min.x + dx * (0.1 + 0.8 * fx), bounds.min.y + dy * (0.1 + 0.8 * fy)},
            5.0F,
            static_cast<float>((i * 17) % 360)
        });
    }

    data.rasterWidth = 32;
    data.rasterHeight = 32;
    data.rasterRgba = BuildCheckerRaster(data.rasterWidth, data.rasterHeight);

    return data;
}

TileData TileManager::BuildTileFromLocalOSM(const TileId& tileId) const {
    TileData data;
    data.id = tileId;
    data.rasterWidth = 0;
    data.rasterHeight = 0;

    if (!localOSMReady_ || osmDataset_ == nullptr) {
        return data;
    }

    const Aabb tileBounds = TileSystem::TileBoundsWorld(tileId);
    const float tileSpan = std::max(tileBounds.max.x - tileBounds.min.x, tileBounds.max.y - tileBounds.min.y);
    const float looseClipPad = std::max(20.0F, tileSpan * 0.35F);
    const float coarseBoundsPad = std::max(40.0F, tileSpan * 1.25F);
    Aabb coarseBounds {
        {tileBounds.min.x - coarseBoundsPad, tileBounds.min.y - coarseBoundsPad},
        {tileBounds.max.x + coarseBoundsPad, tileBounds.max.y + coarseBoundsPad}
    };

    for (const auto& road : osmDataset_->roads) {
        if (!AabbIntersects(road.bounds, coarseBounds)) {
            continue;
        }
        auto clipped = ClipPolylineLoose(road.points, tileBounds, looseClipPad);
        if (clipped.size() < 2) {
            continue;
        }
        PolylineFeature feature;
        feature.points = std::move(clipped);
        feature.widthPixels = 3.0F;
        feature.colorRg = {0.21F, 0.24F};
        feature.colorB = 0.28F;
        data.roads.push_back(std::move(feature));
    }

    for (const auto& polygon : osmDataset_->polygons) {
        if (!AabbIntersects(polygon.bounds, coarseBounds)) {
            continue;
        }
        PolygonFeature feature;
        feature.outer = polygon.points;
        feature.colorRg = {0.55F, 0.72F};
        feature.colorB = 0.56F;
        data.landuse.push_back(std::move(feature));
    }

    for (const auto& poi : osmDataset_->pois) {
        if (poi.point.x < coarseBounds.min.x || poi.point.x > coarseBounds.max.x ||
            poi.point.y < coarseBounds.min.y || poi.point.y > coarseBounds.max.y) {
            continue;
        }
        data.pois.push_back({poi.point, 5.0F, 0.0F});
    }
    return data;
}

bool TileManager::LoadLocalOSMData(const std::string& filePath) {
    std::ifstream stream(filePath);
    if (!stream.is_open()) {
        return false;
    }

    osmDataset_ = std::make_unique<OSMDataset>();
    std::unordered_map<long long, WorldPoint> nodes;
    nodes.reserve(500000);

    struct WayTemp {
        std::vector<long long> refs;
        std::unordered_map<std::string, std::string> tags;
    };

    bool inWay = false;
    bool inNode = false;
    WayTemp currentWay;
    WorldPoint currentNodePoint {};
    std::unordered_map<std::string, std::string> currentNodeTags;

    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("<bounds ") != std::string::npos) {
            const auto minLatStr = Attr(line, "minlat");
            const auto minLonStr = Attr(line, "minlon");
            const auto maxLatStr = Attr(line, "maxlat");
            const auto maxLonStr = Attr(line, "maxlon");
            if (!minLatStr.empty() && !minLonStr.empty() && !maxLatStr.empty() && !maxLonStr.empty()) {
                const double minLat = std::stod(minLatStr);
                const double minLon = std::stod(minLonStr);
                const double maxLat = std::stod(maxLatStr);
                const double maxLon = std::stod(maxLonStr);
                osmDataset_->boundsCenter = {(minLat + maxLat) * 0.5, (minLon + maxLon) * 0.5};
                osmDataset_->hasBoundsCenter = true;
            }
            continue;
        }

        if (line.find("<node ") != std::string::npos) {
            const auto idStr = Attr(line, "id");
            const auto latStr = Attr(line, "lat");
            const auto lonStr = Attr(line, "lon");
            if (!idStr.empty() && !latStr.empty() && !lonStr.empty()) {
                const auto nodeId = std::stoll(idStr);
                const auto world = Projection::LatLngToWorld({std::stod(latStr), std::stod(lonStr)});
                nodes[nodeId] = world;
                currentNodePoint = world;
                currentNodeTags.clear();
                inNode = (line.find("/>") == std::string::npos);
            }
            continue;
        }

        if (inNode && line.find("<tag ") != std::string::npos) {
            const auto k = Attr(line, "k");
            const auto v = Attr(line, "v");
            if (!k.empty() && !v.empty()) {
                currentNodeTags[k] = v;
            }
            continue;
        }

        if (inNode && line.find("</node>") != std::string::npos) {
            inNode = false;
            bool isPOI = false;
            for (const auto& [k, _] : currentNodeTags) {
                if (IsPOITagKey(k)) {
                    isPOI = true;
                    break;
                }
            }
            if (isPOI) {
                osmDataset_->pois.push_back({currentNodePoint});
            }
            continue;
        }

        if (line.find("<way ") != std::string::npos) {
            inWay = true;
            currentWay.refs.clear();
            currentWay.tags.clear();
            continue;
        }

        if (inWay && line.find("<nd ") != std::string::npos) {
            const auto refStr = Attr(line, "ref");
            if (!refStr.empty()) {
                currentWay.refs.push_back(std::stoll(refStr));
            }
            continue;
        }

        if (inWay && line.find("<tag ") != std::string::npos) {
            const auto k = Attr(line, "k");
            const auto v = Attr(line, "v");
            if (!k.empty() && !v.empty()) {
                currentWay.tags[k] = v;
            }
            continue;
        }

        if (inWay && line.find("</way>") != std::string::npos) {
            inWay = false;
            std::vector<WorldPoint> points;
            points.reserve(currentWay.refs.size());
            for (const auto ref : currentWay.refs) {
                const auto it = nodes.find(ref);
                if (it != nodes.end()) {
                    points.push_back(it->second);
                }
            }
            if (points.size() < 2) {
                continue;
            }

            const bool isRoad = HasTag(currentWay.tags, "highway");
            const bool isArea = HasTag(currentWay.tags, "building") || HasTag(currentWay.tags, "landuse") ||
                                HasTag(currentWay.tags, "leisure") || (HasTag(currentWay.tags, "natural") && currentWay.tags["natural"] != "coastline");
            if (isRoad) {
                auto decimated = DecimatePoints(points, 1024, false);
                osmDataset_->roads.push_back({decimated, ComputeBounds(decimated)});
            } else if (isArea && points.front().x == points.back().x && points.front().y == points.back().y && points.size() >= 4) {
                auto decimated = DecimatePoints(points, 768, true);
                osmDataset_->polygons.push_back({decimated, ComputeBounds(decimated)});
            }
        }
    }
    return !osmDataset_->roads.empty() || !osmDataset_->polygons.empty() || !osmDataset_->pois.empty();
}

bool TileManager::LoadOpenStreetMapRaster(const TileId& tileId, TileData* data) const {
    const std::string cachePath = CachePathFor(tileId);
    if (!std::filesystem::exists(cachePath)) {
        const std::string url = OSMUrlFor(tileId);
        QueueRasterDownload(cachePath, url);
        return false;
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* decoded = stbi_load(cachePath.c_str(), &width, &height, &channels, 4);
    if (decoded == nullptr) {
        return false;
    }
    data->rasterWidth = static_cast<unsigned int>(width);
    data->rasterHeight = static_cast<unsigned int>(height);
    data->rasterRgba.assign(decoded, decoded + static_cast<std::size_t>(width * height * 4));
    stbi_image_free(decoded);
    return true;
}

void TileManager::QueueRasterDownload(const std::string& cachePath, const std::string& url) const {
    if (downloadState_ == nullptr) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(downloadState_->mutex);
        if (downloadState_->inFlight.find(cachePath) != downloadState_->inFlight.end()) {
            return;
        }
        constexpr std::size_t kMaxConcurrentDownloads = 4;
        if (downloadState_->inFlight.size() >= kMaxConcurrentDownloads) {
            return;
        }
        downloadState_->inFlight.insert(cachePath);
    }

    const auto state = downloadState_;
    std::thread([state, cachePath, url]() {
        const std::filesystem::path path(cachePath);
        std::filesystem::create_directories(path.parent_path());
        const std::string tmpPath = cachePath + ".tmp";
        const std::string command = "curl -L --silent --show-error --fail -o \"" + tmpPath + "\" \"" + url + "\"";
        const int result = std::system(command.c_str());
        if (result == 0 && std::filesystem::exists(tmpPath)) {
            std::error_code ec;
            std::filesystem::rename(tmpPath, cachePath, ec);
            if (ec) {
                std::filesystem::remove(tmpPath, ec);
            }
        } else {
            std::error_code ec;
            std::filesystem::remove(tmpPath, ec);
        }
        std::lock_guard<std::mutex> lock(state->mutex);
        state->inFlight.erase(cachePath);
    }).detach();
}

std::string TileManager::CachePathFor(const TileId& tileId) const {
    std::filesystem::path path(tileCacheDir_);
    path /= std::to_string(tileId.z);
    path /= std::to_string(tileId.x);
    std::filesystem::create_directories(path);
    path /= std::to_string(tileId.y) + ".png";
    return path.string();
}

std::string TileManager::OSMUrlFor(const TileId& tileId) const {
    std::string url = osmUrlTemplate_;
    url = ReplaceAll(url, "{z}", std::to_string(tileId.z));
    url = ReplaceAll(url, "{x}", std::to_string(tileId.x));
    url = ReplaceAll(url, "{y}", std::to_string(tileId.y));
    return url;
}

std::size_t TileManager::HashTile(const TileId& tileId) {
    const std::size_t z = static_cast<std::size_t>(tileId.z) & 0x3F;
    const std::size_t x = static_cast<std::size_t>(tileId.x) & 0x1FFFFFFF;
    const std::size_t y = static_cast<std::size_t>(tileId.y) & 0x1FFFFFFF;
    return (z << 58) | (x << 29) | y;
}

}  // namespace minimap::map
