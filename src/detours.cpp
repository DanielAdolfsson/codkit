#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <stdexcept>

#include <detours.h>

#include "address.h"
#include "game.h"
#include "logging.h"
#include "msg.h"

extern char **Args;

namespace codkit::detours {
    static auto WndProc = ref<LRESULT CALLBACK(HWND, UINT, WPARAM, LPARAM)>(
        address::Console_WndProc
    );
    static auto log = ref<void(int, const char *, ...)>(address::DefLog);
    static auto CommandHandler = ref<void()>(address::ServerHandler);

    static CALLBACK LRESULT
    impl_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_USER) {
            try {
                if (lParam) {
                    *(std::exception_ptr *)lParam = nullptr;
                }
                return (*reinterpret_cast<std::function<int()> *>(wParam))();
            } catch (...) {
                if (lParam) {
                    *(std::exception_ptr *)lParam = std::current_exception();
                }
                return -1;
            }
        }

        return WndProc(hWnd, uMsg, wParam, lParam);
    }

    // Replacement logger since their built-in crashes on long lines.
    static void impl_log(int d, const char *fmt, ...) {
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

    static void server_command_inner(const char *s) noexcept {
        try {
            msg::HandleCommand(s);
        } catch (...) {
            // We'll throw away any exceptions.
        }
    }

    __attribute__((naked)) static void impl_CommandHandler() {
        __asm__ __volatile__("mov %%eax, %%edi;"
                             "push %%edi;"
                             :
                             :
                             : "edi");

        // Maybe do some more saving of registers.
        __asm__ __volatile__("and $0x3f, %%edi;"
                             "shl $0xa, %%edi;"
                             "add %0, %%edi;"
                             "push %%edi;"
                             "call %P1;"
                             "add $4, %%esp;"
                             "pop %%eax;"
                             "call *%2;"
                             "ret;"
                             :
                             : "a"(&game::gCommandQueue),
                               "i"(+[](const char *s) {
                                   try {
                                       msg::HandleCommand(s);
                                   } catch (...) {
                                       // We'll throw away any exceptions.
                                   }
                               }),
                               //"m"(server_command_inner),
                               "m"(CommandHandler)
                             :);
    }

    void Enqueue(const std::function<int()> &function) {
        std::exception_ptr e;
        SendMessage(game::gConsoleWnd, WM_USER, (WPARAM)&function, (LPARAM)&e);
        if (e) {
            std::rethrow_exception(e);
        }
    }

    void Initialize() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&log, impl_log);
        DetourAttach(&WndProc, impl_WndProc);
        DetourAttach(&CommandHandler, impl_CommandHandler);
        DetourTransactionCommit();
    }
} // namespace codkit::detours
