#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cassert>
#include <iostream>
#include <string>

#include "detours.h"
#include "game.h"
#include "logging.h"
#include "lua/base.h"

extern "C" int mainCRTStartup();

extern bool Injected;
extern char **Args;

bool ExecuteAndInject(const std::string &commandLine, char *args[]);
bool Inject(DWORD processId, char *args[]);

using namespace codkit;

int main(int argc, char *argv[]) {
    if (Injected) {
        detours::Initialize();

        while (game::gConsoleWnd == nullptr)
            Sleep(100);

        detours::Enqueue([]() {
            ShowWindow(game::gConsoleWnd, 1);

            logging::Logger{} << "(codkit) startup";
            logging::Logger{} << "(codkit) Initialize lua";

            lua::Initialize();

            if (Args[2] != nullptr) {
                logging::Logger{} << "[codkit] running " << Args[2] << "...";
                lua::run(Args[2]);
            }

            logging::Logger{} << "(codkit) startup complete";
            return 0;
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
