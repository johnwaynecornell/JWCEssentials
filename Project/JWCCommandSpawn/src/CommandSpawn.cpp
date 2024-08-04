#include "JWCCommandSpawn/CommandSpawn.h"

#include <cstring>

using namespace JWCEssentials;

namespace JWCEssentials {
    void utf8_string_struct_move(utf8_string_struct &This, utf8_string_struct &other);
}

namespace JWCCommandSpawn {

    CommandSpawn::Shell::Shell(Shell &other) {
        shell = other.shell;
        shell_switch = other.shell_switch;
    }

    CommandSpawn::Shell::Shell(Shell &&other) noexcept{
        utf8_string_struct_move(shell, other.shell);
        utf8_string_struct_move(shell_switch, other.shell_switch);
    }

    CommandSpawn::Shell::Shell(utf8_string_struct shell, utf8_string_struct shell_switch) {
        this->shell = shell;
        this->shell_switch = shell_switch;
    }

    CommandSpawn::Shell & CommandSpawn::Shell::operator=(Shell &other) {
        shell = other.shell;
        shell_switch = other.shell_switch;
        return *this;
    }

    CommandSpawn::Shell & CommandSpawn::Shell::operator=(Shell &&other) noexcept {
        if (&other != this) {
            utf8_string_struct_move(shell, other.shell);
            utf8_string_struct_move(shell_switch, other.shell_switch);
        }
        return *this;
    }

    CommandSpawn::CommandSpawn() = default;
    CommandSpawn::~CommandSpawn() = default;

    CommandSpawn::Shell CommandSpawn::GetShell_Python() {
        return { "python", "-c" };
    }

    void CommandSpawn::SetShell(Shell shell) {
        this->shell = shell;
    }

    void CommandSpawn::SetShellExplicit(utf8_string_struct shell, utf8_string_struct shell_switch) {
        if (shell.c_str == nullptr || shell_switch.c_str == nullptr) {
            throw std::invalid_argument("Shell and switch must not be null");
        }

        this->shell = { shell, shell_switch };
    }

    utf8_string_struct CommandSpawn::ToString(utf8_string_struct command) {

        if (shell.shell) {
            // Launch the command directly
            if (!command) {
                return shell.shell;

                throw std::invalid_argument("Command must not be null when shell is not set");
            } else {
                // Escape the command for command line
                utf8_string_struct escaped_command = escapeStringForCommandLine(command.c_str);
                // Decorate the command with the shell and switch
                return shell.shell + " " + shell.shell_switch + " " + escaped_command;
            }
            // Implement direct process launching logic here
        } else {
            if (!command) throw std::invalid_argument("Command must not be null when shell is not set");

            return command;
        }
    }

    utf8_string_struct CommandSpawn::ReadLine(E_PIPE targ) {
        if (END[targ]) return nullptr;

        std::string line;
        int ch;
        while ((ch = ReadByte(targ)) != EOF && ch != '\n') {
            if (ch != '\r') line += (char) ch;
        }
        if (ch == EOF && line.empty()) return utf8_string_struct();

        return line.c_str();
    }

    utf8_string_struct CommandSpawn::ReadToEnd(E_PIPE targ) {
        if (END[targ]) return nullptr;

        std::string output;
        int ch;
        while ((ch = ReadByte(targ)) != EOF) {
            output += (char) ch;
        }
        return output.c_str();
    }

    void CommandSpawn::WriteString(utf8_string_struct str)
    {
        for (int i =0; i<str.length && str[i] != 0; i++) {
            WriteByte(str[i]);
        }
    }

    void CommandSpawn::WriteLine(utf8_string_struct line)
    {
        WriteString(line);
        WriteString(PlatformLineEnding());
    }

    void dump_strings();

    utf8_string_struct_array CStyle_ParseByWhitespace(const utf8_string_struct& command) {
        std::vector<utf8_string_struct> args;
        std::string arg;
        char inQuotes = 0;
        bool escape = false;

        for (int i=0; i<command.length; i++) {
            char c = command[i];
            if (escape) {
                arg.push_back(c);
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (inQuotes) {
                if (c == inQuotes) {
                    inQuotes = 0; // End of quoted string
                } else {
                    arg.push_back(c);
                }
            } else if (c == '\'' || c == '"') {
                inQuotes = c; // Start of quoted string
            } else if (isspace(c)) {
                if (!arg.empty()) {
                    args.emplace_back(arg.c_str());
                    arg.clear();
                }
            } else {
                arg.push_back(c);
            }
        }

        if (!arg.empty()) {
            args.emplace_back(arg.c_str());
        }

        utf8_string_struct_array R;
        R.Alloc(args.size());
        for (int i=0; i < args.size(); i++) {
            R[i] = args[i].c_str;
        }

        return R;
    }


    void CommandSpawn_Destroy(CommandSpawn *This) {
        delete This;
    }



    CommandSpawn::Shell CommandSpawn_GetShell_Defaultl(CommandSpawn *This) {
        return This->GetShell_Defaultl();
    }

    CommandSpawn::Shell CommandSpawn_GetShell_Bash(CommandSpawn *This) {
        return This->GetShell_Bash();
    }

    CommandSpawn::Shell CommandSpawn_GetShell_Python(CommandSpawn *This) {
        return This->GetShell_Python();
    }

    bool CommandSpawn_HasShell(CommandSpawn *This, CommandSpawn::Shell shell) {
        return This->HasShell(shell);
    }

    void SetCommandSpawn_SetShell(CommandSpawn *This, CommandSpawn::Shell shell) {
        This->SetShell(shell);
    }

    void SetCommandSpawn_SetShellExplicit(CommandSpawn *This, utf8_string_struct shell, utf8_string_struct shell_switch) {
        This->SetShellExplicit(shell, shell_switch);
    }

    bool CommandSpawn_Command(CommandSpawn *This, utf8_string_struct command,  CommandSpawn::E_PIPE pipes) {
        return This->Command(command, pipes);
    }

    long CommandSpawn_Join(CommandSpawn * This) {
        return This->Join();
    }

    utf8_string_struct CommandSpawn_ToString(CommandSpawn * This, utf8_string_struct command) {
        return This->ToString(command);
    }



    bool CommandSpawn_HasData(CommandSpawn *This, CommandSpawn::E_PIPE targ) {
        return This->HasData(targ);
    }

    int CommandSpawn_ReadByte(CommandSpawn *This, CommandSpawn::E_PIPE targ) {
        return This->ReadByte(targ);
    }

    utf8_string_struct CommandSpawn_ReadLine(CommandSpawn *This, CommandSpawn::E_PIPE targ) {
        return This->ReadLine(targ);
    }

    utf8_string_struct CommandSpawn_ReadToEnd(CommandSpawn *This, CommandSpawn::E_PIPE targ) {
        return This->ReadToEnd(targ);
    }

    void CommandSpawn_WriteByte(CommandSpawn *This, char byte) {
        This->WriteByte(byte);
    }

    void CommandSpawn_WriteString(CommandSpawn *This, utf8_string_struct string) {
        This->WriteString(string);
    }

    void CommandSpawn_WriteLine(CommandSpawn *This, utf8_string_struct line) {
        This->WriteLine(line);
    }
}