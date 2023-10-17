#pragma once

#include <sstream>
#include <string>

#include "detours.h"

namespace codkit::logging {
    class Logger {
    private:
        std::ostringstream s;

    public:
        void flush();
        ~Logger();

        template <typename T>
        Logger &operator<<(const T &rhs) {
            s << rhs;
            return *this;
        }
    };
} // namespace codkit::logging
