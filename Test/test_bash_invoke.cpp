#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>
#include "../JWCProcessHandler/ProcessHandler.h"

/*
class BashProcessHandler : public ProcessHandler {
public:
    void StartProcess(const std::string &command) override {
        std::string bashCommand = "C:\\Program Files\\Git\\git-bash.exe -c \"" + command + "\"";

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Use CreateProcess to run bash without creating a new window
        if (!CreateProcess(
                NULL,
                const_cast<char*>(bashCommand.c_str()), // Command line
                NULL,                                   // Process security attributes
                NULL,                                   // Primary thread security attributes
                FALSE,                                  // Handles are inherited
                CREATE_NO_WINDOW,                       // Creation flags
                NULL,                                   // Use parent's environment block
                NULL,                                   // Use parent's starting directory
                &si,                                    // Pointer to STARTUPINFO structure
                &pi                                     // Pointer to PROCESS_INFORMATION structure
        )) {
            throw std::runtime_error("CreateProcess failed");
        }

        // Wait until the child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
};
*/
int main() {

    ProcessHandler *handler = ProcessHandler_Create();
    try {
        handler->StartProcess("echo Hello, World!", ProcessHandler::E_PIPE_STDOUT);
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    utf8_string_handle line;
    while ((line = handler->ReadLine(ProcessHandler::E_PIPE_STDOUT)) != nullptr) {
        std::cout << "got " << line << std::endl;

    }




    return 0;
}
