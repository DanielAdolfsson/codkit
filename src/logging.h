#pragma once

#include <string>

#include "detours.h"

namespace codkit::logging {
    template <typename... T>
    void Log(const std::string &fmt, T... args) {
        // detours::Log(0, fmt.c_str(), args...);
        // detours::Log(0, "\n");
    }
} // namespace codkit::logging