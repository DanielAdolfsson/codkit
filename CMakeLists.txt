cmake_minimum_required(VERSION 3.26)
project(codkit CXX C)

include(ExternalProject)

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX i686-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_BUILD ON)

add_library(lua OBJECT
        # core
        vendor/lua/lapi.c
        vendor/lua/lcode.c
        vendor/lua/lctype.c
        vendor/lua/ldebug.c
        vendor/lua/ldo.c
        vendor/lua/ldump.c
        vendor/lua/lfunc.c
        vendor/lua/lgc.c
        vendor/lua/llex.c
        vendor/lua/lmem.c
        vendor/lua/lobject.c
        vendor/lua/lopcodes.c
        vendor/lua/lparser.c
        vendor/lua/lstate.c
        vendor/lua/lstring.c
        vendor/lua/ltable.c
        vendor/lua/ltm.c
        vendor/lua/lundump.c
        vendor/lua/lvm.c
        vendor/lua/lzio.c
        vendor/lua/ltests.c
        # aux
        vendor/lua/lauxlib.c
        # lib
        vendor/lua/lbaselib.c
        vendor/lua/ldblib.c
        vendor/lua/liolib.c
        vendor/lua/lmathlib.c
        vendor/lua/loslib.c
        vendor/lua/ltablib.c
        vendor/lua/lstrlib.c
        vendor/lua/lutf8lib.c
        vendor/lua/loadlib.c
        vendor/lua/lcorolib.c
        vendor/lua/linit.c
)

get_target_property(lua_SOURCES lua SOURCES)
set_source_files_properties(${lua_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(Detours OBJECT
        vendor/Detours/src/image.cpp
        vendor/Detours/src/disasm.cpp
        vendor/Detours/src/modules.cpp
        vendor/Detours/src/detours.cpp
        vendor/Detours/src/disolx64.cpp
        vendor/Detours/src/creatwth.cpp
)

add_executable(codkit src/proc.cpp src/main.cpp src/lua.cpp src/game.cpp src/detours.cpp src/httpserver.cpp)

target_link_libraries(codkit PRIVATE
        m
        lua
        Detours
        -lws2_32
        -static-libgcc -static-libstdc++ -static)

target_include_directories(codkit PUBLIC
        vendor/lua
        vendor/Detours/src
        vendor/cpp-httplib
        vendor/sol2/include
)

