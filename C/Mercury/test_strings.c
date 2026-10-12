#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include "Mercury.h"
#include "MercuryCPU.h"

static void test_hex_did(void) {
    assert(mercuryHexDid('0') == 0);
    assert(mercuryHexDid('9') == 9);
    assert(mercuryHexDid('A') == 10);
    assert(mercuryHexDid('F') == 15);
    assert(mercuryHexDid('a') == 10);
    assert(mercuryHexDid('f') == 15);
    assert(mercuryHexDid('g') == -1);
    assert(mercuryHexDid('@') == -1);
    assert(mercuryHexDid('+') == -1);
    assert(mercuryHexDid('-') == -1);
    printf("test_hex_did passed\n");
}

static void test_get_precision(void *stack) {
    // Basic numbers
    assert(mercuryHexStringGetPrecision(stack, "0") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "1") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "1.0") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "1.2345678") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "12345678") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "123456789") >= 2);

    // With exponent
    assert(mercuryHexStringGetPrecision(stack, "1.0@+100") >= 1);
    assert(mercuryHexStringGetPrecision(stack, "1.23456789ABCDEF01@+3FFFFFFFF") >= 2);
    assert(mercuryHexStringGetPrecision(stack, "1.0@-400000000") >= 1);

    // Negative exponent
    assert(mercuryHexStringGetPrecision(stack, "-1.5@-20") >= 1);

    printf("test_get_precision passed\n");
}

static void test_basic_serialization(void *stack) {
    const int Precision = 8;
    uint val[10];
    char buf[128];

    // Zero
    mercuryFromString(stack, Precision, "0", val);
    assert(mercuryIsZero(Precision, val));
    mercuryToString(stack, Precision, val, buf, sizeof(buf));
    assert(strcmp(buf, "0") == 0);

    // Positive integer
    mercuryFromString(stack, Precision, "1", val);
    assert(!mercuryIsZero(Precision, val));
    mercuryToString(stack, Precision, val, buf, sizeof(buf));
    assert(strcmp(buf, "1") == 0);

    // Negative integer
    mercuryFromString(stack, Precision, "-1", val);
    assert((val[0] & 1) == 1);
    mercuryToString(stack, Precision, val, buf, sizeof(buf));
    assert(strcmp(buf, "-1") == 0);

    // Simple fraction
    mercuryFromString(stack, Precision, ".5", val);
    mercuryToString(stack, Precision, val, buf, sizeof(buf));
    assert(strcmp(buf, ".5") == 0);

    // Negative fraction
    mercuryFromString(stack, Precision, "-.8", val);
    mercuryToString(stack, Precision, val, buf, sizeof(buf));
    assert(strcmp(buf, "-.8") == 0);

    printf("test_basic_serialization passed\n");
}

static void test_scientific_notation_roundtrip(void *stack) {
    const int Precision = 8;
    uint val1[10];
    uint val2[10];
    char buf[256];

    const char *test_cases[] = {
        "1@+0",
        "1@+1",
        "1@+7",
        "1@+8",
        "1@+F",
        "1@+10",
        "1@+17",
        "1@+18",
        "1@+20",
        "1@+100",
        "1@+1000",
        "1@+10000",
        "1@+100000",
        "1@+1000000",
        "1@+10000000",
        "1@+20000000",
        "1@+100000000",
        "1@+3FFFFFFFF",   // Upper boundary of representable range (INT32_MAX word exp)
        "-1@+3FFFFFFFF",
        "1@-10",
        "1@-20",
        "1@-100",
        "1@-1000",
        "1@-10000",
        "1@-100000",
        "1@-1000000",
        "1@-10000000",
        "1@-100000000",
        "1@-3FFFFFFC1",  // Lower boundary of representable range for P=8 (INT32_MIN + 7 word exp)
        "-1@-3FFFFFFC1",
        "3.243F6A8885A308D@+10",
        "2.B7E151628AED2A6@-20",
        "F.FFFFFFFFFFFFFFF@+100",
        "A.BCDEF0123456789@-50",
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        const char *input = test_cases[i];
        mercuryFromString(stack, Precision, input, val1);
        mercuryToString(stack, Precision, val1, buf, sizeof(buf));
        mercuryFromString(stack, Precision, buf, val2);

        assert(mercuryCmp(Precision, val1, val2) == 0);
        assert((val1[0] & 1) == (val2[0] & 1));
        assert(val1[1] == val2[1]);
    }

    printf("test_scientific_notation_roundtrip passed\n");
}

