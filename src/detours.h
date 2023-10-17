#pragma once

#include <functional>

namespace codkit::detours {
    void Initialize();
    void Enqueue(const std::function<int()> &function);
} // namespace codkit::detours
