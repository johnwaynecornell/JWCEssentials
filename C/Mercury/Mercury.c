//
// Created by jwc on 9/11/21.
//
//extern "C"
//{
#include "Mercury.h"
//}
//#include <math.h>

void * mercuryThreadedStackGet(int thread, void *master)
{
    void *base = (char *) master + 8 + ((ulong *)master)[0] * thread;
    return base;
}

void * mercuryStackAlloc(void *stack, int size)
{
    void *r = (char *) stack + 8 + *((slong *)stack);
    for (int i=0; i<size; i++)
    {
        *((char *)r + i) = 0;
    }

    *((slong*)stack) += size;

    return r;
}

void mercuryStackFree(void *stack, int size)
{
    *((slong *)stack) -= size;
}

slong mercuryStackOffset(void *stack)
{
    return *(slong *)stack;
}

void mercuryLoadZero(void *stack, int Precision, uint *val)
{
    for (int i=0; i<Precision+2; i++) val[i] = 0;
}

void mercuryLoadRaw(void *stack,int Precision, uint *val, bool Negative, int exp, uint *digits, int digitsLen)
{
    while (digitsLen > 0 && digits[digitsLen-1]==0)
    {
        digitsLen--;
        exp--;
    }

    if (digitsLen == 0) exp = 0;

    val[0] = (Negative && digitsLen !=0) ? 1 : 0;
    val[1] = (uint) exp;

    int pl = digitsLen-Precision;
    if (pl<0) pl = 0;

    int p;
    for (p=0; p < Precision - digitsLen; p++)
    {
        val[2+p] = 0;
    }

    int i = pl;
    for (;p < Precision; p++)
    {
        val[2+p] = digits[i++];
    }
/*
    if (pl>0 && digits[pl-1] >= 0x80000000)
    {
        uint *t = (uint *) mercuryStackAlloc(stack, (Precision+2)<<2);
        mercuryLoadUint(stack, Precision, t, 1);
        t[1] = val[1]-Precision+1;
        mercuryAdd(stack, Precision, val, t, val);
        mercuryStackFree(stack, (Precision+2)<<2);
    }
    */
}

void mercuryLoadMercury(void *stack,int Precision, uint *a, uint *val)
{
    for (int i=0;i<Precision+2; i++)
    {
        val[i] = a[i];
    }
}

void mercuryLoadExtendMercury(void *stack,int OldPrecision, int Precision, uint *a, uint *val)
{
    mercuryLoadRaw(stack, Precision, val, (a[0]&1)==1, (int) a[1], a+2, OldPrecision);
}

void mercuryLoadInt(void *stack,int Precision, uint *val, int a)
{
    uint d = a < 0 ? -a : a;
    mercuryLoadRaw(stack, Precision, val, a<0, 0, &d, 1);
}

void mercuryLoadUint(void *stack,int Precision, uint *val, uint a)
{
    mercuryLoadRaw(stack, Precision, val, false, 0, &a, 1);
}

void mercuryLoadLong(void *stack,int Precision, uint *val, slong a)
{
    ulong d = a < 0 ? -a : a;
    //uint s[2];
    //s[1] = d >> 32;
    //s[0] = d;

    mercuryLoadRaw(stack, Precision, val, a<0, 1, (uint *) &d, 2);
}

void mercuryLoadUlong(void *stack,int Precision, uint *val, ulong a)
{
    mercuryLoadRaw(stack, Precision, val, false, 1, (uint *) &a, 2);
}

