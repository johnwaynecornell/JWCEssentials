#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Mercury.h"
#include "MercuryCPU.h"

static void test_shift_multiples_of_32(void *stack, int Precision) {
    printf("Testing shift multiples of 32...\n");
    uint a[34], val[34];
    // Load 1.0 at place 2 (i.e. 2^64)
    uint one = 1;
    mercuryLoadRaw(stack, Precision, a, false, 2, &one, 1);
    
    // Shift right by 32 bits -> should have exponent 1 (2^32)
    mercuryShift(stack, Precision, a, -32, val);
    printf("  a exp: %d, val exp after >> 32: %d (expected 1)\n", (int)a[1], (int)val[1]);
    assert((int)val[1] == 1);
    assert(val[2 + Precision - 1] == 1);

    // Shift right by 64 bits -> should have exponent 0 (2^0)
    mercuryShift(stack, Precision, a, -64, val);
    printf("  a exp: %d, val exp after >> 64: %d (expected 0)\n", (int)a[1], (int)val[1]);
    assert((int)val[1] == 0);
    assert(val[2 + Precision - 1] == 1);

    // Shift left by 32 bits -> should have exponent 3 (2^96)
    mercuryShift(stack, Precision, a, 32, val);
    printf("  a exp: %d, val exp after << 32: %d (expected 3)\n", (int)a[1], (int)val[1]);
    assert((int)val[1] == 3);
    assert(val[2 + Precision - 1] == 1);

    // Shift left by 64 bits -> should have exponent 4 (2^128)
    mercuryShift(stack, Precision, a, 64, val);
    printf("  a exp: %d, val exp after << 64: %d (expected 4)\n", (int)a[1], (int)val[1]);
    assert((int)val[1] == 4);
    assert(val[2 + Precision - 1] == 1);
}

static void test_shift_values(void *stack, int Precision) {
    printf("Testing shift bit values (1, 31, 32, 33, 63, 64 and negatives)...\n");
    int shifts[] = {0, 1, 31, 32, 33, 63, 64, -1, -31, -32, -33, -63, -64};
    int num_shifts = sizeof(shifts)/sizeof(shifts[0]);

    for (int i = 0; i < num_shifts; i++) {
        int s = shifts[i];
        uint a[34], val[34], expected[34], pow2[34];
        // Load a known value: 3.5
        mercuryLoadDouble(stack, Precision, a, 3.5);
        mercuryShift(stack, Precision, a, s, val);

        // Compute expected = a * 2^s
        mercury_2Pow(stack, Precision, s, pow2);
        mercuryMul(stack, Precision, a, pow2, expected);

        int cmp = mercuryCmp(Precision, val, expected);
        if (cmp != 0) {
            char val_str[1024], exp_str[1024];
            mercuryToString(stack, Precision, val, val_str, 1024);
            mercuryToString(stack, Precision, expected, exp_str, 1024);
            printf("  FAILED for shift %d: got %s, expected %s\n", s, val_str, exp_str);
            assert(cmp == 0);
        } else {
            printf("  Shift %3d PASSED\n", s);
        }
    }
}

static void test_aliasing(void *stack, int Precision) {
    printf("Testing in-place aliasing (val == a)...\n");
    uint a[34], expected[34];
    mercuryLoadDouble(stack, Precision, a, 12345.6789);
    mercuryShift(stack, Precision, a, 17, expected);
    mercuryShift(stack, Precision, a, 17, a); // in-place
    assert(mercuryCmp(Precision, a, expected) == 0);

    mercuryShift(stack, Precision, a, -17, a); // back in-place
    mercuryLoadDouble(stack, Precision, expected, 12345.6789);
    assert(mercuryCmp(Precision, a, expected) == 0);
    printf("  Aliasing PASSED\n");
}

static void test_zero_and_low_precision(void *stack) {
    printf("Testing zero and low precision...\n");
    for (int prec = 1; prec <= 4; prec++) {
        uint z[8], res[8];
        mercuryLoadZero(stack, prec, z);
        mercuryShift(stack, prec, z, 15, res);
        assert(mercuryIsZero(prec, res));
        mercuryShift(stack, prec, z, -15, res);
        assert(mercuryIsZero(prec, res));

        // Test non-zero power of 2 with low precision
        uint val64 = 64;
        mercuryLoadRaw(stack, prec, z, false, 0, &val64, 1);
        mercuryShift(stack, prec, z, 5, res); // 64 * 32 = 2048
        uint expected_mul = 2048;
        uint exp_res[8];
        mercuryLoadRaw(stack, prec, exp_res, false, 0, &expected_mul, 1);
        assert(mercuryCmp(prec, res, exp_res) == 0);

        mercuryShift(stack, prec, z, -2, res); // 64 / 4 = 16
        uint exp_div = 16;
        mercuryLoadRaw(stack, prec, exp_res, false, 0, &exp_div, 1);
        assert(mercuryCmp(prec, res, exp_res) == 0);

        mercuryShift(stack, prec, z, -6, res); // 64 / 64 = 1
        uint exp_one = 1;
        mercuryLoadRaw(stack, prec, exp_res, false, 0, &exp_one, 1);
        assert(mercuryCmp(prec, res, exp_res) == 0);
    }
    printf("  Zero and low precision PASSED\n");
}

static void test_adjacent_limb_carries(void *stack, int Precision) {
    printf("Testing carrying bits between adjacent limbs...\n");
    // Value spanning multiple limbs: 0x80000000_00000001
    uint digits[2] = {1, 0x80000000U};
    uint a[34], res[34], pow2[34], expected[34];
    mercuryLoadRaw(stack, Precision, a, false, 1, digits, 2);

    int test_shifts[] = {1, 15, 31, 33, -1, -15, -31, -33};
    for (size_t i = 0; i < sizeof(test_shifts)/sizeof(test_shifts[0]); i++) {
        int s = test_shifts[i];
        mercuryShift(stack, Precision, a, s, res);
        mercury_2Pow(stack, Precision, s, pow2);
        mercuryMul(stack, Precision, a, pow2, expected);
        assert(mercuryCmp(Precision, res, expected) == 0);
    }
    printf("  Adjacent limb carries PASSED\n");
}

int main(void) {
    long stack_size = 1024 * 1024 * 4;
    void *stack = mercuryAllocateStack(stack_size);
    *((long *)stack) = 0;
    int Precision = 32;

    test_shift_multiples_of_32(stack, Precision);
    test_shift_values(stack, Precision);
    test_aliasing(stack, Precision);
    test_zero_and_low_precision(stack);
    test_adjacent_limb_carries(stack, Precision);

    printf("ALL SHIFT TESTS PASSED SUCCESSFULLY!\n");
    mercuryDeallocateStack(stack);
    return 0;
}
