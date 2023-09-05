#include <vector>
#include <windows.h>

#include "address.h"
#include "game.h"
#include "logging.h"

namespace codkit::game {
    HWND &G_ConsoleWindow = Ref<HWND>(Address::Console_hWnd);
    auto &G_World = Ref<World *>(Address::World);
    auto &G_Players = Ref<Player (*)[]>(Address::Players);
    auto &G_CArgc = Ref<int>(Address::CmdArgc);
    auto &G_CArgv = Ref<char *[]>(Address::CmdArgv);

    std::vector<Player *> GetPlayers() {
        std::vector<Player *> players;

        if (G_Players == nullptr)
            return {};

        for (auto i = 0; i < G_World->PlayerCount; i++) {
            if ((*G_Players)[i].Status != 0)
                players.emplace_back(&(*G_Players)[i]);
        }

        return players;
    }

    std::vector<std::string> GetCommandArgs() {
        std::vector<std::string> result;

        for (auto i = 0; i < G_CArgc; i++)
            result.emplace_back(G_CArgv[i]);

        return result;
    }

    void RegisterCommand(const std::string &name, void (*fn)()) {
        static auto Impl =
            Ref<void(const char *, decltype(fn))>(Address::RegisterCommand);
        Impl(name.c_str(), fn);
    }
} // namespace codkit::game