void mercuryLoadDouble(void *stack,int Precision, uint *val, double a)
{
    if (a == 0.0)
    {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    ulong l = *((ulong *) &a);

    bool Negative = (l & 0x8000000000000000ULL) != 0;

    int _e = (int) ((l & 0x7FF0000000000000LL) >> 52);

    int rawexp = _e - 1023;

    //printf("exponent = %d\n", rawexp);

    ulong frac[2];

    ulong ll = ((l & 0xFFFFFFFFFFFFFULL) + 0x10000000000000ULL);

    //Output(&ll,8);

    ll <<= (64 - 53);

    frac[1] = ll;
    frac[0] = 0UL;

    int newexp;
    int mod;

    if (rawexp >= 0) {
        newexp = (rawexp >> 5);

        mod = rawexp % 32;
        mod = 31 - mod;
    } else {
        newexp = ((rawexp + 1) / 32) - 1;
        rawexp *= -1;

        mod = (rawexp + 31) % 32;
    }

    if (mod != 0)
        frac[0] = (frac[1] << (64 - mod)); //+(frac[0]>>mod);
    frac[1] = frac[1] >> mod;

    mercuryLoadRaw(stack, Precision, val, Negative, newexp, (uint *) &frac, 4);
}

int mercuryNibbles(void *stack,int Precision, uint *val)
{
    int highNibble = (Precision)*8;

    bool C;

    do {
        highNibble--;
        int pos = highNibble / 8;
        C = val[2+pos] & (0xF<<((highNibble%8)*4));
    } while (highNibble !=0 && C==0);

    int lowNibble = -1;
    do {
        lowNibble++;
        int pos = lowNibble / 8;
        C = val[2+pos] & (0xF<<((lowNibble%8)*4));
    } while (lowNibble <highNibble && C==0);

    return highNibble-lowNibble+1;
}

__host__ __device__ ulong doshift(ulong val, int shift) {
    if (shift > 0)
        return val << shift;
    return val >> (-shift);
}

double mercuryToDouble(void *stack, int Precision, uint * a) {
    if (a == 0)
        return 0.0;

    if (mercuryIsZero(Precision, a))
        return 0.0;

    ulong l = 0;

    if (a[0] & 1)
        l += 0x8000000000000000ULL;

    ulong ll = 0;

    int x = (int) a[1];

    uint p;

    do {
        p = mercuryGetAt(Precision, a, x);
        if (p == 0)
            x--;
    } while (p == 0);

    int shift = 0;

    while ((p & 0x80000000U) == 0) {
        p <<= 1;
        shift++;
    }

    ll += (ulong) p << (21);

    ll += doshift((ulong) mercuryGetAt(Precision, a, x - 1), (21 - 32 + shift));

    ulong ll2 = doshift((ulong) mercuryGetAt(Precision, a, x - 2), 21 - 64 + shift);// << (21-shift);

    //Output(&ll2, 8);

    ll += ll2;

    //Output(&ll, 8);

    l += ll & 0xFFFFFFFFFFFFFULL;

    if (x >= 0) {
        x = x * 32 + (31 - shift);
    } else {
        x = x * 32 + 31 + (-shift);
    }

    //printf("new exponent = %d\n", x);

    x += 1023;

    if (x < 0) return 0;
    if (x > 0x7FF) {
        l = (l & 0x8000000000000000ULL) | 0x7FF0000000000000ULL;
        return *((double *) &l);
    }

    l += (ulong) x << 52;

    return *((double *) &l);
}






bool mercuryIsZero(int Precision, uint *val) {
    for (int i=0; i<Precision; i++)
    {
        if (val[2+i] != 0) return false;
    }
    return true;
}

uint mercuryGetAt(int Precision, uint *a, int Place)
{
    int x = Place - ((int) a[1] - Precision+1);
    if (x<0 || x>=Precision) return 0;
    return a[2+x];
}

int mercuryAbsCmp(int Precision, uint *a, uint *b)
{

    int ah = a[1];
    int bh = b[1];

    int al = ah - Precision + 1;
    int bl = bh - Precision + 1;

    if ((al==ah)&&(bl==bh))
    {
        bool az = (al == ah) && mercuryGetAt(Precision, a, al) == 0;
        bool bz = (bl == bh) && mercuryGetAt(Precision, b, bl) == 0;

        if (az) {
            if (bz)
                return 0;
            else
                return -1;
        }

        if (bz) {
            return 1;
        }

        if (ah > bh)
            return 1;

        if (bh > ah)
            return -1;

    }

    int l = al > bl ? (int) bl : (int) al;
    int h = ah > bh ? (int) ah : (int) bh;

    for (int i = (int) h; i >= l; i--) {
        uint ad = mercuryGetAt(Precision, a, i);
        uint bd = mercuryGetAt(Precision, b, i);

        if (ad > bd)
            return 1;
        if (ad < bd)
            return -1;
    }

    return 0;
}

int mercuryCmp(int Precision, uint *a, uint *b) {
    if ((a[0] & 1) == 0 && (b[0] & 1) == 1)
        return 1;
    if ((a[0] & 1) == 1 && (b[0] & 1) == 0)
        return -1;

    int neg = ((a[0] & 1) == 1) ? -1 : 1;

    return mercuryAbsCmp(Precision, a, b) * neg;
}

void mercuryShift(void *stack, int Precision, uint *a, int shift, uint *val) {
    int places;

    if (shift == 0) {
        mercuryLoadRaw(stack, Precision, val, (a[0] & 1) == 1,
                       a[1],
                       a+2, Precision);

        return;
    }

    uint *reg = (uint *) mercuryStackAlloc(stack, (Precision + 1)*4);

    //if (a[0]&1) shift*=-1;

    if (shift >= 0) {
        places = shift >> 5;
        shift -= places << 5;

        if (shift == 0) {
            mercuryLoadRaw(stack, Precision, val, (a[0] & 1) == 1,
                           a[1] + places,
                           a+2, Precision);
            mercuryStackFree(stack, (Precision+1)*4);
            return;
        }

        reg[Precision] = a[2 + Precision - 1] >> (32 - shift);

        for (int i = Precision - 1; i > 0; i--) {
            reg[i] = ((a[2+i - 1] >> (32 - shift)))
                     + (a[2+i] << shift);
        }

        reg[0] = a[2+0] << shift;
        mercuryLoadRaw(stack, Precision, val, (a[0] & 1) == 1,
                       a[1] + places + 1,
                       reg, Precision+1);
        mercuryStackFree(stack, (Precision+1)*4);
        return;
    }

    shift *= -1;

    places = shift >> 5;
    shift -= places << 5;

    if (shift == 0) {
        mercuryLoadRaw(stack, Precision, val, (a[0] & 1) == 1,
                       a[1] + places,
                       a+2, Precision);
        mercuryStackFree(stack, (Precision+1)*4);
        return;
    } else {

        reg[Precision] = a[2 + Precision - 1] >> shift;

        for (int i = Precision - 1; i >= 0; i--) {
            reg[i] = (a[2 + i - 1] >> shift)
                     + (a[2 + i] << (32 - shift));
        }

        reg[0] = a[2 + 0] << (32 - shift);
        mercuryLoadRaw(stack, Precision, val, (a[0] & 1) == 1,
                       a[1] - places,
                       reg, Precision + 1);
        mercuryStackFree(stack, (Precision + 1)*4);
    }
}

void mercuryAbsAdd(void *stack, int Precision, uint *a, uint *b, uint *val) {

    if (((int)a[1])<((int)b[1]))
    {
        uint *tmp = a;
        a = b;
        b = tmp;
    }

     int ah = a[1];
     int bh = b[1];

     int al = ah - Precision + 1;
     int bl = bh - Precision + 1;

/*
     int l = al < bl ? al : bl;
     int h = ah > bh ? ah : bh;
*/

//     int range = h - l + 1;

    int l;// = al < bl ? al : bl;
    int h = ah;//ah > bh ? ah : bh;

    l = h -Precision + 1;

    //int range = h - l + 1;
/*
    if (range > Precision) {
        l += range - Precision;
        range = (h - l) + 1;
    }
*/
    int len = h - l + 1+1;//Precision + 1;

/*
     if (range > Precision) {
         l += range - Precision;
         range = (h - l) + 1;
     }
*/
     int e = h + 1;
     uint *scratch = (uint *) mercuryStackAlloc(stack, (len)*4);

     int i;

     for (i = 0; i < len; i++)
         scratch[i] = 0;
     ulong reg = 0;

     for (i = l; i < bl && i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));

         scratch[i - l] = a[2+i-al];
    }

    for (;i <= bh && i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));

        reg += ((slong) a[2+i-al] + (slong) b[2+i-bl]);

        scratch[i - l] = (uint) reg;
        reg>>=32;
    }

    while (reg != 0 && i<= h)
    {
        reg += a[2+i-al];
        scratch[i - l] = (uint) reg;
        reg>>=32;
        i++;
    }

    for (;i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));
        scratch[i - l] = a[2+i-al];
        reg=0;
    }

    scratch[i - l] = (uint) reg;

/*
        ulong reg = ((ulong) mercuryGetAt(Precision, a, l - 1)
                     + (ulong) mercuryGetAt(Precision, b, l - 1)) >> 32;

        for (i = l; i <= h; i++) {
            reg += ((ulong) mercuryGetAt(Precision, a, i)
                    + (ulong) mercuryGetAt(Precision, b, i));

            scratch[i - l] = (uint) reg;

            reg >>= 32;
        }

        scratch[i - l] = (uint) reg;
*/
     mercuryLoadRaw(stack, Precision, val, false, e, scratch, len);
     mercuryStackFree(stack, len*4);
 }

