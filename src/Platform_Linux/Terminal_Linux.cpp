//
// Created by jwc on 5/23/26.
//

#include "JWCEssentials/JWCEssentials.h"
#include <mutex>

namespace JWCEssentials
{
    void JWCEssentials_EnableTerminalEffects()
    {
        static std::once_flag once;
        std::call_once(once, [] {
        });
    }
}
