#include "../ProcessHandler.h"
#include "Args.h"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// Function to split a string by spaces into a vector of strings
std::vector<std::string> splitString(const std::string& command) {
    std::vector<std::string> args;
    std::string arg;
    bool inQuotes = false;

    for (char c : command) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!arg.empty()) {
                args.push_back(arg);
                arg.clear();
            }
        } else {
            arg.push_back(c);
        }
    }
    if (!arg.empty()) {
        args.push_back(arg);
    }

    return args;
}

// Function to convert a vector of strings to a format compatible with execv
char** execvArgs_convertTo(const utf8_string_handle &command_line) {

    const std::vector<std::string> args = splitString(command_line);
    char** argv = new char*[args.size() + 1];
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = new char[args[i].size() + 1];
        std::strcpy(argv[i], args[i].c_str());
    }
    argv[args.size()] = nullptr; // Null-terminate the array

    return argv;
}

// Function to free the memory allocated for execv arguments
void execvArgs_free(char** argv) {
    for (size_t i = 0; argv[i]; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
}

