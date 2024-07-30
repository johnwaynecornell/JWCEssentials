#include "JWCEssentials/JWCEssentials.h"

utf8_string_handle WindowsToLinux(const utf8_string_handle &path);
utf8_string_handle LinuxToWindows(const utf8_string_handle &path);

utf8_string_handle escapeStringForCommandLine(utf8_string_handle string)
{
    return escapeStringForCommandLine_Linux(string);
}

LPath::LPath(utf8_string_handle native_path)
{
    handle = native_path;
}

LPath::operator utf8_string_handle() const
{
    return handle;
}

LPath LPath::FromNative(utf8_string_handle native)
{
    return native;
}

utf8_string_handle LPath::ToNative()
{
    return handle;
}

WPath::WPath(utf8_string_handle wpath) {
    handle = wpath;
}

WPath::operator utf8_string_handle() const
{
    return handle;
}

WPath WPath::FromNative(utf8_string_handle native)
{
    return LinuxToWindows(native);
}

utf8_string_handle WPath::ToNative()
{
    return WindowsToLinux(handle);
}
