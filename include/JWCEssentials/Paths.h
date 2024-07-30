#ifndef PROCESSHANDLER_PATHS_H
#define PROCESSHANDLER_PATHS_H

struct LPath;
struct WPath;

#pragma pack(push, 0)
struct LPath
{
    utf8_string_handle handle;

    utf8_string_handle * explicit_handle() const;

    LPath();

    LPath(utf8_string_handle lpath);
    operator utf8_string_handle() const;

    LPath(WPath windows_path);
    operator WPath() const;

    static LPath FromNative(utf8_string_handle native);
    utf8_string_handle ToNative();

};
#pragma pack(pop)

#pragma pack(push, 0)
struct WPath
{
    utf8_string_handle handle;

    utf8_string_handle * explicit_handle() const;

    WPath();

    WPath(utf8_string_handle wpath);
    operator utf8_string_handle() const;

    WPath(LPath linux_path);
    operator LPath() const;

    static WPath FromNative(utf8_string_handle native);
    utf8_string_handle ToNative();
};

#pragma pack(pop)

#ifdef _WIN32
#define _Path WPath
//#define _PathI LPath
#else
#define _Path LPath
//#define _PathI WPath
#endif

#endif //PROCESSHANDLER_PATHS_H
