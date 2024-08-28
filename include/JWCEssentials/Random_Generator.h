// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
#ifndef HASHPRNG_RANDOM_GENERATOR_H
#define HASHPRNG_RANDOM_GENERATOR_H

#include <cstdint>
#include <cstdlib>

namespace JWCEssentials {
    class _CLASSEXPORT_ Random_Generator
    {
    public:
        virtual ~Random_Generator() = default;

        Random_Generator();
        virtual void SetSeed(uint32_t seed);
        virtual uint32_t GetSeed();

        virtual uint32_t Get_uint32_t() = 0;
        virtual uint64_t Get_uint64_t() = 0;
        virtual double Get_double();
        virtual utf8_string_struct cstyle_identifier(int length);
        virtual void Reset() = 0;

        virtual struct_array_struct<uint8_t> get_state() = 0;
        virtual void set_state(struct_array_struct<uint8_t> state) = 0;

        virtual uint8_t GetByte();
    protected:
        uint32_t seed;
        uint32_t ByteRegister;
        int32_t ByesRemain;
    };

    _EXPORT_ void Random_Generator_SetSeed(P_INSTANCE(Random_Generator) This, uint32_t seed);
    _EXPORT_ uint32_t Random_Generator_GetSeed(P_INSTANCE(Random_Generator) This);

    _EXPORT_ uint32_t Random_Generator_Get_uint32_t(P_INSTANCE(Random_Generator) This);
    _EXPORT_ uint64_t Random_Generator_Get_uint64_t(P_INSTANCE(Random_Generator) This);
    _EXPORT_ double Random_Generator_Get_double(P_INSTANCE(Random_Generator) This);

    _EXPORT_ utf8_string_struct Random_Generator_cstyle_identifier(P_INSTANCE(Random_Generator) This, int length);
    _EXPORT_ void Random_Generator_Reset(P_INSTANCE(Random_Generator) This);

    _EXPORT_ struct_array_struct<uint8_t> Random_Generator_get_state(P_INSTANCE(Random_Generator) This);
    _EXPORT_ void Random_Generator_set_state(P_INSTANCE(Random_Generator) This, struct_array_struct<uint8_t> state);

    _EXPORT_ uint8_t Random_Generator_GetByte(P_INSTANCE(Random_Generator) This);

    _EXPORT_ void Random_Destroy(P_INSTANCE(Random_Generator) This);


}

#endif