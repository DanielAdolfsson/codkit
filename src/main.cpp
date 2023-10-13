#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cassert>
#include <iostream>
#include <string>

#include "detours.h"
#include "game.h"
#include "logging.h"
#include "lua.h"

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

        detours::run_on_main_thread([]() {
            ShowWindow(game::g_console_window, 1);

            logging::log("(codkit) startup");

            logging::log("(codkit) initialize lua");
            lua::initialize();

            if (Args[2] != nullptr) {
                logging::log("[codkit] running %s...", Args[2]);
                lua::run(Args[2]);
            }

            logging::log("(codkit) startup complete");
        });

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