void mercuryAbsSub(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    int ah = a[1];
    int bh = b[1];

    int al = ah - Precision + 1;
    int bl = bh - Precision + 1;

    int l;// = al < bl ? al : bl;
    int h = ah;//ah > bh ? ah : bh;

    l = h -Precision + 1;

    //int range = h - l + 1;
/*
    if (range > Precision) {
        l += range - Precision;
        range = (h - l) + 1;
    }
*/
    int len = h - l + 1;//Precision + 1;

    int e = h;
    uint *scratch = (uint *) mercuryStackAlloc(stack, len*4);

    int i;

	for (i = 0; i < len; i++)
		scratch[i] = 0;

	slong reg;
    //reg = ((long) mercuryGetAt(Precision, a, l - 1)
    //        - (long) mercuryGetAt(Precision, b, l - 1));

	//if (reg > 0)
    reg = 0;

    if (bl<l)
    {
        int _h = bh > l ? l : bh;
        for (i=bl; i <= _h; i++)
        {
            if (mercuryGetAt(Precision, b, i) != 0)
            {
                reg = -1;
                break;
            }
        }
    }



    for (i = l; i < bl && i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));
        reg += ((slong) a[2+i-al]);

        bool carry = reg < 0;

        if (carry)
        {
            reg += 0x100000000LL;
            scratch[i - l] = (uint) reg;
            reg = -1;
        } else
        {
            scratch[i - l] = (uint) reg;
            reg = 0;
        }
    }

    for (;i <= bh && i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));
        reg += ((slong) a[2+i-al] - (slong) b[2+i-bl]);

        bool carry = reg < 0;

        if (carry)
        {
            reg += 0x100000000LL;
            scratch[i - l] = (uint) reg;
            reg = -1;
        } else
        {
            scratch[i - l] = (uint) reg;
            reg = 0;
        }
    }

    for (;i <= h; i++) {
        //reg += ((long) mercuryGetAt(Precision, a, i)
        //        - (long) mercuryGetAt(Precision, b, i));
        reg += ((slong) a[2+i-al]);

        bool carry = reg < 0;

        if (carry)
        {
            reg += 0x100000000LL;
            scratch[i - l] = (uint) reg;
            reg = -1;
        } else
        {
            scratch[i - l] = (uint) reg;
            reg = 0;
        }
    }

    mercuryLoadRaw(stack, Precision, val, false, e, scratch, len);
    mercuryStackFree(stack, len*4);
}

