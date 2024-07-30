// JWCEssentials.h
#pragma once
#include <string>

#include "JWCEssentials/JWCEssentials.h"

class ProcessHandler {
public:
    enum E_PIPE
    {
        E_PIPE_NONE=0,
        E_PIPE_STDOUT=1,
        E_PIPE_STDERR=2,
        E_PIPE_STDIN=4
    };

    bool END[4] = {};

    utf8_string_handle shell = nullptr;

    _CLASSEXPORT_ virtual ~ProcessHandler() = default;

    _CLASSEXPORT_ virtual void StartProcess(utf8_string_handle command, E_PIPE pipes) = 0;

    _CLASSEXPORT_ virtual bool HasData(E_PIPE targ) = 0;
    _CLASSEXPORT_ virtual int ReadByte(E_PIPE targ) = 0;
    _CLASSEXPORT_ virtual utf8_string_handle ReadLine(E_PIPE targ);
    _CLASSEXPORT_ virtual utf8_string_handle ReadToEnd(E_PIPE targ);

    _CLASSEXPORT_ virtual void WriteByte(char byte) = 0;
    _CLASSEXPORT_ virtual void WriteString(utf8_string_handle string);
    _CLASSEXPORT_ virtual void WriteLine(utf8_string_handle line);
};

extern "C"
{
    _EXPORT_ ProcessHandler *ProcessHandler_Create();
    _EXPORT_ void ProcessHandler_Destroy(ProcessHandler *This, ProcessHandler *Handler);

    _EXPORT_ void ProcessHandler_StartProcess(ProcessHandler *This, utf8_string_handle command_line,  ProcessHandler::E_PIPE pipes);

    _EXPORT_ bool ProcessHandler_HasData(ProcessHandler *This, ProcessHandler::E_PIPE targ);
    _EXPORT_ int ProcessHandler_ReadByte(ProcessHandler *This, ProcessHandler::E_PIPE targ);
    _EXPORT_ utf8_string_handle ProcessHandler_ReadLine(ProcessHandler *This, ProcessHandler::E_PIPE targ);
    _EXPORT_ utf8_string_handle ProcessHandler_ReadToEnd(ProcessHandler *This, ProcessHandler::E_PIPE targ);

    _EXPORT_ void ProcessHandler_WriteByte(ProcessHandler *This, char byte);
    _EXPORT_ void ProcessHandler_WriteString(ProcessHandler *This, utf8_string_handle string);
    _EXPORT_ void ProcessHandler_WriteLine(ProcessHandler *This, utf8_string_handle line);
}