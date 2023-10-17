//
// Created by daniel on 16/10/23.
//

// axis score:   5 <num>
// allies score: 6 <num>

#include <string>

#include "game.h"
#include "logging.h"
#include "lua/base.h"
#include "msg.h"

namespace codkit::msg {
    // a b c
    void HandleCommand(const std::string &cmd) {
        std::vector<std::string> args;

        // Splitting strings
        for (auto i = cmd.cbegin(); i < cmd.cend();) {
            while (i < cmd.cend() && std::isspace(*i))
                i++;

            if (i == cmd.cend())
                break;

            if (*i == '"') {
                auto start = ++i;

                while (i < cmd.cend() && *i != '"')
                    i++;

                args.emplace_back(start, i);

                if (i < cmd.cend())
                    i++;
            } else {
                auto start = i;

                while (i < cmd.cend() && *i != '"' && !std::isspace(*i))
                    i++;

                args.emplace_back(start, i);
            }
        }

        if (args.size() >= 4 && args[0] == "b") {
            lua::UpdateScore(game::Team::Axis, std::stoi(args[2]));
            lua::UpdateScore(game::Team::Allies, std::stoi(args[3]));
        }

        else if (args.size() == 3 && args[0] == "d") {
            if (args[1] == "5") {
                lua::UpdateScore(game::Team::Axis, std::stoi(args[2]));
            } else if (args[1] == "6") {
                lua::UpdateScore(game::Team::Allies, std::stoi(args[2]));
            }
        }

        // logging::Logger{} << "CMD: " << cmd;
    }
} // namespace codkit::msg
