#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

#include "address.h"
#include "game.h"
#include "logging.h"

namespace codkit::game {
    HWND &g_console_window = ref<HWND>(address::Console_hWnd);
    auto &g_world = ref<World *>(address::World);
    auto &g_players = ref<Player (*)[]>(address::Players);
    auto &g_c_argc = ref<int>(address::CmdArgc);
    auto &g_c_argv = ref<char *[]>(address::CmdArgv);

    void (&log)(int, const char *, ...) =
        ref<void(int, const char *, ...)>(address::DefLog);

    std::vector<Player *> get_players() {
        std::vector<Player *> players;

        if (g_players == nullptr)
            return {};

        for (auto i = 0; i < g_world->player_count; i++) {
            if ((*g_players)[i].status != 0)
                players.emplace_back(&(*g_players)[i]);
        }

        return players;
    }

    std::vector<std::string> get_command_args() {
        std::vector<std::string> result;

        for (auto i = 0; i < g_c_argc; i++)
            result.emplace_back(g_c_argv[i]);

        return result;
    }

    void register_command(const std::string &name, void (*fn)()) {
        static auto Impl =
            ref<void(const char *, decltype(fn))>(address::RegisterCommand);
        Impl(name.c_str(), fn);
    }
} // namespace codkit::game