void mercuryAbsMul(void *stack, int Precision, uint *a, uint *b, uint *val) {
    if (mercuryIsZero(Precision, a) || mercuryIsZero(Precision, b)) {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    int ah = a[1];
    int bh = b[1];

    int al = ah - Precision + 1;
    int bl = bh - Precision + 1;

    int i;

    int as, bs;

    as=bs=0;

    for (i=0; al != ah && a[2+i]==0; i++) as++;
    for (i=0; bl != bh && b[2+i]==0; i++) bs++;


    int e = ah + bh + 1;
    int len;

    len = Precision+2;
    int len2 = (ah-al)+1+(ah-al)+1;

    //len = len2+2;

    int adj = len2-len;

    //do {
//        len = (ah - al) + 1 + (bh - bl) + 1;
/*
        if (len > Precision+2) {
            if (al <= bl) al++;
            else bl++;
        }
    } while (len > Precision+2);
*/
    uint *scratch = (uint *) mercuryStackAlloc(stack, len*4);

    for (i = 0; i < len; i++) scratch[i] = 0;

    int br = bh - bl;
    int ar = ah - al;

    int bq = Precision-1-br;//bl - ((int) b[1] - Precision + 1);
    int aq = Precision-1-ar;//al - ((int) a[1]- Precision + 1);

    //for (int bi = bl; bi<=bh; bi++)
    for (int bi = bs; bi <= br; bi++) {
        uint bx = b[2 + bi + bq];//GetAt(b,bi);

        int ai;
        ulong reg = 0;

        //      if (bx != 0) {
        for (ai = as; ai <= ar; ai++) {
            int place = ai + bi-adj;

            if (place >= 0) {
                uint ax = a[2 + ai + aq];//GetAt(a, ai);

                //int place = (bi-bl)+(ai-al);

//                if (ax != 0) {
                reg += (ulong) bx * (ulong) ax + scratch[place];
//                } else reg += scratch[place];

                scratch[place] = (uint) reg;
                reg >>= 32;
            } else reg = 0;
        }

//        } else ai = ar;

        //int place = (bi-bl)+(ai-al);
        int place = ai + bi - adj;

        if (place >=0) {
            reg += scratch[place];
            scratch[place] = (uint) reg;
        }
    }

    mercuryLoadRaw(stack, Precision, val, false, e, scratch, len);
    mercuryStackFree(stack, len*4);
}

void mercuryAbsDiv(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    uint *dividend = (uint *) mercuryStackAlloc(stack, (Precision+2)*4);
    mercuryLoadMercury(stack, Precision, a, dividend);
    dividend[0] = dividend[0]&0xFFFFFFFE;

    uint *divisor = (uint *) mercuryStackAlloc(stack, (Precision+2)*4);
    mercuryLoadMercury(stack, Precision, b, divisor);
    divisor[0] = divisor[0]&0xFFFFFFFE;

    /*
    printf("----\n");

    mercuryPrint(stack, Precision, a);
    printf("\n");
    mercuryPrint(stack, Precision, b);
    printf("\n");
    */
    int m = ((int) divisor[1])-Precision+1;
    int m1 = m;

    dividend[1] -= m;
    divisor[1] -= m;

    int exp = 0;//dividend->Exponent;

    m = (int) dividend[1] - (int) divisor[1];
    dividend[1] -= m;

    /*
    mercuryPrint(stack, Precision, dividend);
    printf("\n");
    mercuryPrint(stack, Precision, divisor);
    printf("\n");
*/
    //printf("dividend exp = %d, divisor exp = %d\n", dividend->Exponent, divisor->Exponent);
    //printf("divisor explow = %d\n", divisor->ExponentLow);

    //fflush(stdout);

    int compare = mercuryCmp(Precision, dividend,divisor);

    //exp -= (m-m1);

    //printf("initialshift = %d tempshift = %d rawexp=%d idigit=%s\n", m1, m, exp, compare < 0 ? "false" : "true");

    exp = m;

    if (compare<0)
    {
        exp--;
        dividend[1]++;
    }

    uint *TableMem = (uint *) mercuryStackAlloc(stack, (Precision+2)*4*8*16);

    uint *Table[8][16];

    for (int q=0; q<8; q++)
    {
        for (int i=0; i<16; i++)
        {
            Table[q][i] = TableMem + ((q*16 + i)*(Precision+2));
        }
    }

    mercuryLoadZero(stack, Precision, Table[0][0]);
    mercuryLoadMercury(stack, Precision, divisor, Table[0][1]);

    for (int i=2; i<16; i++)
    {
        mercuryAdd(stack, Precision, Table[0][i-1], divisor, Table[0][i]);
    }

    for (int q=1; q<8; q++)
    {
        mercuryLoadZero(stack, Precision, Table[q][0]);

        for (int i=1; i<16; i++)
        {
            mercuryShift(stack, Precision, Table[0][i], q*4, Table[q][i]);
        }
    }

    /*
    char txt[1024];

    for (int i=0; i<16; i++)
    {
        printf("%d: ", i);

        for (int q=0; q<8; q++)
        {
            mercuryToString(stack, Precision,Table[q][i], txt, 1024);
            printf("%s ", txt);
        }

        printf("\n");
    }
    */
    uint *reg = (uint *) mercuryStackAlloc(stack, Precision*4);

    for (int i=0; i<Precision; i++) reg[i] = 0;

    for (int i = Precision-1; i>=0; i--)
    {
        for (int p=7; p>=0; p--)
        {
            for (int d=0xF; d>=0; d--)
            {
                uint *x = Table[p][d];

                if (mercuryAbsCmp(Precision, dividend, x)>=0)
                {
                    mercurySub(stack, Precision, dividend, x, dividend);

                    reg[i] += ((uint) d) << (p*4);

                    if (mercuryIsZero(Precision, dividend))
                    {
                        goto ret;
                    }

                    d = -1;
                }
            }
        }

        dividend[1]++;
    }

    ret:

    mercuryLoadRaw(stack, Precision,val, false,exp,reg, Precision);
    mercuryStackFree(stack, (Precision+2)*2*4 + (Precision+2)*4*8*16+Precision*4);
}

void mercuryAdd(void *stack, int Precision, uint *a, uint *b, uint *val) {
    if ((a[0] & 1) == (b[0] & 1)) {
        int sgn = (a[0] & 1);
        mercuryAbsAdd(stack, Precision, a, b, val);
        val[0] |= sgn;
        return;
    }

    int s = mercuryAbsCmp(Precision, a, b);
    if (s == 0) {
        mercuryLoadZero(stack, Precision, val);
        //mercuryShift(stack, Precision, a, 1, val);
        return;
    }

    if (a[0] & 1) {
        if (s == 1) {
            mercuryAbsSub(stack, Precision, a, b, val);
            val[0] |= 1;
            return;
        } else {
            mercuryAbsSub(stack, Precision, b, a, val);
            //val[0] |= 0
            return;
        }
    } else {
        if (s == 1) {
            mercuryAbsSub(stack, Precision, a, b, val);
            return;
        } else {
            mercuryAbsSub(stack, Precision, b, a, val);
            val[0] |= 1;
            return;
        }
    }
}

void mercurySub(void *stack, int Precision, uint *a, uint *b, uint *val) {
    if ((a[0] & 1) != (b[0] & 1)) {
        bool sign = a[0] & 1;
        mercuryAbsAdd(stack, Precision, a, b, val);
        if (sign) val[0] |= 1;
        return;
    }

    int s = mercuryAbsCmp(Precision, a, b);

    if (s == 0)
    {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    if (a[0] & 1) {
        if (s == 1) {
            mercuryAbsSub(stack, Precision, a, b, val);
            val[0] |= 1;
            return;
        } else {
            mercuryAbsSub(stack, Precision, b, a, val);
            return;
        }
    } else {
        if (s == 1) {
            mercuryAbsSub(stack, Precision, a, b, val);
            return;
        } else {
            mercuryAbsSub(stack, Precision, b, a, val);
            if (s != 0) val[0] |= 1;
            return;
        }
    }
}

void mercuryMul(void *stack, int Precision, uint *a, uint *b, uint *val) {
    bool sign = (a[0] & 1) != (b[0] & 1);
    mercuryAbsMul(stack, Precision, b, a, val);
    if (sign) val[0] |= 1;
}

void mercuryDiv(void *stack, int Precision, uint *a, uint *b, uint *val) {
    bool sign = (a[0] & 1) != (b[0] & 1);
    mercuryAbsDiv(stack, Precision, a, b, val);
    if (sign) val[0] |= 1;
}

void mercury_2Pow(void *stack, int Precision, int Place, uint *val) {

    if (Place >= 0) {

        int e = Place >> 5;
        int p = Place - (e << 5);

        uint i;

        if (p == 0)
            i = 1;
        else
            i = 1U << p;

        mercuryLoadRaw(stack, Precision, val, false, e, &i, 1);
    } else {
        int e = (Place - 31) / 32;

        int p = (Place - 32) - (e * 32);

        uint i;

        if (p == 0)
            i = 1;
        else
            i = 1U << p;

        mercuryLoadRaw(stack, Precision, val, false, e, &i, 1);
    }
}

int mercuryGetBit(void *stack, int Precision, int Place, uint *a)
{
    int Did = Place / 32;
    int bit = Place;

    if (Place >= 0)
    {
        bit %= 32;
    } else
    {
        //Did--;
        //bit = (32 + bit) % 32;

        if ((-bit)%32 != 0) Did--;
        bit = (-bit)%32;
        bit = (32 - bit) % 32;
    }

    int x = Did - ((int) a[1] - Precision+1);
    if (x<0 || x>=Precision) return 0;
    uint v = a[2+x];

    if ((v & (1u<<bit)) != 0) return 1;
    return 0;
}


void mercurySqr(void *stack, int Precision, uint *a, uint *val) {
    int ah = (int)a[1];
    int al = ah - Precision + 1;

    int e = ah + ah+1;
    int len;

    int as = 0;

    int i=0;
    //while (al != ah && a[2+i++]==0) as++;

    len = Precision+1;
    int len2 = (ah-al)+1+(ah-al)+1;

    //len = len2;

    /*
    do
    {
        len = (ah-al)+1+(ah-al)+1;
        if (len > Precision)
        {
            al++;
        }
    } while (len>Precision);
    */
    uint *scratch = (uint *) mercuryStackAlloc(stack, len*4);

    for (i=0; i<len; i++) scratch[i] = 0;

    int ar = ah-al;

    int aq = Precision-1-ar;//al-((int)a[1] - Precision + 1);

    ulong ax,bx, reg;
    int ai, bi;
    int place;

    int adj = len2-len;

    //for (int bi = bl; bi<=bh; bi++)
    bi = as;

    if (bi <= ar)
    {
        bx = a[2+bi+aq];//GetAt(b,bi);

        reg = 0;

        for (ai = as; ai<=ar; ai++)
        {
            place = ai+bi;

            if (place - adj >=0) {
                ax = a[2 + ai + aq];//GetAt(a, ai);

                //int place = (bi-bl)+(ai-al);
                reg += bx * ax;

                //if (place<0||place>=len) throw 1;

                scratch[place-adj] = (uint) reg;

                reg >>= 32;
            } else reg = 0;
        }

        //int place = (bi-bl)+(ai-al);
        place = ai+bi;

        if (place - adj < len) scratch[place-adj] = (uint) reg;

        bi++;;
    }

    for (; bi<=ar; bi++)
    {
        bx = a[2+bi+aq];//GetAt(b,bi);

        reg = 0;

        for (ai = as; ai<=ar; ai++)
        {
            place = ai+bi;


            if (place-adj>=0) {
                ax = a[2 + ai + aq];//GetAt(a, ai);

                //int place = (bi-bl)+(ai-al);

                reg += bx * ax + scratch[place-adj];

                //if (place<0||place>=len) throw 1;

                scratch[place-adj] = (uint) reg;

                reg >>= 32;
            } else reg = 0;
        }

        //int place = (bi-bl)+(ai-al);
        place = ai+bi;

        if (place-adj >=0) {
            reg += scratch[place-adj];
            scratch[place-adj] = (uint) reg;
        }
    }

    mercuryLoadRaw(stack, Precision, val, false, e, scratch, len);
    mercuryStackFree(stack, len*4);
}

void mercurySqrt(void *stack, int Precision, uint *a, uint *val) {
    int h = (int)a[1];

    int highbit = ((int) ((h + 1) * 32)) / 2;
    int lowbit = highbit - Precision * 32;

    if (mercuryIsZero(Precision, a)) {
        mercuryLoadZero(stack, Precision, val);
        return;
    }

    uint *A = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);

    mercuryLoadRaw(stack, Precision+1, A, false, a[1], a+2, Precision);


    uint *m = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);
    uint *mark = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);
    uint *q = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);
    uint *x = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);

    mercuryLoadZero(stack, Precision+1, m);

    for (int i = highbit; i >= lowbit; i--) {
        mercury_2Pow(stack, Precision+1, i, mark);
        mercuryAdd(stack, Precision+1, m, mark, q);
        mercurySqr(stack, Precision+1, q, x);

        int s = mercuryCmp(Precision+1, x, A);

        if (s <= 0) {
            mercuryLoadMercury(stack, Precision+1, q, m);
            if (s == 0)
                break;
        }
    }

    mercuryLoadRaw(stack, Precision, val, false, m[1], m+2, Precision+1);

    mercuryStackFree(stack, (Precision+1+2)*4);
    mercuryStackFree(stack, (Precision+1+2)*4);
    mercuryStackFree(stack, (Precision+1+2)*4);
    mercuryStackFree(stack, (Precision+1+2)*4);
    mercuryStackFree(stack, (Precision+1+2)*4);
}

