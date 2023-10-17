#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <map>
#include <string>

#define SOL_USING_CXX_LUA 1
#include "sol/sol.hpp"

#include "../address.h"
#include "../game.h"
#include "../logging.h"
#include "httpserver.h"

namespace codkit::lua {
    static sol::state state;

    /*
        static int Impl_RegisterCommand(lua_State *) {
            static std::map<std::string, lua_Integer> commands;
            auto command = luaL_checkstring(L, 1);
            luaL_checktype(L, 2, LUA_TFUNCTION);

            commands[command] = luaL_ref(L, LUA_REGISTRYINDEX);

            game::RegisterCommand(
                command,
                +[]() {
                    auto args = game::GetCommandArgs();
                    if (auto it = commands.find(args[0]); it != commands.end())
       { lua_rawgeti(L, LUA_REGISTRYINDEX, it->second); for (const auto &arg :
       args) lua_pushstring(L, arg.c_str()); if (lua_pcall(L, std::size(args),
       0, 0) != 0) lua_pop(L, 1);
                    }
                });

            return 0;
        }



        static int Impl_HttpServer(lua_State *) {
            logging::Log("codkit: New HTTP server");

            auto *server = new (lua_newuserdata(L, sizeof(httplib::Server)))
       httplib::Server; luaL_getmetatable(L, "HttpServer"); lua_setmetatable(L,
       -2);

            return 1;
        }*/

    static auto exception_handler(
        lua_State *L,
        sol::optional<const std::exception &> maybe_exception,
        sol::string_view description
    ) {
        return sol::stack::push(L, "an exception");
    }

    static std::map<UINT_PTR, std::function<void()>> timer_functions;

    static auto impl_Log(const sol::object &value) {
        value.push();
        logging::Logger{} << luaL_tolstring(value.lua_state(), -1, nullptr);
    }

    static auto impl_print(const sol::variadic_args &va) {
        logging::Logger logger{};
        for (auto i = 0; i < va.size(); i++) {
            if (i > 0)
                logger << '\t';

            logger << luaL_tolstring(
                va[i].lua_state(),
                va[i].stack_index(),
                nullptr
            );
        }
    }

    static auto impl_CreateTimer(const std::function<void()> &fn, int time) {
        auto id = SetTimer(
            nullptr,
            0,
            time,
            (TIMERPROC) +
                [](HWND, UINT, UINT_PTR id, DWORD) {
                    if (const auto &it = timer_functions.find(id);
                        it != timer_functions.end()) {
                        it->second();
                    }
                }
        );

        if (id == 0)
            // TODO: Failure.
            return;

        timer_functions[id] = fn;
    }

    static auto impl_Exec(const std::string &cmd) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s\n", cmd.c_str());
        buf[sizeof(buf) - 1] = 0;

        // Optimized, so we can't call it normally.
        __asm__("call %P1;"
                :
                : "a"(buf), "r"(&ref<void *>(address::Cbuf_AddText)));
    }

    static auto impl_GetPlayers() {
        auto players = game::GetPlayers();
        auto table = state.create_table((int)std::size(players));

        for (int i = 0; i < std::size(players); i++) {
            table.add(
                state.create_table_with("Name", std::string{players[i]->name})
            );
        }

        return table;
    }

    static auto impl_RequestScores() {
        // Optimized, so we can't call it normally.
        __asm__("call %P1;"
                :
                : "a"("score"), "r"(&ref<void *>(address::Cbuf_AddText)));
    }

    void UpdateScore(game::Team team, int score) {
        try {
            state[team == game::Team::Axis ? "Axis" : "Allies"] = score;
            state["OnScoreUpdated"](
                team == game::Team::Axis ? "Axis" : "Allies",
                score
            );
        } catch (const sol::error &error) {
            logging::Logger{} << "Fail:\n" << error.what();
        }
    }

    void Initialize() {
        state.open_libraries();

        // state.set_exception_handler(&exception_handler);

        state.set(
            "Log",
            impl_Log,
            "print",
            impl_print,
            "CreateTimer",
            impl_CreateTimer,
            "exec",
            impl_Exec,
            "GetPlayers",
            impl_GetPlayers,
            "RequestScores",
            impl_RequestScores
        );

        state.new_usertype<HttpServer>(
            "HttpServer",
            "Get",
            &HttpServer::Get,
            "Mount",
            &HttpServer::Mount,
            "Listen",
            sol::overload(
                static_cast<void (HttpServer::*)(int port)>(&HttpServer::Listen
                ),
                static_cast<
                    void (HttpServer::*)(int port, const std::string &host)>(
                    &HttpServer::Listen
                )
            )
        );

        state.new_usertype<HttpServerRequest>(
            "HttpServerRequest",
            sol::no_constructor,
            "Body",
            sol::property(&HttpServerRequest::GetBody)
        );

        state.new_usertype<HttpServerResponse>(
            "HttpServerResponse",
            sol::factories([]() {
                return std::make_shared<HttpServerResponse>();
            }),
            "WithBody",
            &HttpServerResponse::WithBody
        );

        game::RegisterCommand(
            "lua",
            +[]() {
                auto args = game::GetCommandArgs();

                if (std::size(args) != 2)
                    logging::Logger{} << "Usage: lua <code>";
                else {
                    try {
                        state.safe_script(args[1]);
                    } catch (const sol::error &e) {
                        logging::Logger{} << "--- lua ---\n" << e.what();
                    }
                }
            }
        );
    }

    void run(const std::string &path) {
        try {
            state.safe_script_file(path);
        } catch (const sol::error &e) {
            logging::Logger{} << "--- lua ---\n" << e.what();
        }
    }
} // namespace codkit::lua
