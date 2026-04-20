#include "Utils/Timer.h"

namespace minimap::utils {

void Timer::Start() {
    start_ = std::chrono::steady_clock::now();
}

double Timer::ElapsedMs() const {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start_).count();
}

}  // namespace minimap::utils
