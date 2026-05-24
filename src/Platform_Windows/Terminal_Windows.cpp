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
        static bool is_fully_enabled = false;
        if (is_fully_enabled) return;

        // 1. Direct Awareness: Open the active console output buffer explicitly.
        // This bypasses any standard stream redirection or pipe hijacking happening in C#.
        HANDLE hOut = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, 
                                FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        
        if (hOut == INVALID_HANDLE_VALUE) {
            // Fallback to standard handle if CONOUT$ isn't available (e.g., truly headless)
            hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        }

        if (hOut == INVALID_HANDLE_VALUE || hOut == nullptr) return;

        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            // Add the Virtual Terminal flags
            DWORD target_mode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            
            // 2. Set it and immediately verify it
            if (SetConsoleMode(hOut, target_mode)) {
                DWORD verify_mode = 0;
                if (GetConsoleMode(hOut, &verify_mode) && (verify_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                    is_fully_enabled = true; // The terminal is officially aware and locked in
                }
            }
        }

        // Clean up the file handle if we opened CONOUT$ manually
        if (hOut != GetStdHandle(STD_OUTPUT_HANDLE)) {
            CloseHandle(hOut);
        }
    }
}
