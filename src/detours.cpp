#define WIN32_LEAN_AND_MEAN
#include <cstdio>
#include <windows.h>

#include <detours.h>

#include "address.h"
#include "game.h"
#include "httpserver.h"
#include "logging.h"
#include "lua.h"

extern char **Args;

namespace codkit::detours {
    static auto WndProc = ref<LRESULT CALLBACK(HWND, UINT, WPARAM, LPARAM)>(
        address::Console_WndProc
    );
    static auto log = ref<void(int, const char *, ...)>(address::DefLog);

    static CALLBACK LRESULT
    WndProc_impl(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_USER) {
            ShowWindow(game::g_console_window, 1);

            logging::log("(codkit) startup");

            logging::log("(codkit) initialize lua");
            lua::initialize();

            if (Args[2] != nullptr) {
                logging::log("[codkit] running %s...", Args[2]);
                lua::run(Args[2]);
            }

            logging::log("(codkit) startup complete");
            return 0;
        }

        if (uMsg == WM_USER + 1) {
            const auto &context = *(http::HttpContext *)(wParam);
            auto request =
                std::make_shared<http::HttpRequest>(&context.request);
            auto response =
                std::make_shared<http::HttpResponse>(&context.response);
            context.handler(request, response);
            request->invalidate();
            response->invalidate();
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

    void initialize() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&log, log_impl);
        DetourAttach(&WndProc, WndProc_impl);
        DetourTransactionCommit();
    }
} // namespace codkit::detours
