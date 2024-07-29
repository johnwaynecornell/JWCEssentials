#include "ProcessHandler.h"
#include "Intern.h"

#include <cstring>
#include <stdexcept>

utf8_string_handle ProcessHandler::ReadLine(E_PIPE targ) {
    if (END[targ]) return nullptr;

    std::string line;
    int ch;
    while ((ch = ReadByte(targ)) != EOF && ch != '\n') {
        if (ch != '\r') line += (char) ch;
    }
    if (ch == EOF && line.empty()) return utf8_string_handle();

    return line;
}

utf8_string_handle ProcessHandler::ReadToEnd(E_PIPE targ) {
    if (END[targ]) return nullptr;

    std::string output;
    int ch;
    while ((ch = ReadByte(targ)) != EOF) {
        output += (char) ch;
    }
    return output;
}

void ProcessHandler::WriteString(utf8_string_handle str)
{
    for (int i =0; i<str.length && str[i] != 0; i++) {
        WriteByte(str[i]);
    }
}

void ProcessHandler::WriteLine(utf8_string_handle line)
{
    WriteString(line);
    WriteString(PlatformLineEnding());
}


void ProcessHandler_StartProcess(ProcessHandler *This, utf8_string_handle command,  ProcessHandler::E_PIPE pipes) {
    This->StartProcess(command, pipes);
}

bool ProcessHandler_HasData(ProcessHandler *This, ProcessHandler::E_PIPE targ) {
    return This->HasData(targ);
}

int ProcessHandler_ReadByte(ProcessHandler *This, ProcessHandler::E_PIPE targ) {
    return This->ReadByte(targ);
}

utf8_string_handle ProcessHandler_ReadLine(ProcessHandler *This, ProcessHandler::E_PIPE targ) {
    return This->ReadLine(targ);
}

utf8_string_handle ProcessHandler_ReadToEnd(ProcessHandler *This, ProcessHandler::E_PIPE targ) {
    return This->ReadToEnd(targ);
}

void ProcessHandler_WriteByte(ProcessHandler *This, char byte) {
    This->WriteByte(byte);
}

void ProcessHandler_WriteString(ProcessHandler *This, utf8_string_handle string) {
    This->WriteString(string);
}

void ProcessHandler_WriteLine(ProcessHandler *This, utf8_string_handle line) {
    This->WriteLine(line);
}
