//
// Created by jwc on 7/26/24.
//

#include "test.h"

#include <iostream>

using namespace JWCEssentials;
using namespace JWCCommandSpawn;

int main() {
    CommandSpawn *proc = CommandSpawn_Create();

    proc->Command("dir /s", CommandSpawn::E_PIPE_NONE);

    proc->Command("dir /s", CommandSpawn::E_PIPE_STDOUT);

    utf8_string_struct line;
    while ((line = proc->ReadLine(CommandSpawn::E_PIPE_STDOUT)) != nullptr) {
        std::cout << "got " << line << std::endl;

    }

    return 0;
}
