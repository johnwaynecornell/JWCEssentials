//
// Created by jwc on 7/26/24.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "JWCEssentials/JWCEssentials.h"
#include "JWCEssentials/JWCMain.h"

#include "JWCEssentials/JWCEssentials.h"

using namespace JWCEssentials;

static bool ReadTextFile(const std::string& filename, std::string& out)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file)
    {
        std::cerr << "feffect: could not open file: " << filename << std::endl;
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

int JWCMain(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "-list") == 0) {
        utf8_string_struct_array effect = feffect_list();

        int i;
        for (i=0; i < effect.length - 1; i++) {
            std::cout << effect[i].c_str;
            if (i % 5 == 0) std::cout <<  std::endl;
            else std::cout << "\t";
        }

        std::cout << effect[i].c_str << std::endl;
        return 1;
    }

    int i;

    utf8_string_struct command;
    utf8_string_struct escape = "\\033";

    bool emit_escape = false;

    std::vector<std::string> commands;

    for (int i = 1; i < argc; i++)
    {
        std::string a = argv[i];

        if (a == "-e")
        {
            escape = "\033";
            emit_escape = true;
        }
        else if (a == "-file")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "feffect: -file requires a filename" << std::endl;
                return 1;
            }

            std::string filename = argv[++i];
            std::string fileText;

            if (!ReadTextFile(filename, fileText))
                return 1;

            commands.push_back(fileText);
        }
        else
        {
            commands.push_back(a);
        }
    }

    if (commands.empty())
    {
        std::cout << "usage: feffect [-e] [-file filename] [effect_string ...]" << std::endl;
        std::cout << "example: feffect \"fg_green('Hello')\"" << std::endl;
        std::cout << "example: feffect -e \"fg_green('Hello')\" -file banner.feffect" << std::endl;
        std::cout << "  the next two examples are equivalent" << std::endl;
        std::cout << "example: feffect \"underline.italic(\\\"Hello World\\!\\\") fg_red blink(\\\" <---\\\") fg_default\"" << std::endl;;
        std::cout << "example: feffect \"underline(italic(\\\"Hello World\\!\\\") fg_red blink(\\\" <---\\\")) fg_default\"" << std::endl;;
        std::cout << "example: feffect \"underline(italic(\\\"Hello World\\!\\\") fg_red blink(\\\" <---\\\")) fg_default\"" << std::endl;;
        std::cout << "example: feffect \"reverse.underline.italic(\\\"Hello World\\!\\\") fg_red blink(\\\" <---\\\") fg_default\"" << std::endl;;
        std::cout << "example: feffect \"reverse \\\"reversed\\\" reverse_off\"" << std::endl;
        std::cout << "example: feffect -list" << std::endl;
        return 1;
    }

    for (const std::string& command : commands)
    {
        utf8_string_struct commandUtf8 = command.c_str();
        utf8_string_struct result = feffect(commandUtf8, escape);

        if (!result)
            return 1;

        std::cout << result;
    }

    std::cout << std::endl;
    return 0;
}
