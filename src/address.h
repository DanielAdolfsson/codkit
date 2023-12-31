#pragma once

#include <windows.h>

namespace codkit {
    enum class address : DWORD_PTR {
        WndProc = 0x46c160,
        hWnd = 0x01999d68,
        Console_hWnd = 0x93e100,
        Console_WndProc = 0x0046b400,
        DefLog = 0x00412960,
        Log = 0x19b0b00,
        Cbuf_AddText = 0x42a090,
        Players = 0x197f1b0,
        World = 0x019942b0,
        InitializeRenderer = 0x412e20,
        RegisterCommand = 0x0042a870,
        CmdArgc = 0x008d5020,
        CmdArgv = 0x008d2620,
        ServerHandler = 0x00401620,
        ServerHandlerQueue = 0x18a9a68,
    };

    template <typename T>
    T &ref(address address) {
        return *(T *)((DWORD_PTR)GetModuleHandle(nullptr) + //
                      (DWORD_PTR)address -                  //
                      0x400000);
    }
} // namespace codkit
