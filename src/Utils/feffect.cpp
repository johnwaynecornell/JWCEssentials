//
// Created by jwc on 7/26/24.
//

#include <cstring>

#include "JWCEssentials/JWCEssentials.h"

#include <iostream>

using namespace JWCEssentials;

int main(int argc, char **argv) {
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

    for (i=1; i<argc; i++) {
        std::string a = argv[i];
        if (a == "-e") escape = "\033";
        else if (!command) command = a.c_str();
        else break;
    }

    if (!command || i != argc) {
        std::cout << "usage: feffect effect_string" << std::endl;
        std::cout << "  the first two examples are equivelent" << std::endl;
        std::cout << "example: feffect \"underline.italic(\\Hello World!\\\") fg_red blink(\\\" <---\\\") fg_default\"" << std::endl;;
        std::cout << "example: feffect \"underline(italic(\\Hello World!\\\") fg_red blink(\\\" <---\\\")) fg_default\"" << std::endl;;
        std::cout << "example: feffect \"underline(italic(\\Hello World!\\\") fg_red blink(\\\" <---\\\")) fg_default\"" << std::endl;;
        std::cout << "example: feffect \"reverse.underline.italic(\\Hello World!\\\") fg_red blink(\\\" <---\\\") fg_default)\"" << std::endl;;
        std::cout << "example: feffect \"reverse \\\"reversed\\\" reverse_off\"" << std::endl;
        std::cout << "example: feffect -list" << std::endl;
        return 1;
    }

    //feffect("reset underline.italic(\"Hello World!\") fg_red blink(\" <---\")");
    //utf8_string_struct result = feffect("reverse.blink(\"#*$&^&*^$#\" fg_red.italic.bold(\"EXAMPLE ERROR\"))");
    utf8_string_struct result = feffect(command, escape);
    if (!result) return 1;
    std::cout << result << std::endl;

    result = feffect(command, "\033");
    if (!result) return 1;
    std::cout << result << std::endl;

    //res = feffect("underline(italic(\"Hello World!\")) \"test\"");

    return 0;
}
