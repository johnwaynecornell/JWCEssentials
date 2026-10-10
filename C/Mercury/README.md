# Mercury (Native C Engine)

Mercury is a high-performance, arbitrary-precision floating-point arithmetic library written in C. It employs a base-2³² positional representation and caller-managed scratch-stack memory architecture to achieve deterministic, allocation-free numerical evaluation suitable for both CPU and accelerator/GPU runtimes.

## Key Capabilities

- **Arbitrary Precision**: Variable word-length precision (configurable from 256 bits to thousands of bits).
- **Caller-Supplied Scratch Stack**: O(1) bump-allocation without heap fragmentation or garbage collection overhead.
- **Algebraic Power Tier**: Full precision operations across addition, subtraction, multiplication, division, square root, power, root, and logarithm.
- **Hexadecimal Scientific String I/O**: Exact representation and parsing using hex notation (`[-]HEX[.HEX][@[+-]HEXEXP]`).

## Directory Contents

| File | Description |
|------|-------------|
| `Mercury.h` | Core C API declarations, arithmetic functions, support routines, and export macros. |
| `MercuryCPU.h` | Host-side memory allocation (`mercuryAllocateStack`) and debug printing helpers. |
| `Mercury.c` | Core arithmetic, shifting, comparison, transcendental algorithms (`mercuryPow`, `mercurySqrt`, `mercuryLog`). |
| `MercuryCPU.c` | CPU stack lifecycle and host helper implementations. |
| `MercuryStrings.c` | Exact hex string parsing (`mercuryFromString`) and formatting (`mercuryToString`). |
| `CMakeLists.txt` | CMake build definition producing the `Mercury` shared library and native `test` executable. |
| `test.cpp` | Native test driver, performance benchmarks, and high-precision constant computations (e, π). |
| `stack_management.knowledge` | Structural knowledge-base document describing scratch stack allocation semantics. |

## Navigation & Related Components

- **Canonical Specification & API Reference**: [`../../docs/Mercury.md`](../../docs/Mercury.md) — Comprehensive technical guide, number representation details, and full C & .NET API references.
- **Managed .NET Binding (`UltraNumber`)**: [`../../Project/Mercury.net/UltraNumber/`](../../Project/Mercury.net/UltraNumber/) — C# class library with operator overloading, automatic thread-local stack binding, and serialization.
- **Unit Tests**: [`../../Project/Mercury.net/UltraNumber.Tests/`](../../Project/Mercury.net/UltraNumber.Tests/) — xUnit test suite for managed bindings and underlying native engine.
- **Demonstrations & Applications**:
  - [`../../Project/Mercury.net/EDemo/`](../../Project/Mercury.net/EDemo/) — Progressive estimation of Euler's number ($e$) to 4096 bits.
  - [`../../Project/Mercury.net/AbsurdlyLargeDemo/`](../../Project/Mercury.net/AbsurdlyLargeDemo/) — Exact power-of-two, giant roots, and astronomical logarithm scale demonstrations.
  - [`../../Project/Mercury.net/RootTest/`](../../Project/Mercury.net/RootTest/) & [`../../Project/Mercury.net/SimpleTest/`](../../Project/Mercury.net/SimpleTest/) — Precision root and basic operation tests.

## Building

Native build via CMake:

```bash
cmake -S C/Mercury -B build
cmake --build build
```

When building inside a configured NewAge workspace, the native library is automatically staged to `$NewAge/lib/$(Configuration)/$(NewAge_Lane)/` and linked by consuming .NET and native projects.