void mercuryPow(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    int bh = b[1];
    int bl = bh - Precision + 1;

    int i=0;
    while (i<Precision && b[2+i]==0)
    {
        i++;
        bl++;
    }

    int highBit = (bh * 32)+31;
    int lowBit = ((bl) * 32)-1;

    while ((lowBit != highBit) && (mercuryGetBit(stack, Precision, highBit, b)==0)) highBit--;
    while ((lowBit != highBit) && (mercuryGetBit(stack, Precision, lowBit, b)==0)) lowBit++;

    uint *r = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);
    uint *p = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);

    mercuryLoadUint(stack, Precision+1, r, 1);
    mercuryLoadExtendMercury(stack, Precision, Precision+1, a, p);

    for (int bit=0; bit<=highBit; bit++)
    {
        if (mercuryGetBit(stack, Precision, bit, b)==1)
        {
            mercuryMul(stack, Precision+1, r, p, r);
            //else mercuryDiv(stack, Precision+1, r, p, r);
        }

        if (bit != highBit) mercurySqr(stack, Precision+1,p, p);
    }

    mercuryLoadExtendMercury(stack, Precision, Precision+1, a, p);
    for (int bit=-1; bit>=lowBit; bit--)
    {
        mercurySqrt(stack, Precision+1,p, p);

        if (mercuryGetBit(stack, Precision, bit, b)==1)
        {
            mercuryMul(stack, Precision+1, r, p, r);
            //else mercuryDiv(stack, Precision+1, r, p, r);
        }
    }

    if ((b[0]&1)==1)
    {
        uint *one = (uint *) mercuryStackAlloc(stack, (Precision+1+2)*4);
        mercuryLoadUint(stack, (Precision+1), one, 1);
        mercuryDiv(stack, Precision+1, one, r, r);
        mercuryStackFree(stack, (Precision+1+2)*4);
    }

    mercuryLoadExtendMercury(stack, Precision+1, Precision, r, val);
    mercuryStackFree(stack, (Precision+1+2)*4);
    mercuryStackFree(stack, (Precision+1+2)*4);
}

void mercuryRoot(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    uint *p = (uint *) mercuryStackAlloc(stack, (Precision+2)*4);
    uint *one = (uint *) mercuryStackAlloc(stack, (Precision+2)*4);
    mercuryLoadUint(stack, Precision, one, 1);
    mercuryDiv(stack, Precision, one, b, p);
    mercuryStackFree(stack, (Precision+2)*4);
    mercuryPow(stack, Precision, a, p, val);
    mercuryStackFree(stack, (Precision+2)*4);
}

