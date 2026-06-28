# Mercury — Arbitrary-Precision Arithmetic Library

Mercury is a C library for arbitrary-precision floating-point arithmetic. It stores numbers in a base-2³² positional format and exposes every operation through a caller-supplied scratch-stack, making all computation deterministic in memory use and compatible with GPU (CUDA) execution alongside CPU.

The `.NET` binding (`Mercury.net`) wraps the C API in the `UltraNumber` class with operator overloading and thread-local stack management.

---

## Number Representation

A Mercury number is an array of `Precision + 2` unsigned 32-bit words (`uint[]`):

| Index | Contents |
|-------|----------|
| `[0]` | Flags — bit 0 is the sign (`1` = negative) |
| `[1]` | Base-2 exponent (stored as `uint`, interpreted signed) |
| `[2 … Precision+1]` | Mantissa words, most-significant first (base 2³²) |

`Precision` is the number of 32-bit mantissa words. Common choices:

| Precision | Bits | Approx decimal digits |
|-----------|------|-----------------------|
| 8 | 256 | ~77 |
| 16 | 512 | ~154 |
| 32 | 1024 | ~308 |
| 128 | 4096 | ~1233 |

All functions take `Precision` as an explicit parameter — the same buffer can be reinterpreted at a different precision by `mercuryLoadExtendMercury`.

---

## String Format

Mercury reads and writes numbers in **hexadecimal scientific notation**:

```
[-]HEXDIGITS[.HEXDIGITS][@[+-]HEXEXPONENT]
```

- The integer and fractional parts are hex digits (0–9, A–F).
- `@` separates the coefficient from the base-16 exponent (analogous to `e` in decimal notation).
- `+` / `-` prefixes are accepted; uppercase and lowercase hex are both valid on input.

Examples:
- `3.243F6A88...` — π in hex
- `2.B7E15162...` — e in hex
- `-1.8@3` — −1.5 × 16³

`mercuryHexStringGetPrecision` infers a suitable `Precision` value from a string, so you can parse an arbitrarily long hex literal without guessing the size upfront.

---

## The Scratch Stack

Every arithmetic function needs temporary workspace. Mercury manages this through a caller-owned byte buffer called the **stack**.

The stack is a contiguous block of memory. The first `sizeof(ulong)` bytes store the current allocation offset. `mercuryStackAlloc` bumps the offset forward; `mercuryStackFree` bumps it back. This makes scratch allocation O(1) and completely deterministic — no heap fragmentation, no GC pressure.

### Stack Lifecycle

```c
// Allocate a 2 MB scratch stack
void *stack = mercuryAllocateStack(2 * 1024 * 1024);

// ... do work ...

mercuryDeallocateStack(stack);
```

### Scratch Allocation Pattern

Inside a function that needs temporaries:

```c
slong mark = mercuryStackOffset(stack);            // optional: save mark

uint *tmp = mercuryStackAlloc(stack, (Precision + 2) * sizeof(uint));
// ... use tmp ...
mercuryStackFree(stack, (Precision + 2) * sizeof(uint));  // must match alloc size
```

Frees must be issued in reverse-allocation order (LIFO). There is no bounds checking — size the stack generously.

### Thread Safety

Each thread needs its own stack. In .NET, `UltraNumber.Imports.Stack` is `[ThreadStatic]`, so every thread allocates independently. The CUDA entry point `mercuryThreadedStackGet(thread, master)` returns a thread-indexed sub-stack from a shared master buffer.

---

## C API Reference

All symbols are declared in `Mercury.h` and `MercuryCPU.h`. On Windows they use `__declspec(dllexport/dllimport)`; on Linux the API has default visibility. Functions are annotated `__host__ __device__` for CUDA compatibility.

The `mercspec` macro expands to `extern "C" MERCURY_API`, giving clean C linkage from C++ callers.

### Stack Management (`MercuryCPU.h`)

