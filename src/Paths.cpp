#include "JWCEssentials/JWCEssentials.h"

#include <algorithm>
#include <regex>

// Convert Platform_Windows path to Linux path
utf8_string_handle WindowsToLinux(const utf8_string_handle &path) {
    std::string normalized_path = path;
    std::replace(normalized_path.begin(), normalized_path.end(), '\\', '/');
    std::regex drive_letter_regex(R"(^([a-zA-Z]):/)");
    normalized_path = std::regex_replace(normalized_path, drive_letter_regex, "/$1/");
    std::transform(normalized_path.begin(), normalized_path.end(), normalized_path.begin(), ::tolower);
    return normalized_path;
}

// Convert Linux path to Platform_Windows path
utf8_string_handle LinuxToWindows(const utf8_string_handle &path) {
    std::string normalized_path = path;
    std::replace(normalized_path.begin(), normalized_path.end(), '/', '\\');
    std::regex linux_path_regex(R"(^/([a-zA-Z])/)");
    normalized_path = std::regex_replace(normalized_path, linux_path_regex, "$1:/");
    return normalized_path;
}

utf8_string_handle * LPath::explicit_handle() const {
    return (utf8_string_handle *) &handle;
}

utf8_string_handle * WPath::explicit_handle() const {
    return (utf8_string_handle *) &handle;
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
