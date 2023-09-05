#pragma once

#include <string>

namespace codkit::lua {
    void Initialize();
    void Run(const std::string &path);
} // namespace codkit::lua