```c
void *mercuryAllocateStack(long Size);
void  mercuryDeallocateStack(void *stack);

// Debug — prints the number to stdout in hex notation
void  mercuryPrint(void *stack, int Precision, uint *val);
```

### Stack Operations (`Mercury.h`)

```c
void *mercuryStackAlloc(void *stack, int size);   // bump-allocate `size` bytes
void  mercuryStackFree (void *stack, int size);   // LIFO free
slong mercuryStackOffset(void *stack);            // current byte offset

// GPU: get a per-thread sub-stack from a master buffer
void *mercuryThreadedStackGet(int thread, void *master);
```

### Load / Convert

```c
void mercuryLoadZero  (void *stack, int Precision, uint *val);
void mercuryLoadRaw   (void *stack, int Precision, uint *val,
                       bool Negative, int exp, uint *digits, int digitsLen);
void mercuryLoadMercury(void *stack, int Precision, uint *a, uint *val);       // copy a → val
void mercuryLoadExtendMercury(void *stack, int OldPrecision, int Precision,
                               uint *a, uint *val);                            // re-precision

void mercuryLoadInt   (void *stack, int Precision, uint *val, int    a);
void mercuryLoadUint  (void *stack, int Precision, uint *val, uint   a);
void mercuryLoadLong  (void *stack, int Precision, uint *val, slong  a);
void mercuryLoadUlong (void *stack, int Precision, uint *val, ulong  a);
void mercuryLoadDouble(void *stack, int Precision, uint *val, double a);
```

### Inspect

```c
int    mercuryNibbles(void *stack, int Precision, uint *val);          // hex digit count
double mercuryToDouble(void *stack, int Precision, uint *a);
bool   mercuryIsZero  (int Precision, uint *val);
uint   mercuryGetAt   (int Precision, uint *a, int Place);             // word at index
int    mercuryGetBit  (void *stack, int Precision, int Place, uint *a);
```

### String I/O

```c
int  mercuryHexStringGetPrecision(void *stack, const char *Input); // infer Precision
void mercuryFromString(void *stack, int Precision, const char *Input, uint *val);
int  mercuryToString  (void *stack, int Precision, uint *val, char *buffer, int len);
```

`mercuryToString` returns the number of bytes written (excluding the null terminator). Pass a buffer of at least `Precision * 9 + 32` bytes to be safe.

### Comparison

```c
int mercuryAbsCmp(int Precision, uint *a, uint *b); // ignore sign; returns −1, 0, +1
int mercuryCmp   (int Precision, uint *a, uint *b); // signed;  returns −1, 0, +1
```

Neither comparison function needs a stack.

### Shift and Power-of-Two

```c
void mercuryShift(void *stack, int Precision, uint *a, int shift, uint *val);
// shift > 0: left (multiply by 2^(32*shift)); shift < 0: right (divide)

void mercury_2Pow(void *stack, int Precision, int Place, uint *val);
// val = 2^Place  (Place may be negative)
```

### Arithmetic

Signed variants respect the sign bit; absolute variants operate on magnitudes only.

```c
void mercuryAdd   (void *stack, int Precision, uint *a, uint *b, uint *val);
void mercurySub   (void *stack, int Precision, uint *a, uint *b, uint *val);
void mercuryMul   (void *stack, int Precision, uint *a, uint *b, uint *val);
void mercuryDiv   (void *stack, int Precision, uint *a, uint *b, uint *val);

void mercuryAbsAdd(void *stack, int Precision, uint *a, uint *b, uint *val);
void mercuryAbsSub(void *stack, int Precision, uint *a, uint *b, uint *val);
void mercuryAbsMul(void *stack, int Precision, uint *a, uint *b, uint *val);
void mercuryAbsDiv(void *stack, int Precision, uint *a, uint *b, uint *val);
```

All output pointers (`val`) may alias an input (`a` or `b`) — the implementation handles aliasing internally by staging through scratch.

