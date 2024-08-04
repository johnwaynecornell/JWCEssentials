#include "../JWCEssentials.h"

namespace JWCEssentials {
utf8_string_struct WindowsToLinux(const utf8_string_struct &path);
utf8_string_struct LinuxToWindows(const utf8_string_struct &path);

utf8_string_struct escapeStringForCommandLine(utf8_string_struct string)
{
    return escapeStringForCommandLine_Windows(string);
}

LPath::LPath(utf8_string_struct lpath)
{
    handle = lpath;
}

LPath::operator utf8_string_struct() const
{
    return handle;
}

LPath LPath::FromNative(utf8_string_struct native)
{
    return WindowsToLinux(native);
}

utf8_string_struct LPath::ToNative()
{
    return LinuxToWindows(handle);
}

WPath::WPath(utf8_string_struct native_path)
{
    handle = native_path;
}

WPath::operator utf8_string_struct() const
{
    return handle;
}

WPath WPath::FromNative(utf8_string_struct native)
{
    return native;
}

utf8_string_struct WPath::ToNative()
{
    return handle;
}
}