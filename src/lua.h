#pragma once

#include <string>

namespace codkit::lua {
    void initialize();
    void run(const std::string &path);
} // namespace codkit::lua
