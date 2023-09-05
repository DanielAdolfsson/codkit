#include "address.h"
#include <cstdio>
#include <detours.h>
#include <windows.h>

namespace codkit::detours {
    static auto Log = &Ref<void(int, const char *, ...)>(Address::DefLog);

    // Replacement logger since their built-in crashes on long lines.
    static void Log_Impl(int d, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        auto n = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (n >= sizeof(buf))
            strcpy(buf + sizeof(buf) - 13, " [truncated]");
        else
            buf[sizeof(buf) - 1] = 0;

        Log(d, "%s", buf);
    }

    void Initialize() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&Log, Log_Impl);
        DetourTransactionCommit();
    }
} // namespace codkit::detours
