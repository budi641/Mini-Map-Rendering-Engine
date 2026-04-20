#pragma once

#include <string_view>

namespace minimap::utils {

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    static void Log(LogLevel level, std::string_view message);
};

}  // namespace minimap::utils
