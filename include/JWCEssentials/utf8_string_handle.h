//
// Created by johnw on 7/28/2024.
//

#ifndef PROCESSHANDLER_UTF8_STRING_HANDLE_H
#define PROCESSHANDLER_UTF8_STRING_HANDLE_H

#include <string>
#include <iostream>
#include <algorithm>

#pragma pack(push, 0)
struct utf8_string_handle
{
    char *c_str = nullptr;
    size_t length = 0;
    void (*free_c_str)(char * c_str) = nullptr;

    _CLASSEXPORT_ void verify_contained();
    // Default constructor
    _CLASSEXPORT_ utf8_string_handle();

    // Copy constructor
    _CLASSEXPORT_ utf8_string_handle(const utf8_string_handle &other);

    // Move constructor
    _CLASSEXPORT_ utf8_string_handle(utf8_string_handle&& other) noexcept;

    // Constructor from const char*
    _CLASSEXPORT_ utf8_string_handle(const char *source);
    _CLASSEXPORT_ utf8_string_handle(std::string source);

    // Constructor from const char* with length
    _CLASSEXPORT_ utf8_string_handle(const char *source, size_t length);

    // Copy assignment operator
    _CLASSEXPORT_ utf8_string_handle& operator=(const utf8_string_handle &other);

    // Move assignment operator
    _CLASSEXPORT_ utf8_string_handle& operator=(utf8_string_handle&& other) noexcept;

    // Allocate memory
    _CLASSEXPORT_ void Alloc(size_t length);

    // Release memory
    _CLASSEXPORT_ void Release();

    // Destructor
    _CLASSEXPORT_ ~utf8_string_handle();

    // Implicit conversion to char*
    _CLASSEXPORT_ operator char*() const;
    // Implicit conversion to std::string
    _CLASSEXPORT_ operator std::string() const;

};
#pragma pack(pop)

_CLASSEXPORT_ utf8_string_handle operator +(utf8_string_handle A, utf8_string_handle B);

_EXPORT_ utf8_string_handle PlatformLineEnding();

#endif //PROCESSHANDLER_UTF8_STRING_HANDLE_H
