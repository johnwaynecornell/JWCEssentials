#include "JWCEssentials/JWCEssentials.h"

#include <string>
#include <sstream>

namespace JWCEssentials {
    utf8_string_struct escapeStringForCommandLine_Windows(utf8_string_struct text) {
        std::ostringstream escaped;
        escaped << '"';

        std::string input = std::string(text.c_str);

        for (size_t i = 0; i < input.size(); ++i) {
            char c = input[i];

            if (c == '\\') {
                size_t numBackslashes = 0;

                // Count the number of consecutive backslashes
                while (i < input.size() && input[i] == '\\') {
                    ++i;
                    ++numBackslashes;
                }

                // If this is the end of the string or the next character is a quote, double the backslashes
                if (i == input.size() || input[i] == '"') {
                    numBackslashes *= 2;
                }

                // Output the backslashes
                escaped << std::string(numBackslashes, '\\');

                // If we've reached the end, break out of the loop
                if (i == input.size()) {
                    break;
                }

                // Output the quote if the next character is a quote
                if (input[i] == '"') {
                    escaped << '\\';
                }

                // Output the next character
                --i; // to adjust for the outer loop increment
            } else if (c == '"') {
                // If the character is a quote, escape it
                escaped << '\\' << '"';
            } else {
                // Output the character as is
                escaped << c;
            }
        }

        escaped << '"';
        return escaped.str().c_str();
    }

    utf8_string_struct escapeStringForCommandLine_Linux(utf8_string_struct text) {
        std::ostringstream escaped;
        escaped << '\'';

        std::string input = std::string(text.c_str);

        for (char c : input) {
            if (c == '\'') {
                // End the current single-quoted string and add an escaped single quote
                escaped << "'\\''";
            } else {
                escaped << c;
            }
        }

        escaped << '\'';
        return escaped.str().c_str();
    }
}