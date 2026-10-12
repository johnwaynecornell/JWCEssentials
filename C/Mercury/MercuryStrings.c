//
// Created by jwc on 9/22/21.
//

#include "Mercury.h"
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

int mercuryHexDid(int Ch)
{
    //int r = Ch;
    int r = -1;
    if (Ch >= '0' && Ch <= '9') r = Ch - '0';
    else if (Ch >= 'A' && Ch <= 'F') r = 10 + Ch - 'A';
    else if (Ch >= 'a' && Ch <= 'f') r = 10 + Ch - 'a';

    //return (Ch - 0);
    return r;
}

/*
 * Distinction between word exponents and hexadecimal digit exponents:
 * Mercury stores numbers in base-2^32 limbs (words).
 * A word exponent 'e' represents limb place (2^32)^e.
 * Since each 32-bit limb contains 8 hexadecimal digits (nibbles, base 16 = 2^4),
 * a limb exponent 'e' corresponds to hex digit places [e * 8, e * 8 + 7].
 * The highest hex place of limb 'e' is (e * 8 + 7), and word exponent
 * for a given hex place is floor(hexPlace / 8).
 */
static int64_t mercuryFloorDiv8(int64_t value)
{
    int64_t q = value / 8;
    int64_t r = value % 8;

    if (value < 0 && r != 0) q--;

    return q;
}

static int mercuryMod8Floor(int64_t value)
{
    int r = (int)(value % 8);
    if (r < 0) r += 8;
    return r;
}

int mercuryHexStringGetPrecision(void *stack, const char *Input)
{
    if (Input == NULL || *Input == '\0') {
        return 1;
    }

    int I;
    I = 0;

    if (Input[I] == '-' || Input[I] == '+') I++;

    int P;
    P = I;

    while (Input[I] != '\0' && Input[I] != '.' && Input[I] != '@' && mercuryHexDid(Input[I]) > -1)
    {
        I++;
    }

    while (Input[P] == '0')
    {
        P++;
    }

    int Begin = P;

    while (mercuryHexDid(Input[P]) > -1) P++;

    int E = P - Begin;
    int L = 0;

    if (Input[P] == '.')
    {
        int C = 0;
        P++;
        int H;
        while ((H = mercuryHexDid(Input[P])) != -1) {
            C++;
            P++;
        }
        L += C;
    }

    if (Input[P] == '@') {
        P++;
        bool negativeExponent = false;
        if (Input[P] == '-') {
            negativeExponent = true;
            P++;
        }
        else if (Input[P] == '+') {
            P++;
        }

        uint64_t V = 0;
        int did;

        while ((did = mercuryHexDid(Input[P])) != -1)
        {
            V = (V << 4) + (uint64_t)did;
            P++;
        }

        int shift = (int)(V % 8);
        if (negativeExponent)
        {
            L += shift;
            E -= shift;
        } else
        {
            E += shift;
            L -= shift;
        }
    }

    if (E < 0) { L -= E; E = 0; }
    if (L < 0) { E -= L; L = 0; }

    int Q = 0;

    Q = E / 8;
    if (E % 8 != 0) Q++;
    Q += L / 8;
    if (L % 8 != 0) Q++;

    if (Q <= 0) Q = 1;

    return Q;
}

