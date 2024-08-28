//
// Created by jwc on 8/22/24.
//

#include <cstring>
#include <iostream>
#include <ostream>

bool match(char *&in, char *string) {
    int len = strlen(string);
    if (strlen(in) < len) return false;

    for (int i=0; i<len; i++) {
        if (string[i] != in[i]) return false;
    }

    in += len;
    return true;
}

int main(int argc, char **argv) {

    char *input = argv[1];
    char *divider;

    if (argc == 3) divider = argv[2];
    else divider = "/";

    //std::cerr << "argc : \"" << argc << "\" input : \"" << input << "\" divider : \"" << divider << "\"" << std::endl;

    bool has = false;

    char *next;
    for (char *I = input; *I != '\0'; I=next) {
        next = I+1;

        if (match(I, divider)) {
            next = I;
            has = false;
            printf("\n");
            fflush(stdout);

      //      std::cout << "\r\n";
        } else {
            std::cout << *I;
            has = true;
        }
    }

    if (has) printf("\n");
    fflush(stdout);

    return 0;
}
