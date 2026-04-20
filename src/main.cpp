#include "Core/Application.h"
#include "Core/Window.h"
#include "GPU/GlHeaders.h"
#include "Renderer/RenderEngine.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {

std::uint64_t HashFramebuffer(int width, int height) {
    std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Sample pixels on a coarse grid to keep this inexpensive.
    constexpr std::uint64_t kFnvOffset = 1469598103934665603ULL;
    constexpr std::uint64_t kFnvPrime = 1099511628211ULL;
    std::uint64_t hash = kFnvOffset;
    const int stepY = std::max(1, height / 48);
    const int stepX = std::max(1, width / 64);
    for (int y = 0; y < height; y += stepY) {
        for (int x = 0; x < width; x += stepX) {
            const std::size_t idx = (static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4U;
            hash ^= pixels[idx + 0];
            hash *= kFnvPrime;
            hash ^= pixels[idx + 1];
            hash *= kFnvPrime;
            hash ^= pixels[idx + 2];
            hash *= kFnvPrime;
            hash ^= pixels[idx + 3];
            hash *= kFnvPrime;
        }
    }
    return hash;
}

int RunSelfTest(bool visible, const char* reportPath) {
    minimap::core::Window window(800, 600, visible ? "MiniMap SelfTest (Visible)" : "MiniMap SelfTest", visible);
    minimap::renderer::RenderEngine engine;
    engine.Initialize(window.Width(), window.Height());
    minimap::core::InputState scriptedInput {};

    constexpr int kFrames = 120;
    std::unordered_set<std::uint64_t> frameHashes;
    frameHashes.reserve(kFrames);
    std::ofstream report;
    if (reportPath != nullptr) {
        report.open(reportPath, std::ios::trunc);
        if (report.is_open()) {
            report << "frame,hash,visible_tiles,vertices,indices,poi_instances\n";
        }
    }

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kFrames; ++i) {
        window.PollEvents();
        engine.OnResize(window.Width(), window.Height());
        scriptedInput.leftMouseDown = (i >= 20 && i < 95);
        scriptedInput.previousMousePosition = scriptedInput.mousePosition;
        scriptedInput.mousePosition.x += (i % 2 == 0) ? 1.5F : -0.5F;
        scriptedInput.mousePosition.y += 0.25F;
        scriptedInput.scrollDelta = (i < 40) ? 0.06F : ((i < 80) ? -0.04F : 0.0F);
        engine.HandleInput(scriptedInput, 1.0F / 60.0F);
        engine.Update(1.0F / 60.0F);
        engine.Render();
        const std::uint64_t hash = HashFramebuffer(window.Width(), window.Height());
        frameHashes.insert(hash);
        if (report.is_open()) {
            report << i << ',' << hash << ','
                   << engine.LastVisibleTileCount() << ','
                   << engine.LastVertexCount() << ','
                   << engine.LastIndexCount() << ','
                   << engine.LastPoiInstanceCount() << '\n';
        }
        if (glGetError() != GL_NO_ERROR) {
            std::cerr << "Self-test failed: OpenGL error detected on frame " << i << '\n';
            return 2;
        }
        window.SwapBuffers();
    }

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start
    ).count();

    const bool statsOk = engine.LastVisibleTileCount() >= 1 &&
                         engine.LastVertexCount() >= 100 &&
                         engine.LastIndexCount() >= 100 &&
                         engine.LastPoiInstanceCount() >= 1;
    const bool visualDiversityOk = frameHashes.size() >= 2;
    if (!statsOk) {
        std::cerr << "Self-test failed: frame output contracts were not met.\n"
                  << "VisibleTiles=" << engine.LastVisibleTileCount()
                  << " Vertices=" << engine.LastVertexCount()
                  << " Indices=" << engine.LastIndexCount()
                  << " POIs=" << engine.LastPoiInstanceCount() << '\n';
        return 3;
    }
    if (!visualDiversityOk) {
        std::cerr << "Self-test failed: frame buffer hash did not change across frames.\n";
        return 4;
    }

    std::cout << "SELF_TEST_PASS "
              << "frames=" << kFrames
              << " elapsed_ms=" << elapsedMs
              << " visible_tiles=" << engine.LastVisibleTileCount()
              << " vertices=" << engine.LastVertexCount()
              << " indices=" << engine.LastIndexCount()
              << " poi_instances=" << engine.LastPoiInstanceCount()
              << " unique_hashes=" << frameHashes.size() << '\n';
    return 0;
}

int RunVisualVerify() {
    minimap::core::Window window(1280, 720, "MiniMap Visual Verify");
    minimap::renderer::RenderEngine engine;
    engine.Initialize(window.Width(), window.Height());
    std::cout << "Visual verify mode running.\n";
    std::cout << "Controls: left drag pan, mouse wheel zoom, right click twice for distance/bearing.\n";
    std::cout << "Close window when done. Telemetry will print every second.\n";

    double last = glfwGetTime();
    double logAccum = 0.0;
    int frameCount = 0;
    std::uint64_t rollingHash = 0;

    while (!window.ShouldClose()) {
        window.PollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - last);
        last = now;

        engine.OnResize(window.Width(), window.Height());
        engine.HandleInput(window.Input(), dt);
        engine.Update(dt);
        engine.Render();

        rollingHash ^= HashFramebuffer(window.Width(), window.Height()) + 0x9E3779B97F4A7C15ULL + (rollingHash << 6) + (rollingHash >> 2);
        if (glGetError() != GL_NO_ERROR) {
            std::cerr << "Visual verify failed: OpenGL error detected.\n";
            return 5;
        }
        window.SwapBuffers();

        ++frameCount;
        logAccum += dt;
        if (logAccum >= 1.0) {
            const double fps = static_cast<double>(frameCount) / logAccum;
            std::cout << "fps=" << fps
                      << " visible_tiles=" << engine.LastVisibleTileCount()
                      << " vertices=" << engine.LastVertexCount()
                      << " indices=" << engine.LastIndexCount()
                      << " poi_instances=" << engine.LastPoiInstanceCount()
                      << " rolling_hash=" << rollingHash
                      << '\n';
            frameCount = 0;
            logAccum = 0.0;
        }
    }

    std::cout << "VISUAL_VERIFY_COMPLETE rolling_hash=" << rollingHash << '\n';
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc >= 2 && std::string_view(argv[1]) == "--self-test") {
            const char* reportPath = (argc >= 3) ? argv[2] : nullptr;
            return RunSelfTest(false, reportPath);
        }
        if (argc >= 2 && std::string_view(argv[1]) == "--self-test-visible") {
            const char* reportPath = (argc >= 3) ? argv[2] : nullptr;
            return RunSelfTest(true, reportPath);
        }
        if (argc >= 2 && std::string_view(argv[1]) == "--visual-verify") {
            return RunVisualVerify();
        }
        minimap::core::Application app;
        app.Run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
