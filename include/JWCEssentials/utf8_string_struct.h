#ifndef UTF8_STRING_HANDLE_H
#define UTF8_STRING_HANDLE_H

#include <string>
#include <iostream>
#include <algorithm>
#include <atomic>

namespace JWCEssentials {
#pragma pack(push, 0)

    struct utf8_string_struct {
        static std::atomic_llong allocated_utf8_strings;

        char *c_str = nullptr;
        size_t length = 0;

        void (*free_c_str)(const char *c_str) = nullptr;

        // Default constructor
        _CLASSEXPORT_ utf8_string_struct();

        // Copy constructor
        _CLASSEXPORT_ utf8_string_struct(const utf8_string_struct &other);

        _CLASSEXPORT_ utf8_string_struct(utf8_string_struct &&other) noexcept;

        _CLASSEXPORT_ utf8_string_struct(const char *c_str);

        _CLASSEXPORT_ explicit utf8_string_struct(const std::string &other);

        _CLASSEXPORT_ explicit operator std::string();

        _CLASSEXPORT_ char &operator[](size_t index) const;

        //_CLASSEXPORT_ utf8_string_struct(utf8_string_struct&& other) noexcept;

        // Move constructor
        //_CLASSEXPORT_ utf8_string_struct(utf8_string_struct&& other) noexcept;
        // Allocate memory
        _CLASSEXPORT_ void Alloc(size_t length);

        // Release memory
        _CLASSEXPORT_ void Release();

        // Destructor
        _CLASSEXPORT_ ~utf8_string_struct();

        _CLASSEXPORT_ operator char *() const;

        _CLASSEXPORT_ explicit operator bool() const;

        _CLASSEXPORT_ utf8_string_struct &operator=(utf8_string_struct &other);

        _CLASSEXPORT_ utf8_string_struct &operator=(utf8_string_struct &&other) noexcept;

        _CLASSEXPORT_ utf8_string_struct &operator=(const char *c_str);

        _CLASSEXPORT_ void verify_contained();
    };

#pragma pack(pop)

    _EXPORT_ utf8_string_struct PlatformLineEnding();
    _CLASSEXPORT_ void utf8_string_struct_move(utf8_string_struct &This, utf8_string_struct &other);

//_CLASSEXPORT_ JWCEssentials::utf8_string_struct operator"" _utf8_string_struct(const char *source, size_t length);
}

_CLASSEXPORT_ JWCEssentials::utf8_string_struct operator+(const JWCEssentials::utf8_string_struct &A, const JWCEssentials::utf8_string_struct &B);

#endif //UTF8_STRING_HANDLE_H
