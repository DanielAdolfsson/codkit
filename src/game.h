#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace codkit::game {
    union World {
        struct {
            char _0x0000[0x20];
            int PlayerCount;
        };
    };

    union Player {
        struct {
            int Status;
            char _0x0004[0x10a40];
            char Name[];
        };

        char data[371120];
    };

    extern HWND &G_ConsoleWindow;

    std::vector<Player *> GetPlayers();
    void RegisterCommand(const std::string &name, void (*f)());
    std::vector<std::string> GetCommandArgs();
} // namespace codkit::game
