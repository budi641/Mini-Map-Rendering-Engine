#include "Utils/Logger.h"

#include <iostream>

namespace minimap::utils {

void Logger::Log(LogLevel level, std::string_view message) {
    const char* prefix = "[INFO]";
    if (level == LogLevel::Warning) {
        prefix = "[WARN]";
    } else if (level == LogLevel::Error) {
        prefix = "[ERR ]";
    }
    std::cout << prefix << ' ' << message << '\n';
}

}  // namespace minimap::utils
