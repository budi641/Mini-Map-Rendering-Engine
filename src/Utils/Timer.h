#pragma once

#include <chrono>

namespace minimap::utils {

class Timer {
public:
    void Start();
    [[nodiscard]] double ElapsedMs() const;

private:
    std::chrono::steady_clock::time_point start_ {};
};

}  // namespace minimap::utils
