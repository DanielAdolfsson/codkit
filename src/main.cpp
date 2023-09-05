#include <cassert>
#include <iostream>
#include <string>
#include <windows.h>

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
        detours::Initialize();

        while (game::G_ConsoleWindow == nullptr)
            Sleep(100);

        ShowWindow(codkit::game::G_ConsoleWindow, 1);

        logging::Log("[codkit] initializing lua");
        lua::Initialize();

        if (Args[2] != nullptr) {
            logging::Log("[codkit] running %s...", Args[2]);
            lua::Run(Args[2]);
        }

        logging::Log("[codkit] startup complete; exiting thread");
        TerminateThread(GetCurrentThread(), 0);
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
