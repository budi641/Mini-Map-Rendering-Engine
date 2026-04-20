$ErrorActionPreference = "Stop"

Write-Host "==> Configure" -ForegroundColor Cyan
cmake -S . -B build -DMINIMAP_BUILD_DESKTOP=ON -DMINIMAP_BUILD_ANDROID=OFF -DMINIMAP_ENABLE_STRICT_WARNINGS=ON

Write-Host "==> Build (Release)" -ForegroundColor Cyan
cmake --build build --config Release

Write-Host "==> Deterministic lane" -ForegroundColor Cyan
ctest --test-dir build -C Release --output-on-failure -L deterministic

Write-Host "==> Render lane" -ForegroundColor Cyan
ctest --test-dir build -C Release --output-on-failure -L render

Write-Host "==> Local OSM lane (auto-skip if maps/map.osm missing)" -ForegroundColor Cyan
ctest --test-dir build -C Release --output-on-failure -L local_osm

Write-Host "==> Headless render self-test artifact" -ForegroundColor Cyan
$env:MINIMAP_VECTOR_SOURCE = "synthetic"
$env:MINIMAP_RASTER_SOURCE = "synthetic"
& "build/Release/minimap_demo.exe" --self-test self_test_report.csv
$env:MINIMAP_VECTOR_SOURCE = $null
$env:MINIMAP_RASTER_SOURCE = $null

Write-Host "Verification pass complete." -ForegroundColor Green
