#ifndef JWCESSENTIALS_PATHS_H
#define JWCESSENTIALS_PATHS_H

namespace JWCEssentials {
    struct LPath;
    struct WPath;

#pragma pack(push, 0)
    struct LPath
    {
        utf8_string_struct handle;

        utf8_string_struct * explicit_handle() const;

        LPath();

        LPath(utf8_string_struct lpath);
        operator utf8_string_struct() const;

        LPath(WPath windows_path);
        operator WPath() const;

        static LPath FromNative(utf8_string_struct native);
        utf8_string_struct ToNative();

    };
#pragma pack(pop)

#pragma pack(push, 0)
    struct WPath
    {
        utf8_string_struct handle;

        utf8_string_struct * explicit_handle() const;

        WPath();

        WPath(utf8_string_struct wpath);
        operator utf8_string_struct() const;

        WPath(LPath linux_path);
        operator LPath() const;

        static WPath FromNative(utf8_string_struct native);
        utf8_string_struct ToNative();
    };

#pragma pack(pop)

#ifdef _WIN32
#define _Path WPath
    //#define _PathI LPath
#else
#define _Path LPath
    //#define _PathI WPath
#endif
};

#endif //JWCESSENTIALS_PATHS_H