### Advanced Math

```c
void mercurySqr    (void *stack, int Precision, uint *a,           uint *val); // a²
void mercurySqrt   (void *stack, int Precision, uint *a,           uint *val); // √a
void mercuryPow    (void *stack, int Precision, uint *a, uint *b,  uint *val); // a^b
void mercuryRoot   (void *stack, int Precision, uint *a, uint *b,  uint *val); // a^(1/b)
void mercuryLog    (void *stack, int Precision, uint *a, uint *b,  uint *val); // log_b(a) fast
void mercuryLogSlow(void *stack, int Precision, uint *a, uint *b,  uint *val); // log_b(a) Taylor
```

`mercuryLog` and `mercuryLogSlow` both compute log base `b` of `a`. The fast variant converges more quickly for most inputs; the slow variant uses a straightforward Taylor series and is provided for reference or verification.

---

## C Usage Example

```c
#include "MercuryCPU.h"

int main(void)
{
    const int Precision = 32;   // 1024 bits
    void *stack = mercuryAllocateStack(4 * 1024 * 1024);

    uint a[Precision + 2];
    uint b[Precision + 2];
    uint r[Precision + 2];
    char buf[4096];

    mercuryLoadDouble(stack, Precision, a, 2.0);
    mercuryLoadDouble(stack, Precision, b, 0.5);

    mercuryPow(stack, Precision, a, b, r);   // r = 2^0.5 = √2

    mercuryToString(stack, Precision, r, buf, sizeof(buf));
    printf("sqrt(2) = %s\n", buf);           // hex: 1.6A09E667F3BCD...

    mercuryDeallocateStack(stack);
    return 0;
}
```

### Computing e via Taylor series (C)

```c
void ComputeE(void *stack, int Precision, uint *val)
{
    int P = Precision + 1;   // one extra word for guard digits

    uint *acc   = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));
    uint *n     = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));
    uint *fact  = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));
    uint *term  = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));
    uint *prev  = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));
    uint *one   = mercuryStackAlloc(stack, (P + 2) * sizeof(uint));

    mercuryLoadUint(stack, P, acc,  2);
    mercuryLoadUint(stack, P, n,    1);
    mercuryLoadUint(stack, P, fact, 1);
    mercuryLoadUint(stack, P, one,  1);

    do {
        mercuryAdd(stack, P, n, one, n);          // n++
        mercuryMul(stack, P, fact, n, fact);       // fact *= n
        mercuryDiv(stack, P, one, fact, term);     // term = 1/fact
        mercuryLoadMercury(stack, P, acc, prev);
        mercuryAdd(stack, P, acc, term, acc);      // acc += term
    } while (mercuryCmp(P, prev, acc) != 0);

    mercuryLoadRaw(stack, Precision, val,
                   acc[0] & 1, acc[1], acc + 2, P);

    for (int i = 0; i < 6; i++)
        mercuryStackFree(stack, (P + 2) * sizeof(uint));
}
```

---

## .NET API — `UltraNumber`

Namespace: `Mercury.net`  
Assembly: references `UltraNumber` project, which P/Invokes into `Mercury.dll` / `libMercury.so`.

### Setup (per thread)

```csharp
// Set precision to 128 words (4096 bits) and allocate a 1 MB scratch stack:
UltraNumber.SetPrecisionAndAllocateStack(128, 1024 * 1024);
```

`Precision` and `Stack` are `[ThreadStatic]` — call setup on every thread that will use `UltraNumber`.

The default precision if `SetPrecision` is never called is **8** (256 bits).

### Constructors and Conversions

```csharp
UltraNumber x = new UltraNumber();          // zero at current precision
UltraNumber x = new UltraNumber(3.14);     // from double
UltraNumber x = new UltraNumber(other);    // copy

// Implicit conversions
UltraNumber x = 42.0;                      // double → UltraNumber
UltraNumber x = "3.243F6A88...";          // hex string → UltraNumber (via Parse)

// Explicit conversion
double d = (double)x;
```