static void test_boundary_exponents(void *stack) {
    // 1. Test precision P=1: min exponent is INT32_MIN (0x80000000)
    {
        const int Precision = 1;
        uint val[3];
        uint val_parsed[3];
        char buf[256];

        // Valid boundary: word exp = INT32_MIN (-0x80000000), nibble 0 (hex exp -0x400000000)
        uint limb = 1U;
        mercuryLoadRaw(stack, Precision, val, false, (int)0x80000000U, &limb, 1);
        mercuryToString(stack, Precision, val, buf, sizeof(buf));
        assert(strstr(buf, "@-400000000") != NULL);

        mercuryFromString(stack, Precision, buf, val_parsed);
        assert(mercuryCmp(Precision, val, val_parsed) == 0);
        assert((int)val_parsed[1] == (int)0x80000000U);

        // First invalid exponent below P=1 boundary: hex exp -0x400000001
        mercuryFromString(stack, Precision, "1@-400000001", val_parsed);
        assert(mercuryIsZero(Precision, val_parsed));
    }

    // 2. Test precision P=8: min exponent is INT32_MIN + 8 - 1 = -2147483641 (-0x7FFFFFF9)
    {
        const int Precision = 8;
        uint val[10];
        uint val_parsed[10];
        char buf[256];

        // Max positive boundary: word exp = INT32_MAX (0x7FFFFFFF), hex exp @+3FFFFFFFF
        uint limbHigh = 0x10000000U; // nibble 7
        mercuryLoadRaw(stack, Precision, val, false, 0x7FFFFFFF, &limbHigh, 1);
        mercuryToString(stack, Precision, val, buf, sizeof(buf));
        assert(strstr(buf, "@+3FFFFFFFF") != NULL);

        mercuryFromString(stack, Precision, buf, val_parsed);
        assert(mercuryCmp(Precision, val, val_parsed) == 0);
        assert((int)val_parsed[1] == 0x7FFFFFFF);

        // First invalid positive exponent: hex exp +0x400000000
        mercuryFromString(stack, Precision, "1@+400000000", val_parsed);
        assert(mercuryIsZero(Precision, val_parsed));

        // Exact min valid boundary for P=8: word exp = INT32_MIN + 7, nibble 0 (hex exp -0x3FFFFFFC8)
        uint limbLow = 1U;
        int minExp8 = (int)0x80000000U + 7;
        mercuryLoadRaw(stack, Precision, val, false, minExp8, &limbLow, 1);
        mercuryToString(stack, Precision, val, buf, sizeof(buf));
        assert(strstr(buf, "@-3FFFFFFC8") != NULL);

        mercuryFromString(stack, Precision, buf, val_parsed);
        assert(mercuryCmp(Precision, val, val_parsed) == 0);
        assert((int)val_parsed[1] == minExp8);

        // First invalid negative exponent for P=8: hex exp -0x3FFFFFFC9 (word exp INT32_MIN + 6)
        mercuryFromString(stack, Precision, "1@-3FFFFFFC9", val_parsed);
        assert(mercuryIsZero(Precision, val_parsed));

        // Underflow exponent (-0x400000000 is INT32_MIN, invalid for P=8)
        mercuryFromString(stack, Precision, "1@-400000000", val_parsed);
        assert(mercuryIsZero(Precision, val_parsed));
    }

    // 3. Test precision P=32: min exponent is INT32_MIN + 32 - 1 = -2147483617 (-0x7FFFFFE1)
    {
        const int Precision = 32;
        uint val[34];
        uint val_parsed[34];
        char buf[256];

        int minExp32 = (int)0x80000000U + 31;
        uint limbLow = 1U;
        mercuryLoadRaw(stack, Precision, val, false, minExp32, &limbLow, 1);
        mercuryToString(stack, Precision, val, buf, sizeof(buf));
        assert(strstr(buf, "@-3FFFFFF08") != NULL);

        mercuryFromString(stack, Precision, buf, val_parsed);
        assert(mercuryCmp(Precision, val, val_parsed) == 0);
        assert((int)val_parsed[1] == minExp32);

        // First invalid negative exponent for P=32: hex exp -0x3FFFFFF09
        mercuryFromString(stack, Precision, "1@-3FFFFFF09", val_parsed);
        assert(mercuryIsZero(Precision, val_parsed));
    }

    printf("test_boundary_exponents passed\n");
}

static void test_malformed_and_out_of_range(void *stack) {
    const int Precision = 8;
    uint val[10];

    // Malformed exponent with no digits
    mercuryFromString(stack, Precision, "1.5@", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.5@+", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.5@-", val);
    assert(mercuryIsZero(Precision, val));

    // Exponents beyond representable word range
    mercuryFromString(stack, Precision, "1.0@+400000000", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.0@-400000001", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.0@+FFFFFFFFFFFFFFFF", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.0@-FFFFFFFFFFFFFFFF", val);
    assert(mercuryIsZero(Precision, val));

    mercuryFromString(stack, Precision, "1.0@+100000000000000000000", val);
    assert(mercuryIsZero(Precision, val));

    printf("test_malformed_and_out_of_range passed\n");
}

int main(void) {
    void *stack = mercuryAllocateStack(1024 * 1024);
    *((slong *)stack) = 0;

    test_hex_did();
    test_get_precision(stack);
    test_basic_serialization(stack);
    test_scientific_notation_roundtrip(stack);
    test_boundary_exponents(stack);
    test_malformed_and_out_of_range(stack);

    mercuryDeallocateStack(stack);
    printf("ALL NATIVE STRING TESTS PASSED!\n");
    return 0;
}
