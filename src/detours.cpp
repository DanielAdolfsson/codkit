#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <stdexcept>

#include <detours.h>

#include "address.h"
#include "game.h"
#include "logging.h"

extern char **Args;

namespace codkit::detours {
    static auto WndProc = ref<LRESULT CALLBACK(HWND, UINT, WPARAM, LPARAM)>(
        address::Console_WndProc
    );
    static auto log = ref<void(int, const char *, ...)>(address::DefLog);

    static CALLBACK LRESULT
    WndProc_impl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_USER) {
            try {
                (*reinterpret_cast<std::function<void()> *>(wParam))();
            } catch (const std::runtime_error& error) {
                MessageBox(nullptr, error.what(), "Exception", MB_OK);
            } catch (...) {
                MessageBox(nullptr, "threw", nullptr, MB_OK);
            }
            return 0;
        }

        return WndProc(hWnd, uMsg, wParam, lParam);
    }

    // Replacement logger since their built-in crashes on long lines.
    static void log_impl(int d, const char *fmt, ...) {
        char buf[4096];
        va_list args;
        va_start(args, fmt);
        auto n = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (n >= sizeof(buf))
            strcpy(buf + sizeof(buf) - 13, " [truncated]");
        else
            buf[sizeof(buf) - 1] = 0;

        log(d, "%s", buf);
    }

    void run_on_main_thread(const std::function<void()> &function) {
        SendMessage(game::g_console_window, WM_USER, (WPARAM)&function, 0);
    }

    void initialize() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&log, log_impl);
        DetourAttach(&WndProc, WndProc_impl);
        DetourTransactionCommit();
    }
} // namespace codkit::detours
