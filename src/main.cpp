#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cassert>
#include <iostream>
#include <string>

#include "detours.h"
#include "game.h"

extern "C" int mainCRTStartup();

extern bool Injected;
extern char **Args;

bool ExecuteAndInject(const std::string &commandLine, char *args[]);
bool Inject(DWORD processId, char *args[]);

using namespace codkit;

int main(int argc, char *argv[]) {
    if (Injected) {
        detours::initialize();

        while (game::g_console_window == nullptr)
            Sleep(100);

        PostMessage(game::g_console_window, WM_USER, 12, 34);
        ExitThread(0);
    }

    if (argc != 2 && argc != 3) {
        std::cerr << "usage: codkit.exe <path or pid> [<path to .lua>]"
                  << std::endl;
        return 0;
    }

    char *end;
    DWORD processId = strtoul(argv[1], &end, 10);

    std::cout << "CoDKit" << std::endl;

    bool result;

    if (end > argv[1] && *end == 0)
        result = Inject(processId, argv);
    else
        result = ExecuteAndInject(argv[1], argv);

    std::cout << (result ? "OK" : "FAIL") << std::endl;

    return 0;
}