void mercuryLogSlow(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    int InnerPrecision = Precision;

    uint *one = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *A = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *B = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *last = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *reg = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *x = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *p = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);

    mercuryLoadUint(stack,InnerPrecision,one,1);
    mercuryLoadExtendMercury(stack, Precision, InnerPrecision, a, A);
    mercuryLoadExtendMercury(stack, Precision, InnerPrecision, b, B);

    int c = mercuryCmp(Precision, A, one);
    int c2 = mercuryCmp(Precision, B, one);

    //printf("c = %d, c2 = %d\n", c, c2);

    if (c == 0 || c2 == 0)
    {
        /* TODO */ //c2 == 0 return INF?
        mercuryLoadZero(stack, Precision, val);
        goto stackCleanup;
    }

    int s;

    //if (c * c2 > 0)
    {
        int g = mercuryCmp(InnerPrecision, A,B);

        int bit;

        mercuryLoadZero(stack, InnerPrecision, reg);

        if (g ==0)
        {
            mercuryLoadUint(stack, Precision,val, 1);
            goto stackCleanup;
        }

        if (g == c)
        {
            bit = 0;

            int s;
            do
            {
                mercuryLoadMercury(stack, InnerPrecision, reg, last);
                mercury_2Pow(stack, InnerPrecision, bit, reg);

                if (c*c2<0) reg[0] |= 1;

                mercuryPow(stack, InnerPrecision, B, reg, x);
                s = mercuryCmp(InnerPrecision, x, A)*c;
                if (s<0) bit++;
            } while (s<0);

            if (s ==0)
            {
                //if (last != NULL) delete last;
                //return reg;
                mercuryLoadExtendMercury(stack, InnerPrecision, Precision, reg, val);
                goto stackCleanup;
            }

            //reg = last;
        } else
        {
            bit = -1;

            do
            {
                mercuryLoadMercury(stack, InnerPrecision, reg, last);
                mercury_2Pow(stack, InnerPrecision, bit, reg);
                if (c*c2<0) reg[0] |= 1;

                mercuryPow(stack, InnerPrecision,B, reg, x);
                s = mercuryCmp(InnerPrecision, x, A)*c;
                if (s>0) bit--;
            } while (s>0);

            if (s ==0)
            {
                mercuryLoadExtendMercury(stack, InnerPrecision, Precision, reg, val);
                goto stackCleanup;
            }
        }

        mercuryLoadZero(stack, InnerPrecision, reg);

        int bits;
        //if (bit <0) bits = 32 - ((-bit)%32);
        //else bits = bit %32;

        bits = (Precision)*32+1;

        //if (c*c2<0) bit--;

        for (int i=0; i<=bits; i++)
        {
            //if (bit == -32) __asm("int3");

            mercuryLoadMercury(stack, InnerPrecision, reg, last);

            mercury_2Pow(stack, InnerPrecision, bit, p);
            if (c*c2<0) p[0] |= 1;
/*
            char txt[1024];
            mercuryToString(stack, InnerPrecision, p, txt, 1024);
            printf("bit %d = %s\n", bit, txt);
*/

//if (bit ==-37) __asm("int3");

            mercuryAdd(stack, InnerPrecision, reg, p, reg);
            mercuryPow(stack, InnerPrecision, B, reg, x);
/*
            char txt[3][1024];
            mercuryToString(stack, InnerPrecision, B, txt[0], 1024);
            mercuryToString(stack, InnerPrecision, reg, txt[1], 1024);
            mercuryToString(stack, InnerPrecision, x, txt[2], 1024);
            printf("reg %d = %s pow %s = %s\n", bit, txt[0], txt[1], txt[2]);
*/
            s = mercuryCmp(InnerPrecision, x, A)*c;

            if (s>0)
            {
                mercuryLoadMercury(stack, InnerPrecision, last, reg);
            } else if (s ==0)
            {
                mercuryLoadExtendMercury(stack, InnerPrecision,Precision, reg, val);
                goto stackCleanup;
            }
            bit--;

        }

        mercuryLoadExtendMercury(stack, InnerPrecision,Precision, reg, val);
        goto stackCleanup;
    }

    mercuryLoadZero(stack, Precision, val);
    stackCleanup:
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);

    return;
}

