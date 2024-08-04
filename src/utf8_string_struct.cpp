#include <cstring>

#include "JWCEssentials/JWCEssentials.h"

#include <string>
#include <sstream>
#include <vector>

namespace JWCEssentials {
    utf8_string_struct utf8_string_struct_make(const char *source, size_t length) {
        utf8_string_struct R;

        if (source) {
            size_t _length = std::min(length, std::strlen(source));
            R.Alloc(_length);
            std::copy(source, source + _length, R.c_str);
        }

        return R;
    }

    void my_free(const char *c_str)
    {
        delete[] c_str;
    }

    // Default constructor
    utf8_string_struct::utf8_string_struct()
    = default;

    // Copy constructor
    utf8_string_struct::utf8_string_struct(const utf8_string_struct &other)
    {
        if (other) {
            Alloc(other.length);
            std::copy(other.c_str, other.c_str + length, c_str);
        }
    }

    void utf8_string_struct_move(utf8_string_struct &This, utf8_string_struct &other) {
        if (&This != &other) {
            This.Release();
            This.c_str = other.c_str;
            This.length = other.length;
            This.free_c_str = other.free_c_str;

            other.c_str = nullptr;
            other.length = 0;
            other.free_c_str = nullptr;
        }
    }

    utf8_string_struct::utf8_string_struct(utf8_string_struct &&other)
     noexcept {
        utf8_string_struct_move(*this, other);
    }



    // Constructor from const char*
    utf8_string_struct::utf8_string_struct(const char *source) {
        if (source) {
            size_t length = std::strlen(source);
            Alloc(length);
            std::copy(source, source + length, c_str);
        }
    }

    /*
    // Move constructor
    utf8_string_struct::utf8_string_struct(utf8_string_struct&& other) noexcept
            : c_str(other.c_str), length(other.length)
    {
        other.c_str = nullptr;
        other.length = 0;
    }
    */

    char & utf8_string_struct::operator[](size_t index) const{
        if (index >= length) throw std::runtime_error("index out of bounds");
        return c_str[index];
    }

    std::vector<const char *> utf8_strings;
    std::atomic_llong utf8_string_struct::allocated_utf8_strings = 0;
    // Allocate memory
    void utf8_string_struct::Alloc(size_t length)
    {
        Release();
        utf8_string_struct::allocated_utf8_strings.fetch_add(1);

        this->length = length;

        c_str = new char[length + 1];
        c_str[0] = c_str[length] = 0;

        free_c_str = my_free;

        utf8_strings.push_back(c_str);
    }

    void dump_strings() {
        size_t count = (unsigned long long) utf8_string_struct::allocated_utf8_strings;

        if (count != 0) {
            fprintf(stderr, "utf8_string_struct::allocated_utf8_string=%lld\n", count);

            fprintf(stderr,"DANGLED STRINGS:\n");
            for (auto i=utf8_strings.begin(); i != utf8_strings.end(); i++) {
                fprintf(stderr,"\t|%s\n", *i);
            }
        }
    }

    // Release memory
    void utf8_string_struct::Release()
    {
        if (c_str) {
            if (free_c_str) {
                utf8_string_struct::allocated_utf8_strings.fetch_sub(1);
                for (auto i=utf8_strings.begin(); i != utf8_strings.end(); i++) if (*i == c_str) {
                    utf8_strings.erase(i);
                    break;
                }
                free_c_str(c_str);
            } else {
                std::cerr << "free callback not set string: " << c_str;
            }
        }
        c_str = nullptr;
        length = 0;
        free_c_str = nullptr;
    }

    // Destructor
    utf8_string_struct::~utf8_string_struct() {
        Release();
    }

    utf8_string_struct::operator char *() const {
        return c_str;
    }

    utf8_string_struct::operator bool() const {
        return c_str != nullptr;
    }

    utf8_string_struct &utf8_string_struct::operator =(utf8_string_struct &other) {
        if (other.c_str != c_str) {
            if (other.c_str) {
                Alloc(other.length);
                std::copy(other.c_str, other.c_str + length, c_str);
            } else Release();
        }
        return *this;
    }

    utf8_string_struct & utf8_string_struct::operator =(const char * c_str) {
        if (c_str) {
            size_t length = std::strlen(c_str);

            Alloc(length);
            std::copy(c_str, c_str + length, this->c_str);
        } else Release();
        return *this;
    }

    /*

    // Copy assignment operator
    utf8_string_struct& operator=(utf8_string_struct &This, const utf8_string_struct &other)
    {
        if (&This != &other) {
            if (other.c_str) {
                This.Alloc(other.length);
                std::copy(other.c_str, other.c_str + other.length, This.c_str);
            } else {
                This.Release();
            }
        }
        return This;
    }*/


    // Move assignment operator
    utf8_string_struct& utf8_string_struct::operator=(utf8_string_struct&& other) noexcept
    {
        if (this != &other) {
            utf8_string_struct_move(*this, other);
        }
        return *this;
    }


    void utf8_string_struct::verify_contained() {
        int I;

        for (I=0; I<length+1; I++) {
            if (c_str[I] == 0) return;
        }

        throw std::runtime_error("ERROR Memory corruption unterminated string");
    }



    // Implicit conversion to std::string
    //utf8_string_struct::operator std::string() const {
    //    return std::string(c_str);
    //}

    utf8_string_struct PlatformLineEnding()
    {
        std::ostringstream o;
        o << std::endl;
        return o.str().c_str();
    }
}

JWCEssentials::utf8_string_struct operator+(const JWCEssentials::utf8_string_struct &A, const JWCEssentials::utf8_string_struct &B){
    JWCEssentials::utf8_string_struct R;
    R.Alloc(A.length + B.length);
    int o = 0;
    for (int i=0; i<A.length; i++) R[o++] = A[i];
    for (int i=0; i<B.length; i++) R[o++] = B[i];
    return R;
}
