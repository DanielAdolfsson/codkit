#include <map>
#include <sstream>
#include <string>
#include <windows.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "address.h"
#include "game.h"
#include "logging.h"

namespace codkit::lua {
    static lua_State *L;

    /// LUA: log(string)
    static int Impl_Log(lua_State *) {
        logging::Log("%s", luaL_checkstring(L, 1));
        return 0;
    }

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

    /// LUA: GetPlayers()
    static int Impl_GetPlayers(lua_State *) {
        auto players = game::GetPlayers();

        lua_createtable(L, (int)std::size(players), 0);

        for (int i = 0; i < std::size(players); i++) {
            lua_createtable(L, 0, 1);
            lua_pushstring(L, "Name");
            lua_pushstring(L, players[i]->Name);
            lua_settable(L, -3);
            lua_rawseti(L, -2, i + 1);
        }

        return 1;
    }

    static int Impl_CreateTimer(lua_State *) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        auto delay = luaL_checkinteger(L, 2);

        lua_pushvalue(L, 1);
        auto ref = luaL_ref(L, LUA_REGISTRYINDEX);

        SetTimer(
            game::G_ConsoleWindow,
            ref,
            delay,
            (TIMERPROC) + [](HWND, UINT, UINT_PTR id, DWORD) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, id);
                if (lua_pcall(L, 0, 0, 0) != 0)
                    lua_pop(L, 1);
            });

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
                if (auto it = commands.find(args[0]); it != commands.end()) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, it->second);
                    for (const auto &arg : args)
                        lua_pushstring(L, arg.c_str());
                    if (lua_pcall(L, std::size(args), 0, 0) != 0)
                        lua_pop(L, 1);
                }
            });

        return 0;
    }

    static int Impl_print(lua_State *) {
        std::stringstream s;
        for (auto i = 1; i <= lua_gettop(L); i++) {
            if (i > 1)
                s << '\t';

            s << lua_tostring(L, i);
        }
        logging::Log("%s", s.str().c_str());
        return 0;
    }

    void Initialize() {
        L = luaL_newstate();
        luaL_openlibs(L);

        lua_getglobal(L, "_G");
        luaL_setfuncs(
            L,
            (const luaL_Reg[]){{"Log", Impl_Log},
                               {"Execute", Impl_Execute},
                               {"GetPlayers", Impl_GetPlayers},
                               {"CreateTimer", Impl_CreateTimer},
                               {"RegisterCommand", Impl_RegisterCommand},
                               {"print", Impl_print},
                               {nullptr, nullptr}},
            0);

        game::RegisterCommand(
            "lua",
            +[]() {
                auto args = game::GetCommandArgs();

                if (std::size(args) != 2)
                    logging::Log("Usage: lua <code>\n");
                else {
                    if (luaL_dostring(L, args[1].c_str()) != 0)
                        lua_pop(L, 1);
                }
            });
    }

    void Run(const std::string &path) { luaL_dofile(L, path.c_str()); }
} // namespace codkit::lua
