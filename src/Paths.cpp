#include "JWCEssentials/JWCEssentials.h"

#include <algorithm>
#include <regex>

namespace JWCEssentials {
    // Convert Platform_Windows path to Linux path
    utf8_string_struct WindowsToLinux(const utf8_string_struct &path) {
        std::string normalized_path = path.c_str;
        std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');
        std::regex drive_letter_regex(R"(^([a-zA-Z]):/)");
        normalized_path = std::regex_replace(normalized_path, drive_letter_regex, "/$1/");
        std::transform(normalized_path.begin(), normalized_path.end(), normalized_path.begin(), ::tolower);
        return normalized_path.c_str();
    }

    // Convert Linux path to Platform_Windows path
    utf8_string_struct LinuxToWindows(const utf8_string_struct &path) {
        std::string normalized_path = path.c_str;
        std::replace(normalized_path.begin(), normalized_path.end(), '/', '\\');
        std::regex linux_path_regex(R"(^/([a-zA-Z])/)");
        normalized_path = std::regex_replace(normalized_path, linux_path_regex, "$1:/");
        return normalized_path.c_str();
    }

    utf8_string_struct * LPath::explicit_handle() const {
        return (utf8_string_struct *) &handle;
    }

    utf8_string_struct * WPath::explicit_handle() const {
        return (utf8_string_struct *) &handle;
    }

    LPath::LPath() = default;

    LPath::LPath(WPath windows_path)
    {
        *explicit_handle() = WindowsToLinux(*windows_path.explicit_handle());
    }

    LPath::operator WPath() const
    {
        WPath R;
        *R.explicit_handle() = LinuxToWindows(*explicit_handle());
        return R;
    }

    WPath::WPath() = default;

    WPath::WPath(LPath linux_path)
    {
        *explicit_handle() = LinuxToWindows(*linux_path.explicit_handle());
    }

    WPath::operator LPath() const
    {
        LPath R;
        *R.explicit_handle() = WindowsToLinux(*explicit_handle());
        return R;
    }
}