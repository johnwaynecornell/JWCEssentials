#ifndef STRUCT_ARRAY_STRUCT_H
#define STRUCT_ARRAY_STRUCT_H

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace JWCEssentials {
#pragma pack(push, 0)

    template <typename T> struct struct_array_struct
    {
        static std::atomic_llong allocated_structs;

        T *elems = nullptr;
        size_t length = 0;
        using FreeFunc = void (*)(T *elems);
        FreeFunc free_elems = nullptr;

        // Default constructor
        struct_array_struct();

        // Copy constructor
        struct_array_struct(const struct_array_struct &other);
        struct_array_struct(const std::vector<T> &other);

        // Move constructor
        struct_array_struct(struct_array_struct &&other) noexcept;

        T& operator[](size_t index) const;

        // Allocate memory
        void Alloc(size_t length);

        // Release memory
        void Release();

        // Destructor
        ~struct_array_struct();

        operator T *() const;
        explicit operator bool() const;

        operator std::vector<T>();

        struct_array_struct &operator=(const struct_array_struct &other);
        struct_array_struct &operator=(struct_array_struct &&other) noexcept;
    };

#pragma pack(pop)

    template <typename T> std::atomic_llong struct_array_struct<T>::allocated_structs(0);

    // Free function for arrays
    template <typename T> void my_free_structs(T *elems)
    {
        delete[] elems;
    }

    // Default constructor
    template <typename T> struct_array_struct<T>::struct_array_struct() = default;

    // Copy constructor
    template <typename T> struct_array_struct<T>::struct_array_struct(const struct_array_struct &other)
    {
        if (other) {
            Alloc(other.length);
            for (size_t i = 0; i < length; ++i) {
                (*this)[i] = other.elems[i];
            }
        }
    }

    // Move helper function
    template <typename T> void struct_array_struct_move(struct_array_struct<T> &This, struct_array_struct<T> &other) {
        if (&This != &other) {
            This.Release();

            This.elems = other.elems;
            This.length = other.length;
            This.free_elems = other.free_elems;

            other.elems = nullptr;
            other.length = 0;
            other.free_elems = nullptr;
        }
    }

    // Move constructor
    template <typename T> struct_array_struct<T>::struct_array_struct(struct_array_struct &&other) noexcept {
        struct_array_struct_move(*this, other);
    }

    // Operator[] for accessing elements
    template <typename T> T& struct_array_struct<T>::operator[](size_t index) const {
        if (index >= length) throw std::runtime_error("index out of bounds");
        return elems[index];
    }

    // Allocate memory
    template <typename T> void struct_array_struct<T>::Alloc(size_t length)
    {
        Release();

        this->length = length;

        elems = new T[length];
        free_elems = my_free_structs<T>;
        allocated_structs.fetch_add(1);
    }

    // Release memory
    template <typename T> void struct_array_struct<T>::Release()
    {
        if (elems) {
            if (free_elems) {
                allocated_structs.fetch_sub(1);
                free_elems(elems);
            } else {
                std::cerr << "free callback not set for array: " << elems << std::endl;
            }
        }
        elems = nullptr;
        length = 0;
        free_elems = nullptr;
    }

    // Destructor
    template <typename T> struct_array_struct<T>::~struct_array_struct() {
        Release();
    }

    // Conversion operators
    template <typename T> struct_array_struct<T>::operator T *() const {
        return elems;
    }

    template <typename T> struct_array_struct<T>::operator bool() const {
        return elems != nullptr;
    }

    template <typename T> struct_array_struct<T>::struct_array_struct(const std::vector<T> &other) {
        size_t size = other.size();
        Alloc(size);
        for (int i=0; i<size; i++) (*this)[i] = other[i];
    }

    template <typename T> struct_array_struct<T>::operator std::vector<T>() {
        std::vector<T> R;

        for (int i=0; i<length; i++) {
            R.emplace_back((*this)[i]);
        }

        return R;
    }


    // Copy assignment operator
    template <typename T> struct_array_struct<T>& struct_array_struct<T>::operator=(const struct_array_struct &other) {
        if (this != &other) {
            if (other.elems) {
                Alloc(other.length);
                for (size_t i = 0; i < length; ++i) (*this)[i] = other[i];
            } else {
                Release();
            }
        }
        return *this;
    }

    // Move assignment operator
    template <typename T> struct_array_struct<T>& struct_array_struct<T>::operator=(struct_array_struct &&other) noexcept
    {
        if (this != &other) {
            struct_array_struct_move(*this, other);
        }
        return *this;
    }

    // Explicit template instantiation for utf8_string_struct
    template class struct_array_struct<utf8_string_struct>;
    typedef struct_array_struct<utf8_string_struct> utf8_string_struct_array;
};
#endif // STRUCT_ARRAY_STRUCT_H
