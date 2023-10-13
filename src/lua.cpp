#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <map>
#include <sstream>
#include <string>

#include <httplib.h>
#define SOL_USING_CXX_LUA 1
#include <sol/sol.hpp>

#include "address.h"
#include "game.h"
#include "httpserver.h"
#include "logging.h"

namespace codkit::lua {
    static sol::state state;

    /*
        /// LUA: Execute(string)
        static int Impl_Execute(lua_State *) {
            auto str = luaL_checkstring(L, 1);

            char buf[512];
            snprintf(buf, sizeof(buf), "%s\n", str);
            buf[sizeof(buf) - 1] = 0;

            // Optimized, so we can't call it normally.

            __asm__("call %P1;"
                    :
                    : "a"(buf), "r"(&Ref<void *>(Address::Cbuf_AddText)));

            return 0;
        }

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

    static auto impl_log(const sol::object &value) {
        value.push();
        logging::log("%s", luaL_tolstring(value.lua_state(), -1, nullptr));
    }

    static auto impl_print(const sol::variadic_args &va) {
        std::stringstream ss;
        for (auto arg : va) {
            if (ss.tellp() > 0)
                ss << '\t';

            ss << std::string{
                luaL_tolstring(arg.lua_state(), arg.stack_index(), nullptr)};
        }
        logging::log("%s", ss.str().c_str());
    }

    static auto impl_create_timer(const std::function<void()> &fn, int time) {
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

    static auto impl_exec(const std::string &cmd) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s\n", cmd.c_str());
        buf[sizeof(buf) - 1] = 0;

        // Optimized, so we can't call it normally.
        __asm__("call %P1;"
                :
                : "a"(buf), "r"(&ref<void *>(address::Cbuf_AddText)));
    }

    static auto impl_get_players() {
        auto players = game::get_players();
        auto table = state.create_table((int)std::size(players));

        for (int i = 0; i < std::size(players); i++) {
            table.add(
                state.create_table_with("name", std::string{players[i]->name})
            );
        }

        return table;
    }

    void initialize() {
        state.open_libraries();

        //state.set_exception_handler(&exception_handler);

        state.set(
            "log",
            impl_log,
            "print",
            impl_print,
            "create_timer",
            impl_create_timer,
            "exec",
            impl_exec,
            "get_players",
            impl_get_players
        );

        state.new_usertype<http::HttpServer>(
            "HttpServer",
            "get",
            &http::HttpServer::get,
            "listen",
            &http::HttpServer::listen
        );

        state.new_usertype<http::HttpRequest>(
            "HttpRequest",
            sol::no_constructor,
            "body",
            &http::HttpRequest::body
        );

        state.new_usertype<http::HttpResponse>(
            "HttpResponse",
            sol::no_constructor,
            "body",
            &http::HttpResponse::body
        );

        game::register_command(
            "lua",
            +[]() {
                auto args = game::get_command_args();

                if (std::size(args) != 2)
                    logging::log("Usage: lua <code>\n");
                else {
                    try {
                        state.safe_script(args[1]);
                    } catch (const sol::error &e) {
                        logging::log("--- lua ---\n%s", e.what());
                    }
                }
            }
        );
    }

    void run(const std::string &path) {
        try {
            state.safe_script_file(path);
        } catch (const sol::error &e) {
            logging::log("--- lua ---\n%s", e.what());
        }
    }
} // namespace codkit::lua
