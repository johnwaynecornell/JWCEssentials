#ifndef JWCESSENTIALS_ESCAPES_H
#define JWCESSENTIALS_ESCAPES_H

namespace JWCEssentials {
    _EXPORT_ utf8_string_struct escapeStringForCommandLine(utf8_string_struct string);
    _EXPORT_ utf8_string_struct escapeStringForCommandLine_Windows(utf8_string_struct string);
    _EXPORT_ utf8_string_struct escapeStringForCommandLine_Linux(utf8_string_struct string);
};
#endif //JWCESSENTIALS_ESCAPES_H
