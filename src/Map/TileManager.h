#pragma once

#include "Map/Types.h"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace minimap::map {

class TileManager {
public:
    TileManager();
    ~TileManager();
    const TileData& GetOrCreateTile(const TileId& tileId);
    [[nodiscard]] bool HasSuggestedCenter() const;
    [[nodiscard]] LatLng SuggestedCenter() const;

private:
    enum class VectorSource {
        Synthetic,
        LocalOSMFile
    };

    enum class RasterSource {
        Synthetic,
        OpenStreetMap
    };

    static TileData BuildSyntheticTile(const TileId& tileId);
    TileData BuildTileFromLocalOSM(const TileId& tileId) const;
    bool LoadLocalOSMData(const std::string& filePath);
    bool LoadOpenStreetMapRaster(const TileId& tileId, TileData* data) const;
    void QueueRasterDownload(const std::string& cachePath, const std::string& url) const;
    std::string CachePathFor(const TileId& tileId) const;
    std::string OSMUrlFor(const TileId& tileId) const;
    static std::size_t HashTile(const TileId& tileId);

    struct OSMDataset;
    struct DownloadState;
    bool localOSMReady_ {false};
    std::unique_ptr<OSMDataset> osmDataset_;
    std::shared_ptr<DownloadState> downloadState_;
    VectorSource vectorSource_ {VectorSource::Synthetic};
    std::string osmFilePath_ {};
    RasterSource rasterSource_ {RasterSource::Synthetic};
    std::string tileCacheDir_ {};
    std::string osmUrlTemplate_ {};
    std::unordered_map<std::size_t, TileData> cache_;
};

}  // namespace minimap::map
