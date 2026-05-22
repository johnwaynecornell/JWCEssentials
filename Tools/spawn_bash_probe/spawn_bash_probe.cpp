#include <iostream>
#include <string>
#include "JWCCommandSpawn/CommandSpawn.h"

using namespace JWCCommandSpawn;
using namespace JWCEssentials;

int main() {
    CommandSpawn *spawn = CommandSpawn_Create();
    if (!spawn) {
        std::cerr << "Failed to create CommandSpawn" << std::endl;
        return 1;
    }

    try {
        spawn->SetShell(spawn->GetShell_Bash());

        // Prove JWCCommandSpawn can:
        // - invoke bash
        // - send stdin
        // - capture stdout
        
        // Command: cat | sed 's/^/spawned: /'
        // Input: hello from JWCCommandSpawn
        
        utf8_string_struct command = "cat | sed 's/^/spawned: /'";
        utf8_string_struct input = "hello from JWCCommandSpawn\n";
        
        // We want to capture STDOUT and provide STDIN manually
        if (!spawn->Command(command, nullptr, (CommandSpawn::E_PIPE)(CommandSpawn::E_PIPE_STDOUT | CommandSpawn::E_PIPE_STDIN))) {
            std::cerr << "Failed to execute command" << std::endl;
            CommandSpawn_Destroy(spawn);
            return 1;
        }

        // Send input and close STDIN so cat finishes
        spawn->WriteString(input);
        spawn->ClosePipe(CommandSpawn::E_PIPE_STDIN);

        // Read output
        utf8_string_struct line;
        while ((line = spawn->ReadLine(CommandSpawn::E_PIPE_STDOUT))) {
            std::cout << (const char*)line << std::flush;
        }

        spawn->Join();
        CommandSpawn_Destroy(spawn);
    } catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        CommandSpawn_Destroy(spawn);
        return 1;
    }

    return 0;
}
