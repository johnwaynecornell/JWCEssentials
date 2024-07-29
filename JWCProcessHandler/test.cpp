//
// Created by jwc on 7/26/24.
//

#include "test.h"

#include <iostream>

int main() {
    ProcessHandler *proc = ProcessHandler_Create();

    proc->StartProcess("dir /s", ProcessHandler::E_PIPE_NONE);

    proc->StartProcess("dir /s", ProcessHandler::E_PIPE_STDOUT);

    utf8_string_handle line;
    while ((line = proc->ReadLine(ProcessHandler::E_PIPE_STDOUT)) != nullptr) {
        std::cout << "got " << line << std::endl;

    }

    return 0;
}
