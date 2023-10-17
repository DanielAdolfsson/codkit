#pragma once

#include <string>

#include "../game.h"

namespace codkit::lua {
    void Initialize();
    void UpdateScore(game::Team team, int score);
    void run(const std::string &path);
} // namespace codkit::lua
