//
// Created by jwc on 7/26/24.
//

#include "test.h"

#include <iostream>

using namespace JWCEssentials;

struct test {
    utf8_string_struct A;
    utf8_string_struct B;
};

int main() {
    test A = { "this","pointer"};

    test B = A;

    printf("%s\n", A.A == B.A ? "fail" : "pass");

    printf("%s\n", feffect("fg_red('iuyhdf')").c_str);


    utf8_string_struct string = "hello";

    utf8_string_struct string2 = string;

    utf8_string_struct res;

    Random_MT19937 * gen = new Random_MT19937();
    gen->SetSeed(32);

    for (int i=0; i<16; i++) {
        gen->Get_uint32_t();
    }

    char hex[]="0123456789ABCDEF";

    struct_array_struct<uint8_t> state = gen->get_state();

    for (int i=0; i<4; i++) {
        std::cout << gen->Get_uint32_t() << std::endl;
    }

    int val = 1;
    bool little = ((char *)&val)[0] == 1;

    std::string Out = "";

    for (int i=0; i<8; i++) {
        uint_fast32_t reg = gen->Get_uint32_t();
        for (int i=0; i<4; i++) {
            utf8_string_struct t;
            t.Alloc(2);

            t[0] =hex[reg & ((1<<4)-1)]; reg >>= 4;
            t[1] =hex[reg & ((1<<4)-1)]; reg >>= 4;

            if (little) Out = t.c_str + Out;
            else Out += t.c_str;
        }
    }

    std::cout << gen->cstyle_identifier(7) << std::endl;
    std::cout << Out << std::endl;

    Out = "";

    delete gen;
    gen = nullptr;

    gen = new Random_MT19937();
    gen->SetSeed(45634356);
    gen->set_state(state);

    for (int i=0; i<4; i++) {
        std::cout << gen->Get_uint32_t() << std::endl;
    }

    for (int i=0; i<8; i++) {
        uint_fast32_t reg = gen->Get_uint32_t();
        for (int i=0; i<4; i++) {
            utf8_string_struct t;
            t.Alloc(2);

            t[0] =hex[reg & ((1<<4)-1)]; reg >>= 4;
            t[1] =hex[reg & ((1<<4)-1)]; reg >>= 4;

            if (little) Out = t.c_str + Out;
            else Out += t.c_str;
        }
    }

    std::cout << gen->cstyle_identifier(7) << std::endl;
    std::cout << Out << std::endl;

    return 0;
}
