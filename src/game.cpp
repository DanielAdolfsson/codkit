#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

#include "address.h"
#include "game.h"
#include "logging.h"

namespace codkit::game {
    HWND &gConsoleWnd = ref<HWND>(address::Console_hWnd);
    auto &gWorld = ref<World *>(address::World);
    auto &gPlayers = ref<Player (*)[]>(address::Players);
    auto &gCmdArgc = ref<int>(address::CmdArgc);
    auto &gCmdArgv = ref<char *[]>(address::CmdArgv);
    void *&gCommandQueue = ref<void *>(address::ServerHandlerQueue);

    void (&log)(int, const char *, ...) =
        ref<void(int, const char *, ...)>(address::DefLog);

    std::vector<Player *> GetPlayers() {
        std::vector<Player *> players;

        if (gPlayers == nullptr)
            return {};

        for (auto i = 0; i < gWorld->player_count; i++) {
            if ((*gPlayers)[i].status != 0)
                players.emplace_back(&(*gPlayers)[i]);
        }

        return players;
    }

    std::vector<std::string> GetCommandArgs() {
        std::vector<std::string> result;

        for (auto i = 0; i < gCmdArgc; i++)
            result.emplace_back(gCmdArgv[i]);

        return result;
    }

    void RegisterCommand(const std::string &name, void (*fn)()) {
        static auto Impl =
            ref<void(const char *, decltype(fn))>(address::RegisterCommand);
        Impl(name.c_str(), fn);
    }

} // namespace codkit::game
