#include "JWCEssentials/JWCEssentials.h"

namespace JWCEssentials {
    utf8_string_struct WindowsToLinux(const utf8_string_struct &path);
    utf8_string_struct LinuxToWindows(const utf8_string_struct &path);

    utf8_string_struct escapeStringForCommandLine(utf8_string_struct string)
    {
        return escapeStringForCommandLine_Linux(string);
    }

    LPath::LPath(utf8_string_struct native_path)
    {
        handle = native_path;
    }

    LPath::operator utf8_string_struct() const
    {
        return handle;
    }

    LPath LPath::FromNative(utf8_string_struct native)
    {
        return native;
    }

    utf8_string_struct LPath::ToNative()
    {
        return handle;
    }

    WPath::WPath(utf8_string_struct wpath) {
        handle = wpath;
    }

    WPath::operator utf8_string_struct() const
    {
        return handle;
    }

    WPath WPath::FromNative(utf8_string_struct native)
    {
        return LinuxToWindows(native);
    }

    utf8_string_struct WPath::ToNative()
    {
        return WindowsToLinux(handle);
    }
};