void mercuryLog(void *stack, int Precision, uint *a, uint *b, uint *val)
{
    int InnerPrecision = Precision;
    int PowerPrecision = InnerPrecision+1;

    uint *one = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *A = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *B = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *last = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *reg = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *x = (uint *) mercuryStackAlloc(stack, (InnerPrecision+2)*4);
    uint *_bit = (uint *) mercuryStackAlloc(stack, (InnerPrecision + 2) * 4);

    uint *r = (uint *) mercuryStackAlloc(stack, (PowerPrecision+2)*4);
    uint *lastR = (uint *) mercuryStackAlloc(stack, (PowerPrecision+2)*4);
    uint *p = (uint *) mercuryStackAlloc(stack, (PowerPrecision+2)*4);


    mercuryLoadUint(stack,InnerPrecision,one,1);
    mercuryLoadExtendMercury(stack, Precision, InnerPrecision, a, A);
    mercuryLoadExtendMercury(stack, Precision, InnerPrecision, b, B);

    mercuryLoadUint(stack, PowerPrecision, r, 1);
    mercuryLoadExtendMercury(stack, Precision, PowerPrecision, b, p);

    int c = mercuryCmp(InnerPrecision, A, one);
    int c2 = mercuryCmp(InnerPrecision, B, one);

    //printf("c = %d, c2 = %d\n", c, c2);

    if (c == 0 || c2 == 0)
    {
        /* TODO */ //c2 == 0 return INF?
        mercuryLoadZero(stack, Precision, val);
        goto stackCleanup;
    }

    int s;

    //if (c * c2 > 0)
    {
        int g = mercuryCmp(InnerPrecision, A,B);

        int bit;

        mercuryLoadZero(stack, InnerPrecision, reg);

        if (g ==0)
        {
            mercuryLoadUint(stack, Precision,val, 1);
            goto stackCleanup;
        }

        if (g == c)
        {
            bit = 0;

            int s;
            do
            {
                //mercuryLoadMercury(stack, InnerPrecision, reg, last);
                //mercury_2Pow(stack, InnerPrecision, bit, reg);
                //if (c*c2<0) reg[0] |= 1;
                //mercuryPow(stack, InnerPrecision, B, reg, x);

                mercuryLoadExtendMercury(stack, PowerPrecision, InnerPrecision, p, x);
                if (c*c2<0) mercuryDiv(stack, InnerPrecision, one, x, x);

                s = mercuryCmp(InnerPrecision, x, A)*c;
                if (s<0)
                {
                    bit++;
                    mercurySqr(stack, PowerPrecision,p, p);
                }
            } while (s<0);

            if (s ==0)
            {
                //if (last != NULL) delete last;
                //return reg;
                mercuryLoadExtendMercury(stack, InnerPrecision, Precision, reg, val);
                goto stackCleanup;
            }

            //reg = last;
        } else
        {
            bit = 0;

            do
            {
                bit--;
                mercurySqrt(stack, PowerPrecision,p, p);
                mercuryLoadExtendMercury(stack, PowerPrecision, InnerPrecision, p, x);
                if (c*c2<0) mercuryDiv(stack, InnerPrecision, one, x, x);

                s = mercuryCmp(InnerPrecision, x, A)*c;
                /*if (s<0)
                {
                    bit--;
                }*/

            } while (s>0);

            if (s ==0)
            {
                mercuryLoadExtendMercury(stack, InnerPrecision, Precision, reg, val);
                goto stackCleanup;
            }
        }

        mercuryLoadZero(stack, InnerPrecision, reg);

        int bits;
        //if (bit <0) bits = 32 - ((-bit)%32);
        //else bits = bit %32;

        bits = (InnerPrecision)*32+1;

        //if (c*c2<0) bit--;

        for (int i=0; i<=bits; i++)
        {
            //if (bit == -32) __asm("int3");

            mercuryLoadMercury(stack, InnerPrecision, reg, last);
            mercuryLoadMercury(stack, PowerPrecision, r, lastR);

            mercury_2Pow(stack, InnerPrecision, bit, _bit);
            if (c*c2<0) _bit[0] |= 1;

            mercuryAdd(stack, InnerPrecision, reg, _bit, reg);

            mercuryMul(stack, PowerPrecision, r, p, r);

            mercuryLoadExtendMercury(stack, PowerPrecision, InnerPrecision, r, x);
            if (c*c2<0) mercuryDiv(stack, InnerPrecision, one, x, x);

            //mercuryPow(stack, InnerPrecision, B, reg, x);

            s = mercuryCmp(InnerPrecision, x, A)*c;

            if (s>0)
            {
                mercuryLoadMercury(stack, InnerPrecision, last, reg);
                mercuryLoadMercury(stack, PowerPrecision, lastR, r);
            } else if (s ==0)
            {
                mercuryLoadExtendMercury(stack, InnerPrecision,Precision, reg, val);
                goto stackCleanup;
            }
            bit--;

            if (i != bits-1)
            {
                mercurySqrt(stack, PowerPrecision,p,p);
            }
        }

        mercuryLoadExtendMercury(stack, InnerPrecision,Precision, reg, val);
        goto stackCleanup;
    }

    mercuryLoadZero(stack, Precision, val);
stackCleanup:
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);
    mercuryStackFree(stack, (InnerPrecision+2)*4);

    mercuryStackFree(stack, (PowerPrecision+2)*4);
    mercuryStackFree(stack, (PowerPrecision+2)*4);
    mercuryStackFree(stack, (PowerPrecision+2)*4);

    return;
}

/*
int fracPlaces(UltraNumber *a)
{
    int h = a->Exponent;
    int l = a->ExponentLow;

    if (h>=0) h = -1;

    if (h<l) return 0;

    return (h - l)+1;
}
*/


