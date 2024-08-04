#include "JWCCommandSpawn//CommandSpawn.h"
#include "Args.h"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace JWCEssentials;
using namespace JWCCommandSpawn;

// Function to convert a vector of strings to a format compatible with execv
char** execvArgs_convertTo(const utf8_string_struct &command_line) {

    const utf8_string_struct_array args = CStyle_ParseByWhitespace(command_line);
    char** argv = new char*[args.length + 1];
    for (size_t i = 0; i < args.length; ++i) {
        argv[i] = new char[args[i].length + 1];
        std::strcpy(argv[i], args[i].c_str);
    }
    argv[args.length] = nullptr; // Null-terminate the array

    return argv;
}

// Function to free the memory allocated for execv arguments
void execvArgs_free(char** argv) {
    for (size_t i = 0; argv[i]; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
}

