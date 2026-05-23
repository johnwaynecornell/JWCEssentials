//
// Created by jwc on 5/23/26.
//

#include "JWCEssentials/JWCEssentials.h"
#include <Windows.h>
#include <mutex>

namespace JWCEssentials
{
    void JWCEssentials_EnableTerminalEffects()
    {
        static std::once_flag once;
        std::call_once(once, [] {
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hOut == INVALID_HANDLE_VALUE) return;

            DWORD mode = 0;
            if (!GetConsoleMode(hOut, &mode)) return;

            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        });
    }
}
