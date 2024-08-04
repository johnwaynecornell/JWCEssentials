//
// Created by jwc on 7/30/24.
//

#ifndef ARGS_H
#define ARGS_H

char** execvArgs_convertTo(const utf8_string_struct &command_line);
void execvArgs_free(char** argv);

#endif //ARGS_H
