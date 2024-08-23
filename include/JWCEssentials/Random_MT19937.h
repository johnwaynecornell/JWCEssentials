// MIT License
// Copyright (c) 2024 John W. Cornell
// See LICENSE file in the project root for full license information.
#ifndef HASHPRNG_RANDOM_MT19937_H
#define HASHPRNG_RANDOM_MT19937_H

#include "Random_Generator.h"

namespace JWCEssentials {
    class _CLASSEXPORT_ Random_MT19937 : public Random_Generator
    {
    private:
        const int32_t N = 624;
        const int32_t M = 397;
        const uint32_t MATRIX_A = 0x9908b0dfU;
        const uint32_t UPPER_MASK = 0x80000000U;
        const uint32_t LOWER_MASK = 0x7fffffffU;

        uint32_t *mt;
        uint32_t *mt2;
        int32_t mti = N + 1;
    public:

        _CLASSEXPORT_ Random_MT19937();

        _CLASSEXPORT_ ~Random_MT19937() override;
        _CLASSEXPORT_ void Reset() override;

        _CLASSEXPORT_ void SetSeed(uint32_t seed) override;

        _CLASSEXPORT_ struct_array_struct<uint8_t> get_state() override;
        _CLASSEXPORT_ void set_state(struct_array_struct<uint8_t> state) override;

        _CLASSEXPORT_ uint32_t Get_uint32_t() override;
        _CLASSEXPORT_ uint64_t Get_uint64_t() override;
    };

    _EXPORT_ P_INSTANCE(Random_MT19937) Random_MT19937_Create(uint32_t seed);
}

#endif //HASHPRNG_RANDOM_MT19937_H
