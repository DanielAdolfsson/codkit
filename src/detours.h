#pragma once

#include <functional>

namespace codkit::detours {
    void initialize();
    void run_on_main_thread(const std::function<void()> &function);
} // namespace codkit::detours
