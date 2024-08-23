// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
#include "JWCEssentials/JWCEssentials.h"

namespace JWCEssentials {
    Random_Generator::Random_Generator()
    {

    }

    utf8_string_struct Random_Generator::cstyle_identifier(int length) {
        utf8_string_struct first = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        utf8_string_struct second = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

        utf8_string_struct Return;
        Return.Alloc(length);
        if (length != 0) {
            Return[0] = first[GetByte() % first.length];
            for (int i=1; i<length; i++) {
                Return[i] = second[GetByte() % second.length];
            }
        }
        return Return;
    }


    void Random_Generator::SetSeed(uint32_t seed)
    {
        ByesRemain = 0;
    }

    uint8_t Random_Generator::GetByte()
    {
        if (ByesRemain == 0)
        {
            ByteRegister = Get_uint64_t();
            ByesRemain = 4;
        }

        ByesRemain--;
        uint8_t R = (uint8_t)(ByteRegister & 0xFF);
        ByteRegister >>= 8;

        return R;
    }

    void Random_Generator_SetSeed(Random_Generator *This, uint32_t seed) {
        This->SetSeed(seed);
    }

    uint32_t Random_Generator_Get_uint32_t(Random_Generator *This) {
        return This->Get_uint32_t();
    }

    uint64_t Random_Generator_Get_uint64_t(Random_Generator *This) {
        return This->Get_uint64_t();
    }

    utf8_string_struct Random_Generator_cstyle_identifier(Random_Generator *This, int length) {
        return This->cstyle_identifier(length);
    }

    void Random_Generator_Reset(Random_Generator *This) {
        This->Reset();
    }

    struct_array_struct<uint8_t> Random_Generator_get_state(Random_Generator *This) {
        return This->get_state();
    }

    void Random_Generator_set_state(Random_Generator *This, struct_array_struct<uint8_t> state) {
        This->set_state(state);
    }

    uint8_t Random_Generator_GetByte(Random_Generator *This) {
        return This->GetByte();
    }
}