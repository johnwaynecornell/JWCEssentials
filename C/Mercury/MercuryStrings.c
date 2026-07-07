//
// Created by jwc on 9/22/21.
//

#include "Mercury.h"

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

static int mercuryFloorDiv8(int value)
{
        int q = value / 8;
        int r = value % 8;

        if (value < 0 && r != 0) q--;

        return q;
    }

static int mercuryMod8Floor(int value)
{
        int r = value % 8;
        if (r < 0) r += 8;
        return r;
    }

int mercuryHexStringGetPrecision(void *stack, const char *Input)
{
    int I;
    I=0;

    if (Input[I]=='-' || Input[I]=='+') I++;

    int P;
    P=I;

    while (Input[I] != '\0' && Input[I] != '.' && Input[I] && mercuryHexDid(Input[I]) > -1)
    {
        I++;
    }

    while (Input[P] == '0')
    {
        P++;
    }

    int Begin = P;

    while (mercuryHexDid(Input[P]) > -1) P++;

    int E = P-Begin;
    int L = 0;

    if (Input[P] == '.')
    {
        int C = 0;
        P++;
        int H;
        do {
            H = mercuryHexDid(Input[P++]);
            if (H != -1) C++;
        } while (H != -1);

        L += C;
    }

    bool negativeExponent = false;

    if (Input[P] == '@') {
        P++;
        if (Input[P] == '-') {
            negativeExponent = true;
            P++;
        }
        else if (Input[P] == '+') {
            negativeExponent = false;
            P++;
        }

        long V = 0;

        int did;

        do
        {
            did = mercuryHexDid(Input[P]);
            if (did != -1) {
                V = (V << 4) + did;
                P++;
            }
        } while (did != -1);

        if (negativeExponent)
        {
            L += V % 8;
            E -= V % 8;
        } else
        {
            E += V % 8;
            L -= V % 8;
        }
    }

    if (E<0) { L -= E; E = 0; }
    if (L<0) { E -= L; L = 0; }

    int Q = 0;

    Q = E / 8;
    if (E % 8 != 0) Q++;
    Q += L / 8;
    if (L %8 != 0) Q++;
    //Q += L / 8;
    //if (L % 8 != 0) Q++;

    return Q;

}

void mercuryFromString(void *stack, int Precision, const char *Input, uint *val) {
    mercuryLoadZero(stack, Precision, val);

    if (*Input == '\0') {
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
           && mercuryHexDid(Input[Decimal]) > -1) Decimal++;
        bool hasDecimal = Input[Decimal] == '.';

    int sciNot = 0;

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

                        long V = 0;
                        int did;

                        do {
                                did = mercuryHexDid(Input[P]);
                               if (did != -1) {
                                        V = (V << 4) + did;
                                        P++;
                                   }
                            } while (did != -1);

                        sciNot = negativeExponent ? -V : V;
                   }
            }
    int S = Start;
    while (Input[S] == '0') S++;

    int wholeLen = 0;
    while (mercuryHexDid(Input[S + wholeLen]) > -1) wholeLen++;

    int Nibble ;

    int E;
    if (wholeLen > 0) {

                int highestHexPlace = wholeLen - 1 + sciNot;

                E = mercuryFloorDiv8(highestHexPlace);
                Nibble = mercuryMod8Floor(highestHexPlace);
    } else if (hasDecimal)
    {
        int fracPos = 0;
        while (Input[Decimal+1+fracPos]=='0') fracPos++;

        S = Decimal+1+fracPos;

        if (mercuryHexDid(Input[Decimal + 1 + fracPos]) < 0)
        {
            mercuryLoadZero(stack, Precision, val);
            return;
        }

        fracPos++;

        int highestHexPlace = -fracPos + sciNot;

                E = mercuryFloorDiv8(highestHexPlace);
                Nibble = mercuryMod8Floor(highestHexPlace);

    } else
    {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    val[1] = E;

    int P = 0;
    int I = S;

    while (Input[I] != '\0' && P < Precision) {
        uint V = 0;

        int d;
        for (int n = Nibble; n >= 0; n--) {
            if (I==Decimal) I++;

            d = mercuryHexDid(Input[I++]);
            if (d == -1) break;

            V |= (d << (n<<2));
        }

        val[2+Precision-1-P++] = V;
        if (d == -1) break;

        Nibble = 7;
    }

    if (negative) val[0] |= 1;

    P = I;
}

int mercuryToString(void *stack,int Precision, uint *val, char *buffer, int len)
{
    /* TODO */ //update to proper toString & bounds check

    const char *PlaceValue = "0123456789ABCDEF";

    int p = 0;
    int l = len - 1;
    buffer[l] = 0;

    if ((val[0] & 1) == 1)
    {
        if (p<l && buffer != 0) buffer[p] = '-';
        p++;
    }

    int e = (int) val[1];
    int el = e - Precision + 1;

    while (el < 0 && (mercuryGetAt(Precision, val, el)==0)) el++;

    //if (e<0) e = 0;
    //if (el>0) el = 0;

    bool scientificNotation = false;

    if (e<-1 || el>0)
    {
        scientificNotation = true;
    }


    int i;

    bool lead = true;

    long E = (e+1) * 8 - 1;

    int D = scientificNotation ? 1 : E+1;

    for (i = e; i >= el; i--) {
        uint d = mercuryGetAt(Precision, val, i);

        if (e==el && d == 0)
        {
            if (p < l && buffer != 0) buffer[p] = '0';
            p++;
            break;
        }

        //        bool _d = false;

        if (i<0 && !scientificNotation) lead = false;
        /*
                if (scientificNotation)
                {
                    if (i == e-1) _d = true;
                } else if (i == -1) _d = true;

                if (_d)
                {
                    if (p<l && buffer != 0) buffer[p] = '.';
                    p++;
                    lead = false;
                }
        */
        int low = 0;

        if (i==el && el<0)
        {
            while (low<=7 && ((int) ((d >> (low << 2)) & 0xF)==0)) low++;
        }

        for (int pl = 7; pl >= low; pl--) {
            int x = (int) ((d >> (pl << 2)) & 0xF);


            if (lead && x==0) E--;

            if (!scientificNotation) {
                D--;

                if (D == -1) {
                    if (p < l && buffer != 0) buffer[p] = '.';
                    p++;

                }
            }

            if (!lead || x != 0) {
                if (scientificNotation) {
                    D--;

                    if (D == -1) {
                        if (p < l && buffer != 0) buffer[p] = '.';
                        p++;

                    }
                }

                if (p < l && buffer != 0) buffer[p] = PlaceValue[x];
                p++;
                lead = false;
            }
        }
    }

    if (scientificNotation)
    {
        if (p < l && buffer != 0) buffer[p] = '@';
        p++;

        if (p < l && buffer != 0) buffer[p] = E<0 ? '-' : '+';
        p++;

        ulong d = (uint)(E < 0 ? -E : E);

        lead = true;
        for (int pl = 10; pl >= 0; pl--) {
            int x = (int) ((d >> (pl << 2)) & 0xF);

            if (!lead || x != 0 || pl == 0) {
                if (p < l && buffer != 0) buffer[p] = PlaceValue[x];
                p++;
                lead = false;            }
        }

    }

    if (p<len) buffer[p] = 0;
    p++;
    return p;
}