### Operators

| Operator | Operation |
|----------|-----------|
| `a + b` | Addition |
| `a - b` | Subtraction |
| `a * b` | Multiplication |
| `a / b` | Division |
| `a << n` | Left shift by `n` base-2³² places |
| `a >> n` | Right shift by `n` base-2³² places |
| `==`, `!=`, `<`, `<=`, `>`, `>=` | Signed comparison |

### Static Methods

```csharp
UltraNumber UltraNumber.Abs(UltraNumber a)
UltraNumber UltraNumber.Pow2(int place)          // 2^place
UltraNumber UltraNumber.Sqrt(UltraNumber a)      // √a
UltraNumber UltraNumber.Pow(UltraNumber a, UltraNumber b)   // a^b
UltraNumber UltraNumber.Log(UltraNumber a, UltraNumber b)   // log_b(a)
int         UltraNumber.Compare(UltraNumber a, UltraNumber b)
UltraNumber UltraNumber.Parse(string s)
UltraNumber UltraNumber.Interpolate(UltraNumber a, UltraNumber b, UltraNumber x)
                                                 // a + (b - a) * x
```

### Instance Methods

```csharp
string  x.ToString()   // hex string representation
bool    x.Equals(object obj)
int     x.GetHashCode()
```

### Precision Management

```csharp
UltraNumber.SetPrecision(int precision);
IntPtr UltraNumber.AllocateThreadStack(long size);    // returns the stack pointer
UltraNumber.SetPrecisionAndAllocateStack(int precision, long size);  // combined
```

Changing `Precision` mid-session is supported. Reading `Elements` after a precision change automatically re-packs the internal buffer.

### Serialization

`UltraNumber` implements `ISerializable`. It serializes as a `(Precision, Hex)` pair using `BinaryFormatter` or any `IFormatter`. Deserializing at a different active precision is handled correctly.

### .NET Usage Example

```csharp
using Mercury.net;

UltraNumber.SetPrecisionAndAllocateStack(64, 1024 * 1024);  // 2048-bit

UltraNumber e = 0;
UltraNumber factorial = 1;

for (int n = 0; n < 200; n++)
{
    if (n > 0) factorial *= (double)n;
    e += 1.0 / factorial;
}

Console.WriteLine(e);   // prints hex approximation of e
```

---

## Building

Mercury is built as a shared library (`SHARED` in CMake):

```cmake
add_library(Mercury SHARED Mercury.c MercuryCPU.c MercuryStrings.c)
target_compile_definitions(Mercury PRIVATE BUILD_MERCURY)
newage_configure_native_paths(Mercury)
shuttle(Mercury lib)
```

The `BUILD_MERCURY` definition triggers `dllexport` on Windows. Consumers must link against `Mercury` and include the `Mercury.h` / `MercuryCPU.h` headers.

The `.NET` project (`UltraNumber/UltraNumber.csproj`) targets **.NET 10.0** and locates the native library through the standard `DllImport("Mercury")` search path. The post-build step calls `NewAge_stage.sh` to copy the assembly into the workspace `DotNet/Libs` lane.

---

## Implementation Notes

- **No heap allocation during arithmetic** — all temporaries come from the caller-supplied stack. This makes Mercury suitable for real-time contexts and GPU kernels where `malloc` is unavailable.
- **Aliasing is safe** — output `val` may point to the same buffer as any input; the library stages intermediate results through scratch before writing the final answer.
- **Guard digits** — the test and demo code consistently computes at `Precision + 1` internally, then truncates to `Precision` on output via `mercuryLoadRaw`. This prevents the last word from accumulating rounding error.
- **Convergence loops** — iterative algorithms (e, π, log, sqrt) detect convergence by comparing the result before and after each term; the loop exits when two consecutive values are bit-identical at the working precision.
