#ifndef SPAWNEDPROCESS_H
#define SPAWNEDPROCESS_H

#include <string>
#include <vector>

#include "JWCEssentials/JWCEssentials.h"

using namespace JWCEssentials;

#ifdef BUILD_SPAWNEDPROCESS
    #define _EXPORT_ __EXPORT__
    #define _CLASSEXPORT_ __CLASSEXPORT__
#else
    #define _EXPORT_ __IMPORT__
    #define _CLASSEXPORT_ __CLASSIMPORT__
#endif

namespace JWCCommandSpawn {
    class _CLASSEXPORT_ CommandSpawn {
    public:
        enum E_PIPE
        {
            E_PIPE_NONE=0,
            E_PIPE_STDOUT=1,
            E_PIPE_STDERR=2,
            E_PIPE_STDIN=4
        };

        bool END[4] = {};

        struct Shell {
            utf8_string_struct shell = nullptr;
            utf8_string_struct shell_switch = nullptr;

            _CLASSEXPORT_ Shell(Shell &other);
            _CLASSEXPORT_ Shell(Shell &&other) noexcept;

            _CLASSEXPORT_ Shell(utf8_string_struct shell, utf8_string_struct shell_switch);

            _CLASSEXPORT_ Shell & operator = (Shell &other);
            _CLASSEXPORT_ Shell & operator = (Shell &&other) noexcept;

        };

        long last_return = 0;

        Shell shell = {nullptr, nullptr};

        CommandSpawn();

        _CLASSEXPORT_ virtual ~CommandSpawn();

        _CLASSEXPORT_ virtual Shell GetShell_Defaultl() = 0;
        _CLASSEXPORT_ virtual Shell GetShell_Bash() = 0;
        _CLASSEXPORT_ virtual Shell GetShell_Python();

        _CLASSEXPORT_ virtual bool HasShell(Shell shell) = 0;

        _CLASSEXPORT_ virtual void SetShell(Shell shell);
        _CLASSEXPORT_ virtual void SetShellExplicit(utf8_string_struct shell, utf8_string_struct shell_switch);
        _CLASSEXPORT_ virtual bool Command(utf8_string_struct command, E_PIPE pipes) = 0;

        _CLASSEXPORT_ virtual void Close() = 0;
        _CLASSEXPORT_ virtual long Join() = 0;

        _CLASSEXPORT_ virtual utf8_string_struct ToString(utf8_string_struct command);

        _CLASSEXPORT_ virtual bool HasData(E_PIPE targ) = 0;
        _CLASSEXPORT_ virtual int ReadByte(E_PIPE targ) = 0;
        _CLASSEXPORT_ virtual utf8_string_struct ReadLine(E_PIPE targ);
        _CLASSEXPORT_ virtual utf8_string_struct ReadToEnd(E_PIPE targ);

        _CLASSEXPORT_ virtual void WriteByte(char byte) = 0;
        _CLASSEXPORT_ virtual void WriteString(utf8_string_struct string);
        _CLASSEXPORT_ virtual void WriteLine(utf8_string_struct line);
    };

    utf8_string_struct_array CStyle_ParseByWhitespace(const utf8_string_struct& command);

    _EXPORT_ CommandSpawn *CommandSpawn_Create();
    _EXPORT_ void CommandSpawn_Destroy(CommandSpawn *This);

    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Defaultl(CommandSpawn *This);
    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Bash(CommandSpawn *This);
    _EXPORT_ CommandSpawn::Shell CommandSpawn_GetShell_Python(CommandSpawn *This);

    _EXPORT_ bool CommandSpawn_HasShell(CommandSpawn *This, CommandSpawn::Shell shell);

    _EXPORT_ void SetCommandSpawn_SetShell(CommandSpawn *This, CommandSpawn::Shell shell);
    _EXPORT_ void SetCommandSpawn_SetShellExplicit(CommandSpawn *This, utf8_string_struct shell, utf8_string_struct shell_switch);
    _EXPORT_ bool CommandSpawn_Command(CommandSpawn *This, utf8_string_struct command, CommandSpawn::E_PIPE pipes);

    _EXPORT_ long CommandSpawn_Join(CommandSpawn * This);

    _EXPORT_ utf8_string_struct CommandSpawn_ToString(CommandSpawn * This, utf8_string_struct command);

    _EXPORT_ bool CommandSpawn_HasData(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ int CommandSpawn_ReadByte(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ utf8_string_struct CommandSpawn_ReadLine(CommandSpawn * This, CommandSpawn::E_PIPE targ);
    _EXPORT_ utf8_string_struct CommandSpawn_ReadToEnd(CommandSpawn * This, CommandSpawn::E_PIPE targ);

    _EXPORT_ void CommandSpawn_WriteByte(CommandSpawn *This, char byte);
    _EXPORT_ void CommandSpawn_WriteString(CommandSpawn * This, utf8_string_struct string);
    _EXPORT_ void CommandSpawn_WriteLine(CommandSpawn * This, utf8_string_struct line);
}

#undef _EXPORT_
#undef  _CLASSEXPORT_

#endif