/*
uint doshift(uint val, int shift) {
	if (shift > 0)
		return val << shift;
	return val >> (-shift);
}

double ToDouble(UltraNumber * a) {
	if (a == NULL)
		return 0.0;

	if (IsZero(a))
		return 0.0;

	ulong l = 0;

	if (a->Negative)
		l += 0x8000000000000000UL;

	ulong ll = 0;

	int x = a->Exponent;

	uint p;

	do {
		p = GetAt(a, x);
		if (p == 0)
			x--;
	} while (p == 0);

	int shift = 0;

	while ((p & 0x80000000U) == 0) {
		p <<= 1;
		shift++;
	}

	ll += (ulong) p << (21);

	ll += doshift((ulong) GetAt(a, x - 1), (21 - 32 + shift));

	ulong ll2 = doshift((ulong) GetAt(a, x - 2), 21 - 64 + shift);// << (21-shift);

	//Output(&ll2, 8);

	ll += ll2;

	//Output(&ll, 8);

	l += ll & 0xFFFFFFFFFFFFFUL;

	if (x >= 0) {
		x = x * 32 + (31 - shift);
	} else {
		x = x * 32 + 31 + (-shift);
	}

	//printf("new exponent = %d\n", x);

	x += 1023;

	if (x < 0 || x > 0x7FF)
		throw new std::overflow_error("Exponent");

	l += (ulong) x << 52;

	return *((double *) &l);
}

const char * PlaceValue = "0123456789ABCDEF";

char *_show(bool Negative, StringBuilder *Input, int exp, int expl) {
	expl++;

	char *ret;
	if (expl == 0) {
		ret = new char[exp + 2 + (Negative ? 1 : 0)];
	} else
		ret = new char[exp - expl + 3+ (Negative ? 1 : 0)];

	int x = 0;

	if (Negative) ret[x++]='-';

	for (int i = exp; i >= expl; i--) {
		if (i == -1) {
			ret[x++] = '.';
		}
		ret[x++] = Input->GetAt(exp - i);
	}

	ret[x++] = 0;

	return ret;
}

char *_showPadLeft(bool Negative, int pad, StringBuilder *Input, int exp, int expl) {
	expl++;

	char *ret;

	int x = 0;

	ret = new char[pad + exp - expl + 3 + (Negative ? 1 : 0)];

	if (Negative) ret[x++]='-';

	ret[x++] = '0';
	ret[x++] = '.';

	for (int i = 1; i < pad; i++) {
		ret[x++] = '0';
	}

	for (int i = exp; i >= expl; i--) {
		ret[x++] = Input->GetAt(exp - i);
	}

	ret[x++] = 0;

	return ret;
}

char *_showPadRight(bool Negative, int pad, StringBuilder *Input, int exp, int expl) {
	expl++;

	char *ret;

	int x = 0;

	ret = new char[pad + exp - expl + 2 + (Negative ? 1 : 0)];

	if (Negative) ret[x++]='-';

	for (int i = exp; i >= expl; i--) {
		ret[x++] = Input->GetAt(exp - i);
	}

	for (int i = 0; i < pad; i++) {
		ret[x++] = '0';
	}

	ret[x++] = 0;

	return ret;
}

char *_showScientific(bool Negative, StringBuilder *Input, int exp, int expl) {
	char Exp[1024];

	sprintf(Exp, "(e%i)", exp);
	int ExpLen = strlen(Exp);

	bool pnt = exp - expl > 1;

	expl++;

	char *ret;

	int x = 0;

	int len = exp - expl + 3 + ExpLen + (pnt ? 1 : 0) + (Negative ? 1 : 0);

	ret = new char[len + 1];
	ret[len] = 0xFF;

	if (Negative) ret[x++] = '-';

	int i = exp;

	ret[x++] = Input->GetAt(exp - i);

	if (pnt)
		ret[x++] = '.';

	i--;

	for (; i >= expl; i--) {
		ret[x++] = Input->GetAt(exp - i);
	}

	for (int i = 0; i < ExpLen; i++)
		ret[x++] = Exp[i];

	ret[x++] = 0;

	if (ret[len] != (char) 0xFF)
		asm("int $3");

	return ret;
}

char *ShowDids(bool Negative, StringBuilder *b, int exp, int expl) {
	char *s = NULL;

	if (exp >= 0 && (expl + 1) <= 0) {
		s = _show(Negative, b, exp, expl);
	}
	if (exp < 0) {
		int pad = -exp;
		if (pad <= 8)
			s = _showPadLeft(Negative, pad, b, exp, expl);
	} else if (expl >= 0) {
		int pad = expl + 1;
		if (pad <= 8)
			s = _showPadRight(Negative, pad, b, exp, expl);
	}
if (s == NULL)
s = _showScientific(Negative, b, exp, expl);
delete b;

return s;
}

char *OutHexRaw(UltraNumber *a) {
    StringBuilder *b = new StringBuilder();

    for (int i = a->Length - 1; i >= 0; i--) {
        uint d = a->Didgets[i];

        for (int p = 7; p >= 0; p--) {
            int x = (int) ((d >> (p << 2)) & 0xF);

            b->Add(PlaceValue[x]);
        }
    }

    char *s = b->ToString();
    delete b;
    return s;
}

char *ToStringHex(UltraNumber *a) {
    int exp = ((a->Exponent + 1) * 8) - 1;
    int expl = ((a->ExponentLow) * 8) - 1;

    StringBuilder *b = new StringBuilder();
    StringBuilder *b2;

    for (int i = a->Length - 1; i >= 0; i--) {
        uint d = a->Didgets[i];

        for (int p = 7; p >= 0; p--) {
            int x = (int) ((d >> (p << 2)) & 0xF);

            b->Add(PlaceValue[x]);
        }
    }
    b2 = b->ZeroTrim(&exp, &expl);
    delete b;
    b = b2;
    return ShowDids(a->Negative, b, exp, expl);

}

char * ToString(UltraNumber *a, TextMode mode) {
    if (mode == Hexadecimal)
        return ToStringHex(a);
    throw 1;

}

uint GetDid(char c) {
    uint ret;

    uint val = (uint) c;

    for (ret = 0; ret < 0x10; ret++) {
        if (PlaceValue[ret] == (char) val)
            break;
    }

    return ret;
}

void getCharString(char c, char *buffer)
{
    unsigned char _c = (unsigned char) c;

    if (c >= 32 &&_c != 127 && _c != 129)
    {
        sprintf(buffer, "\'%c\'", _c);
    } else sprintf(buffer, "[%i]", (int) _c);
}

UltraNumber *FromStringHex(const char *a) {
    UltraNumber *ret = new UltraNumber();

    int i = 0;
    char c;
    int place = 4;
    bool esign = false;
    int exp = 0;
    bool Negative = false;

    int l = (a == NULL) ? 0 : strlen(a);

    if (a == NULL) { sprintf(ERROR, "FromString: NULL input"); goto error; }

    if (i >= l) { sprintf(ERROR, "FromString: empty input"); goto error; }

    if (a[i] == '-') { Negative = true; i++; }
    else if (a[i] == '+') { Negative = false; i++; }

    if (i >= l) { sprintf(ERROR, "FromString: missing digits"); goto error; }

    while (i < l) {
        c = a[i];
        if (c == '.')
            goto decimal;
        if (c == '(')
            goto exponent;

        uint d = GetDid(c);

        if (d == 0x10) goto illegalChar;

        UltraNumber * _d = new UltraNumber(false, 0, &d, 1);

        Replace(&ret, Shift(ret, 4));
        Replace(&ret, Add(ret, _d));

        delete _d;
        i++;
    }

    if (i ==l) goto finish;

    decimal:
    c = a[i];
    if (c == '.') i++;
    if (c == '(')
        goto exponent;

    while (i<l)
    {
        c = a[i];

        if (c == '(') goto exponent;

        uint d = GetDid(c);

        if (d == 0x10) { goto illegalChar; }

        UltraNumber * _d = new UltraNumber(false, 0, &d, 1);

        Replace(&_d, Shift(_d, -place));
        Replace(&ret, Add(ret, _d));

        delete _d;
        i++;
        place += 4;
    }

    if (i == l) goto finish;
    exponent:
    c = a[i];
    if (c != '(') goto illegalChar;
    i++;
    if (i == l) { sprintf(ERROR, "FromString: premature termination[exponent]"); goto error; }
    c = a[i];
    if (c != 'e') goto illegalChar;
    i++;
    if (i == l) { sprintf(ERROR, "FromString: premature termination[exponent]"); goto error; }
    c = a[i];
    if (c == '+') { i++;} else if (c == '-') { esign = true; i++; }

    while (i < l) {
        c = a[i];
        if (c == ')')
        {	i++;;
            goto applyexponent;
        }

        uint d = GetDid(c);

        if (d > 9) goto illegalChar;


        exp *= 10; exp = exp + d;
        i++;
    }

    sprintf(ERROR, "FromString: premature termination[exponent]");
    goto error;
    applyexponent:
    if (esign) exp *= -1;
    Replace(&ret, Shift(ret, exp *4));

    if (i<l) { sprintf(ERROR, "FromString: premature termination"); goto error; }

    finish:
    ret->Negative = Negative;
    return ret;
    illegalChar:
    char t[1024];
    getCharString(c, t);
    sprintf(ERROR, "FromString: Ilegal Charachter[%s @ %d]", t, i);
    goto error;
    error:
    delete ret;
    return NULL;
}

UltraNumber *FromString(const char *a, TextMode mode) {
    if (mode == Hexadecimal)
        return FromStringHex(a);
    throw 1;
}

void FreeString(char *String) {
    delete String;
}


*/