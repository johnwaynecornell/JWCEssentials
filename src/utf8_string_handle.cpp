#include <cstring>

#include "JWCEssentials/JWCEssentials.h"

#include <string>
#include <sstream>

// Default constructor
utf8_string_handle::utf8_string_handle()
{

}

// Copy constructor
utf8_string_handle::utf8_string_handle(const utf8_string_handle &other)
{
    if (other.c_str) {
        Alloc(other.length);
        std::copy(other.c_str, other.c_str + length, c_str);
    }
}

// Move constructor
utf8_string_handle::utf8_string_handle(utf8_string_handle&& other) noexcept
        : c_str(other.c_str), length(other.length)
{
    other.c_str = nullptr;
    other.length = 0;
}

// Constructor from const char*
utf8_string_handle::utf8_string_handle(const char *source) {
    if (source) {
        length = std::strlen(source);
        Alloc(length);
        std::copy(source, source + length, c_str);
    } else Release();
}

// Constructor from const char* with length
utf8_string_handle::utf8_string_handle(const char *source, size_t length) {
    if (source) {
        size_t _length = std::min(length, std::strlen(source));
        Alloc(_length);
        std::copy(source, source + _length, c_str);
    }
}

utf8_string_handle::utf8_string_handle(std::string source) : utf8_string_handle(source.c_str())
{

}

// Copy assignment operator
utf8_string_handle& utf8_string_handle::operator=(const utf8_string_handle &other)
{
    if (this != &other) {
        if (other.c_str) {
            Alloc(other.length);
            std::copy(other.c_str, other.c_str + length, c_str);
        } else {
            Release();
        }
    }
    return *this;
}

// Move assignment operator
utf8_string_handle& utf8_string_handle::operator=(utf8_string_handle&& other) noexcept
{
    if (this != &other) {
        Release();
        c_str = other.c_str;
        length = other.length;
        other.c_str = nullptr;
        other.length = 0;
    }
    return *this;
}

void utf8_string_handle::verify_contained() {
    int I;

    for (I=0; I<length+1; I++) {
        if (c_str[I] == 0) return;
    }

    throw std::runtime_error("ERROR Memory corruption unterminated string");
}


void my_free(char *c_str)
{
    delete[] c_str;
}

// Allocate memory
void utf8_string_handle::Alloc(size_t length)
{
    Release();
    this->length = length;

    c_str = new char[length + 1];
    c_str[0] = c_str[length] = 0;

    free_c_str = my_free;
}

// Release memory
void utf8_string_handle::Release()
{
    if (free_c_str && c_str) {
        printf("utf8_string_handle::Release %016llX\n", (unsigned long long) c_str);
        free_c_str(c_str);
    }
    c_str = nullptr;
    length = 0;
}

// Destructor
utf8_string_handle::~utf8_string_handle() {
    Release();
}

// Implicit conversion to char*
utf8_string_handle::operator char*() const {
    return c_str;
}

// Implicit conversion to std::string
//utf8_string_handle::operator std::string() const {
//    return std::string(c_str);
//}


// Implicit conversion from std::string to utf8_string_handle
utf8_string_handle operator"" _utf8_string_handle(const char* str, size_t len) {
    return utf8_string_handle(str, len);
}

utf8_string_handle operator +(utf8_string_handle A, utf8_string_handle B) {
    utf8_string_handle R;
    R.Alloc(A.length + B.length);
    int o = 0;
    for (int i=0; i<A.length; i++) R[o++] = A[i];
    for (int i=0; i<B.length; i++) R[o++] = B[i];
    return R;
}

utf8_string_handle PlatformLineEnding()
{
    std::ostringstream o;
    o << std::endl;
    return o.str();
}