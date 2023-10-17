#include "logging.h"
#include "game.h"

namespace codkit::logging {
    Logger::~Logger() {
        flush();
    }

    void Logger::flush() {
        auto str = s.str();
        if (!str.empty()) {
            game::log(0, str.ends_with('\n') ? "%s" : "%s\n", str.c_str());
        }
    }
} // namespace codkit::logging
