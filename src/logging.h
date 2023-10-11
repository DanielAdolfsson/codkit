#pragma once

#include <string>

#include "detours.h"

namespace codkit::logging {
    template <typename... T>
    void log(const std::string &fmt, T... args) {
        game::log(0, fmt.c_str(), args...);
        game::log(0, "\n");
    }
} // namespace codkit::logging
