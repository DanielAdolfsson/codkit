#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace codkit::game {
    struct World {
        char _0x0000[0x20];
        int player_count;
    };

    struct Player {
        int status;
        char _4[68160];
        char name[0];
        char _68164[302956];
    };

    extern HWND &g_console_window;
    extern void (&log)(int, const char *, ...);

    std::vector<Player *> get_players();
    void register_command(const std::string &name, void (*f)());
    std::vector<std::string> get_command_args();
} // namespace codkit::game
