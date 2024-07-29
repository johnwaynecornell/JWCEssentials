//
// Created by johnw on 7/28/2024.
//

#ifndef PROCESSHANDLER_ESCAPES_H
#define PROCESSHANDLER_ESCAPES_H



_EXPORT_ utf8_string_handle escapeStringForCommandLine(utf8_string_handle string);
_EXPORT_ utf8_string_handle escapeStringForCommandLine_Windows(utf8_string_handle string);
_EXPORT_ utf8_string_handle escapeStringForCommandLine_Linux(utf8_string_handle string);

#endif //PROCESSHANDLER_ESCAPES_H