void mercuryFromString(void *stack, int Precision, const char *Input, uint *val) {
    mercuryLoadZero(stack, Precision, val);

    if (Input == NULL || *Input == '\0') {
        return;
    }

    int Start = 0;

    bool negative = false;

    if (Input[Start] == '-')
    {
        negative = true;
        Start++;
    } else if (Input[Start] == '+') Start++;


    int Decimal = Start;
    while (Input[Decimal] != '\0'
           && Input[Decimal] != '.'
           && Input[Decimal] != '@'
           && mercuryHexDid(Input[Decimal]) > -1) Decimal++;
    bool hasDecimal = (Input[Decimal] == '.');

    int64_t sciNot = 0;
    bool exponentError = false;

    {
        int P = hasDecimal ? Decimal + 1 : Decimal;

        if (hasDecimal) {
            while (mercuryHexDid(Input[P]) != -1) P++;
        }

        if (Input[P] == '@') {
            bool negativeExponent = false;
            P++;

            if (Input[P] == '-') {
                negativeExponent = true;
                P++;
            } else if (Input[P] == '+') {
                P++;
            }

            int expHexDigits = 0;
            while (Input[P] == '0') {
                expHexDigits++;
                P++;
            }

            uint64_t V = 0;
            int did;

            while ((did = mercuryHexDid(Input[P])) != -1) {
                expHexDigits++;
                if (V > (UINT64_MAX >> 4)) {
                    exponentError = true;
                } else {
                    V = (V << 4) + (uint64_t)did;
                }
                P++;
            }

            if (expHexDigits == 0) {
                // Malformed exponent: '@' or '@+'/'@-' with no hex digits
                exponentError = true;
            } else if (V > (uint64_t)INT64_MAX) {
                exponentError = true;
            } else {
                sciNot = negativeExponent ? -(int64_t)V : (int64_t)V;
            }
        }
    }

    if (exponentError) {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    int S = Start;
    while (Input[S] == '0') S++;

    int wholeLen = 0;
    while (mercuryHexDid(Input[S + wholeLen]) > -1) wholeLen++;

    int Nibble;
    int64_t wordExp;

    if (wholeLen > 0) {
        int64_t highestHexPlace = (int64_t)wholeLen - 1 + sciNot;

        wordExp = mercuryFloorDiv8(highestHexPlace);
        Nibble = mercuryMod8Floor(highestHexPlace);
    } else if (hasDecimal)
    {
        int fracPos = 0;
        while (Input[Decimal + 1 + fracPos] == '0') fracPos++;

        S = Decimal + 1 + fracPos;

        if (mercuryHexDid(Input[S]) < 0)
        {
            mercuryLoadZero(stack, Precision, val);
            return;
        }

        fracPos++;

        int64_t highestHexPlace = -(int64_t)fracPos + sciNot;

        wordExp = mercuryFloorDiv8(highestHexPlace);
        Nibble = mercuryMod8Floor(highestHexPlace);
    } else
    {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    // Check if wordExp exceeds Mercury's representable signed 32-bit limb exponent range.
    // For precision P, the lowest supported exponent is INT32_MIN + P - 1 so that the lowest
    // limb place (wordExp - P + 1) does not underflow INT32_MIN.
    int64_t minWordExp = (int64_t)INT32_MIN + Precision - 1;
    if (wordExp > (int64_t)INT32_MAX || wordExp < minWordExp) {
        // Exponent out of representable range -> underflow to zero predictably
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    val[1] = (uint)(int)wordExp;

    int P = 0;
    int I = S;

    while (Input[I] != '\0' && P < Precision) {
        uint V = 0;

        int d = -1;
        for (int n = Nibble; n >= 0; n--) {
            if (I == Decimal) I++;

            d = mercuryHexDid(Input[I++]);
            if (d == -1) break;

            V |= ((uint)d << (n << 2));
        }

        val[2 + Precision - 1 - P++] = V;
        if (d == -1) break;

        Nibble = 7;
    }

    if (negative) val[0] |= 1;

    P = I;
}

int mercuryToString(void *stack, int Precision, uint *val, char *buffer, int len)
{
    /* TODO */ //update to proper toString & bounds check

    const char *PlaceValue = "0123456789ABCDEF";

    int p = 0;
    int l = len - 1;
    if (len > 0 && buffer != NULL) buffer[l] = 0;

    if ((val[0] & 1) == 1)
    {
        if (p < l && buffer != NULL) buffer[p] = '-';
        p++;
    }

    int e = (int) val[1];
    int64_t el = (int64_t)e - Precision + 1;

    while (el < 0 && (mercuryGetAt(Precision, val, (int)el) == 0)) el++;

    bool scientificNotation = false;

    if (e < -1 || el > 0)
    {
        scientificNotation = true;
    }

    int64_t i;

    bool lead = true;

    // Highest hex place corresponding to limb e is e * 8 + 7 = (e + 1) * 8 - 1
    int64_t E = ((int64_t)e + 1) * 8 - 1;

    int64_t D = scientificNotation ? 1 : E + 1;

    for (i = e; i >= el; i--) {
        uint d = mercuryGetAt(Precision, val, (int)i);

        if (e == el && d == 0)
        {
            if (p < l && buffer != NULL) buffer[p] = '0';
            p++;
            break;
        }

        int low = 0;

        if (i == el && el < 0)
        {
            while (low <= 7 && ((int)((d >> (low << 2)) & 0xF) == 0)) low++;
        }

        for (int pl = 7; pl >= low; pl--) {
            int x = (int)((d >> (pl << 2)) & 0xF);

            if (lead && x == 0) E--;

            if (!scientificNotation) {
                D--;

                if (D == -1) {
                    if (p < l && buffer != NULL) buffer[p] = '.';
                    p++;
                }
            }

            if (!lead || x != 0) {
                if (scientificNotation) {
                    D--;

                    if (D == -1) {
                        if (p < l && buffer != NULL) buffer[p] = '.';
                        p++;
                    }
                }

                if (p < l && buffer != NULL) buffer[p] = PlaceValue[x];
                p++;
                lead = false;
            }
        }
    }

    if (scientificNotation && !lead)
    {
        if (p < l && buffer != NULL) buffer[p] = '@';
        p++;

        if (p < l && buffer != NULL) buffer[p] = (E < 0) ? '-' : '+';
        p++;

        uint64_t d = (E < 0) ? (0ULL - (uint64_t)E) : (uint64_t)E;

        lead = true;
        for (int pl = (int)(sizeof(d) * 2 - 1); pl >= 0; pl--) {
            int x = (int)((d >> (pl << 2)) & 0xF);

            if (!lead || x != 0 || pl == 0) {
                if (p < l && buffer != NULL) buffer[p] = PlaceValue[x];
                p++;
                lead = false;
            }
        }
    }

    if (p < len && buffer != NULL) buffer[p] = 0;
    p++;
    return p;
}