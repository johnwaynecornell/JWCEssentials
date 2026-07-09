//
// Created by jwc on 9/11/21.
//

//#ifndef MERCURY_MERCURY_H
#define MERCURY_MERCURY_H

#ifndef uint
#define uint unsigned int
#endif

#ifdef __CUDACC__

#ifndef ulong
#define ulong unsigned long long
#endif

#ifndef slong
#define slong long long
#endif

#else

#ifndef ulong
#ifdef _WIN32
#define ulong unsigned long long
#else
#define ulong unsigned long
#endif
#endif

#ifndef slong
#ifdef _WIN32
#define slong long long
#else
#define slong long
#endif
#endif

#endif

#ifndef bool
#define bool int
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef __host__
#define __host__
#endif

#ifndef __device__
#define __device__
#endif

#ifndef MERCURY_API
    #ifdef _WIN32
        #ifdef BUILD_MERCURY
            #define MERCURY_API __declspec(dllexport)
        #else
            #define MERCURY_API __declspec(dllimport)
        #endif
    #else
        #define MERCURY_API
    #endif
#endif

#ifndef MERCURY_EXTERN
    #ifdef __cplusplus
        #define MERCURY_EXTERN extern "C"
    #else
        #define MERCURY_EXTERN
    #endif
#endif

#ifndef mercspec
    #define mercspec MERCURY_EXTERN MERCURY_API
#endif
/*
void mercuryLoadZero(struct MercuryStack *stack);
void mercuryLoadRaw(struct MercuryStack *stack, bool Negative, int exp, uint *digits, int digitsLen);
void mercuryLoadDouble(struct MercuryStack *stack, double a);
void mercuryToString(struct MercuryStack *stack, char *buffer, int len);
*/

mercspec __host__ __device__ void * mercuryThreadedStackGet(int thread, void *master);
mercspec __host__ __device__ void * mercuryStackAlloc(void *stack, int size);
mercspec  __host__ __device__ void mercuryStackFree(void *stack, int size);
mercspec __host__ __device__ slong mercuryStackOffset(void *stack);

mercspec  __host__ __device__ void mercuryLoadZero(void *stack,int Precision, uint *val);
mercspec  __host__ __device__ void mercuryLoadRaw(void *stack,int Precision, uint *val, bool Negative, int exp, uint *digits, int digitsLen);

mercspec  __host__ __device__ void mercuryLoadRawRounded(void *stack,int Precision, uint *val, bool Negative, int exp, uint *digits, int digitsLen);
mercspec  __host__ __device__ void mercuryRoundPlaces(void *stack, int Precision, uint *a, int places, uint *val);

mercspec  __host__ __device__ void mercuryLoadMercury(void *stack,int Precision, uint *a, uint *val);
mercspec  __host__ __device__ void mercuryLoadExtendMercury(void *stack,int OldPrecision, int Precision, uint *a, uint *val);

mercspec  __host__ __device__ void mercuryLoadInt(void *stack,int Precision, uint *val, int a);
mercspec  __host__ __device__ void mercuryLoadUint(void *stack,int Precision, uint *val, uint a);
mercspec  __host__ __device__ void mercuryLoadLong(void *stack,int Precision, uint *val, slong a);
mercspec  __host__ __device__ void mercuryLoadUlong(void *stack,int Precision, uint *val, ulong a);
mercspec  __host__ __device__ void mercuryLoadDouble(void *stack,int Precision, uint *val, double a);

mercspec  __host__ __device__ int mercuryNibbles(void *stack,int Precision, uint *val);
mercspec __host__ __device__ double mercuryToDouble(void *stack, int Precision, uint *a);

mercspec  __host__ __device__ int mercuryHexStringGetPrecision(void *stack, const char *Input);
mercspec  __host__ __device__ void mercuryFromString(void *stack, int Precision, const char *Input, uint *val);
mercspec  __host__ __device__ int mercuryToString(void *stack,int Precision, uint *val, char *buffer, int len);

mercspec  __host__ __device__ bool mercuryIsZero(int Precision, uint *val);
mercspec  __host__ __device__ uint mercuryGetAt(int Precision, uint *a, int Place);
mercspec  __host__ __device__ bool mercurySetAt(int Precision, uint *a, int Place, uint value);
mercspec  __host__ __device__ bool mercurySetAtWithNormalize(void *stack, int Precision, uint *a, int Place, uint value);

mercspec  __host__ __device__ int mercuryAbsCmp(int Precision, uint *a, uint *b);
mercspec  __host__ __device__ int mercuryCmp(int Precision, uint *a, uint *b);
mercspec  __host__ __device__ void mercuryShift(void *stack, int Precision, uint *a, int shift, uint *val);

mercspec  __host__ __device__ void mercuryAbsAdd(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryAbsSub(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryAbsMul(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryAbsDiv(void *stack, int Precision, uint *a, uint *b, uint *val);

mercspec  __host__ __device__ void mercuryAdd(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercurySub(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryMul(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryDiv(void *stack, int Precision, uint *a, uint *b, uint *val);

mercspec  __host__ __device__ void mercury_2Pow(void *stack, int Precision, int Place, uint *val);
mercspec  __host__ __device__ int mercuryGetBit(void *stack, int Precision, int Place, uint *a);
mercspec  __host__ __device__ void mercurySqr(void *stack, int Precision, uint *a, uint *val);
mercspec  __host__ __device__ void mercurySqrt(void *stack, int Precision, uint *a, uint *val);
mercspec  __host__ __device__ void mercuryPow(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryRoot(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryLogSlow(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec  __host__ __device__ void mercuryLog(void *stack, int Precision, uint *a, uint *b, uint *val);

//Support entries
mercspec __host__ __device__ bool mercuryHasFraction(void *stack, int Precision, uint *a);
mercspec __host__ __device__ bool mercuryIsInteger(void *stack, int Precision, uint *a);

mercspec __host__ __device__ void mercuryAbs(void *stack, int Precision, uint *a, uint *val);
mercspec __host__ __device__ void mercuryNeg(void *stack, int Precision, uint *a, uint *val);

mercspec __host__ __device__ void mercuryRound(void *stack, int Precision, uint *a, uint *val);
mercspec __host__ __device__ void mercuryTrunc(void *stack, int Precision, uint *a, uint *val);
mercspec __host__ __device__ void mercuryFloor(void *stack, int Precision, uint *a, uint *val);
mercspec __host__ __device__ void mercuryCeil(void *stack, int Precision, uint *a, uint *val);
mercspec __host__ __device__ void mercuryFrac(void *stack, int Precision, uint *a, uint *val);

mercspec __host__ __device__ void mercuryCopySign(void *stack, int Precision, uint *mag, uint *sign, uint *val);
mercspec __host__ __device__ void mercuryMin(void *stack, int Precision, uint *a, uint *b, uint *val);
mercspec __host__ __device__ void mercuryMax(void *stack, int Precision, uint *a, uint *b, uint *val);


//#endif //MERCURY_MERCURY_H
