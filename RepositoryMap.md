# JWCEssentials — Repository Map

This document provides a comprehensive architectural map and navigation guide for the `JWCEssentials` repository. It is designed to assist developers and AI agents in discovering components, understanding dependency flows, and navigating the codebase effectively.

---

## 1. Architectural Overview

`JWCEssentials` serves as the foundational substrate of the **NewAge** workspace ecosystem while also offering modular, standalone utilities. The repository is partitioned into three primary tiers:

1. **Native C/C++ Core Libraries & Utilities**:
   - `JWCEssentials`: Foundational systems library providing cross-platform OS abstractions, hashing algorithms (FarmHash integration via SMHasher), pseudo-random number generators (MT19937), string formatting, error handling, terminal escape sequencing, and thread-local storage.
   - `Mercury`: A high-performance, arbitrary-precision floating-point arithmetic engine utilizing deterministic, caller-managed scratch-stack allocation and a base-2³² positional representation.
2. **Managed .NET Libraries & Tooling**:
   - `MethodPack`: Modern typed CLI composition (`FluentCommandLine`), dataflow streaming/metrics (`JWCFarm`), and CLI expansion tooling (`CLIExpand` / `CLIExpander`).
   - `Mercury.net` (`UltraNumber`): High-level C# binding over native Mercury, featuring operator overloading, thread-static scratch stacks, and arbitrary-precision math functions.
   - `JWCEssentials.net`: Managed interop wrappers for core JWCEssentials primitives.
   - `CSProj`: Command-line utilities for programmatic inspection and manipulation of MSBuild `.csproj` project files.
   - `AnsiEffectSniffer`: Diagnostic CLI and test suite for terminal ANSI escape code handling.
3. **Workspace Automation & Shell Tooling (`Bash/`)**:
   - Workspace lifecycle and bootstrapping (`newage_go.sh`, `configure.sh`).
   - Coordinated multi-repository builds and dependency topological sorting (`newage_all_build_*.sh`, `newage_dep_sort.sh`).
   - Native and managed artifact staging (`NewAge_stage.sh`, `shuttle_to.sh`, `create_symlink.sh`).

---

## 2. Top-Level Directory Layout

| Path | Primary Language | Architectural Role | Key Contents |
|------|------------------|-------------------|--------------|
| `include/` | C / C++ Headers | Public native interface definitions | `include/JWCEssentials/*.h`, `JWCEssentials.cmake` |
| `src/` | C / C++ | Native implementation for `JWCEssentials` | Platform layers (`Platform_Linux/`, `Platform_Windows/`), `Hasher/`, `Random/`, `FromSMHasher/`, `Utils/` |
| `C/` | C / C++ | Isolated native subsystems | `C/Mercury/` (Arbitrary-precision math engine, headers, native tests) |
| `Project/` | C# (.NET) | Managed solutions, libraries, and tools | `MethodPack/`, `Mercury.net/`, `JWCEssentials.net/`, `CSProj/`, `AnsiEffectSniffer/` |
| `Bash/` | Bash Shell | Workspace management, build, and staging tooling | `newage_go.sh`, `NewAge_stage.sh`, `shuttle_to.sh`, `newage_all_*.sh` |
| `docs/` | Markdown | Canonical technical documentation & specifications | `Mercury.md`, `NewAgeEnvironment.md`, `NewAge_Prompt.md`, baseline guides |
| `examples/` | Configuration / Demos | Sample input assets and example files | `examples/feffect/first.feffect` |
| `lib/` | Shared Objects / DLLs | Staged native library output directory | `libJWCEssentials.so` symlink / staged targets |
| `Tools/` | Tooling directory | Workspace tool staging destination | Configured tool outputs |

---

## 3. Subsystem Breakdown

### 3.1 Native Core (`JWCEssentials`)
- **Headers**: `include/JWCEssentials/`
  - `JWCEssentials.h`: Master umbrella header.
  - `feffect.h`, `Terminal.h`, `Escapes.h`: Terminal emulation, formatting, and ANSI effects.
  - `HasherClass.h`, `HasherFactory.h`, `Hasher_SMHasher.h`: Hashing abstraction layer.
  - `Random_Generator.h`, `Random_MT19937.h`: Mersenne Twister and PRNG abstractions.
  - `Paths.h`, `TLS.h`, `ErrorSystem.h`: Cross-platform OS paths, thread-local storage, and diagnostic error tracing.
  - `utf8_string_struct.h`, `struct_array_struct.h`, `BufferHelper.h`: Native memory buffers and string utilities.
  - `JWCEssentials.cmake`: CMake macros (`newage_configure_native_paths`, `shuttle`) for workspace integration.
- **Source**: `src/`
  - Core implementations in `src/*.cpp`.
  - Platform isolation in `src/Platform_Linux/` and `src/Platform_Windows/`.
  - Bundled third-party algorithm implementations in `src/FromSMHasher/` (FarmHash).
  - Utility binaries built via root `CMakeLists.txt`: `feffect`, `rand_identifier`, `split_arg`, `test`, `newage_shell_touch`.

### 3.2 Native Math Engine (`Mercury`)
- **Location**: `C/Mercury/`
- **Documentation**: [`docs/Mercury.md`](docs/Mercury.md) (Canonical reference), [`C/Mercury/README.md`](C/Mercury/README.md)
- **Files**:
  - `Mercury.h`: Core declarations, arithmetic operations, and trigonometric/transcendental prototypes.
  - `MercuryCPU.h`: Host-side scratch stack allocator (`mercuryAllocateStack`, `mercuryDeallocateStack`).
  - `Mercury.c`: High-precision arithmetic routines (addition, subtraction, multiplication, division, powers, roots, logarithms).
  - `MercuryCPU.c`: Stack management and debug formatting.
  - `MercuryStrings.c`: Base-16 scientific notation string serialization and parsing.
  - `test.cpp`: Native benchmark and verification driver ($e$, $\pi$).
  - `stack_management.knowledge`: Formal knowledge-base specification of scratch stack behavior.
- **Build**: Built as a standalone shared library (`libMercury.so` / `Mercury.dll`) via `C/Mercury/CMakeLists.txt`.

### 3.3 Managed Ecosystem (`Project/`)

#### MethodPack (`Project/MethodPack/`)
- **Solution**: `Project/MethodPack/MethodPack.sln`
- **Guides**: `Project/MethodPack/README.md`, `FluentCommandLine.md`, `JWCFarm.md`
- **Components**:
  - `FluentCommandLine`: Strongly-typed, fluent command-line grammar composition framework with automatic help generation and execution routing.
  - `JWCFarm`: Streaming pipelines, metric projections, and operational telemetry.
  - `CLIExpand` & `CLIExpander`: Command line argument expansion and preprocessing tools.
  - `MethodTests`: Comprehensive unit test suite covering FluentCommandLine grammar, parsing, and execution.

#### Mercury.net & UltraNumber (`Project/Mercury.net/`)
- **Solution**: `Project/Mercury.net/Mercury.net.sln`
- **Projects**:
  - `UltraNumber/`: Main managed wrapper around `Mercury`. Implements `UltraNumber` class with arithmetic operator overloads (`+`, `-`, `*`, `/`, `<<`, `>>`), math methods (`Sqrt`, `Pow`, `Log`, `TwoPow`), custom string parsing, and thread-local stack lifecycle management.
  - `UltraNumber.Tests/`: xUnit test suite validating all arithmetic operations, precision stability, and numerical edge cases against the native engine.
  - `EDemo/`: High-precision estimation of Euler's number ($e$) to 4096 bits.
  - `AbsurdlyLargeDemo/`: Demonstration of exact arithmetic across extreme exponents (e.g., $2^{1000000}$).
  - `RootTest/` & `SimpleTest/`: Focused test drivers for root extraction and basic expressions.

#### Other Managed Projects
- **`JWCEssentials.net`**: (`Project/JWCEssentials.net/`) Managed interop bindings for JWCEssentials native APIs.
- **`CSProj`**: (`Project/CSProj/`) Collection of MSBuild project utilities (`csproj_add_reference`, `csproj_add_property`, `csproj_list_references`, `csproj_add_my_references`).
- **`AnsiEffectSniffer`**: (`Project/AnsiEffectSniffer/`) CLI and test suite for evaluating ANSI terminal rendering capabilities.

### 3.4 Workspace Automation & Shell Tooling (`Bash/`)
- **Bootstrap & Configuration**:
  - `newage_go.sh`: End-to-end bootstrap script to create a workspace, clone repositories, configure environment paths, and run coordinated builds.
  - `configure.sh`: Registers JWCEssentials into a NewAge workspace, creates symlinks for headers under `$NewAge/include`, tools under `$NewAge/bin`, and libraries under `$NewAge/lib`.
- **Build Orchestration**:
  - `newage_all_build_coordinated.sh`, `newage_all_build_native.sh`, `newage_all_build_managed.sh`: Multi-repo coordination scripts.
  - `newage_dep_sort.sh`, `newage_get_deps.sh`: Dependency tree resolution and topological build sorting.
- **Artifact Staging**:
  - `NewAge_stage.sh`: MSBuild post-build target script staging .NET assemblies to `$NewAge/DotNet/Libs/lib/$(Configuration)/$(TargetFramework)/`.
  - `shuttle_to.sh`, `create_symlink.sh`: Helper scripts deploying native binaries and libraries to appropriate workspace lanes.

---

## 4. Build and Dependency Relationships

```
                     ┌────────────────────────────────┐
                     │     NewAge Workspace Root      │
                     │  (Environment Variable $NewAge)│
                     └───────────────┬────────────────┘
                                     │
           ┌─────────────────────────┴────────────────────────┐
           ▼                                                  ▼
┌─────────────────────────┐                        ┌────────────────────────┐
│   Native C/C++ Build    │                        │    Managed .NET Build   │
│  (CMake 3.28+ / C++17)  │                        │    (.NET 8.0 / 10.0)   │
└──────────┬──────────────┘                        └──────────┬─────────────┘
           │                                                  │
     ┌─────┴──────────────┐                             ┌─────┴──────────────┐
     ▼                    ▼                             ▼                    ▼
┌──────────────┐   ┌──────────────┐             ┌──────────────┐     ┌──────────────┐
│JWCEssentials │   │   Mercury    │             │UltraNumber   │     │  MethodPack  │
│(libJWCEssen- │   │ (libMercury. │◄──P/Invoke──│(Mercury.net) │     │ (FluentCLI,  │
│ tials.so)    │   │  so)         │             └──────────────┘     │  JWCFarm)    │
└──────┬───────┘   └──────┬───────┘                                  └──────────────┘
       │                  │                                                 │
       └──────────┬───────┘                                                 │
                  ▼                                                         ▼
       ┌────────────────────────┐                         ┌────────────────────────┐
       │ Native Staging (CMake) │                         │ Managed Staging (.NET) │
       │ $NewAge/lib/...        │                         │ $NewAge/DotNet/Libs/.. │
       └────────────────────────┘                         └────────────────────────┘
```

### Staging Rules and Output Destinations
- **Native Headers**: Exported to `$NewAge/include/JWCEssentials/` via `configure.sh`.
- **Native Libraries**: Staged to `$NewAge/lib/$(Configuration)/$(NewAge_Lane)/` and symlinked in repository `lib/`.
- **Managed Assemblies**: Staged to `$NewAge/DotNet/Libs/lib/$(Configuration)/$(TargetFramework)/` via `NewAge_stage.sh`.
- **Tool Executables**: Deployed to `$NewAge/bin/` for direct access in workspace shell sessions.

---

## 5. Confirmed Facts vs. Inferred Relationships

### Confirmed Facts (Source-Verified)
1. **Target Frameworks**:
   - `JWCEssentials.net`: Multi-targets `net8.0` and `net10.0`.
   - `Mercury.net` (`UltraNumber`, `UltraNumber.Tests`, and demos): Targets `net10.0`.
   - `MethodPack` (`FluentCommandLine`, `JWCFarm`, `MethodTests`): Multi-targets `net8.0` and `net10.0`.
2. **Mercury Numerical Contract**:
   - Array layout: `[0]` = flags/sign bit, `[1]` = signed base-$2^{32}$ word exponent, `[2 … Precision+1]` = mantissa words stored **least-significant word first** (ascending place order).
   - `mercuryShift` and `UltraNumber` `<<` / `>>` operators perform **bit shifts** (base 2, multiplying or dividing by $2^{\text{shift}}$), not base-$2^{32}$ word shifts.
   - All scratch allocations operate via caller-provided bump-allocated memory buffers (`mercuryStackAlloc` / `mercuryStackFree`).
   - UltraNumber xUnit test suite contains 30 passing tests.
3. **Workspace Staging**:
   - Post-build MSBuild events invoke `bash NewAge_stage.sh` using the `NewAge` environment variable.
   - CMake native builds use `newage_configure_native_paths` and `shuttle` macros defined in `include/JWCEssentials/JWCEssentials.cmake`.

### Inferred / Contextual Relationships
1. **Repository Autonomy**:
   - `JWCEssentials` can be cloned and built independently for standalone native C++ and managed usage, but optimal automated multi-repo workflow assumes the presence of a NewAge workspace root (`$NewAge`).
2. **CUDA / Accelerator Compatibility**:
   - In `C/Mercury/`, the stack management design (`mercuryThreadedStackGet`) and isolation of memory from host runtime heap allocations (`malloc`) are structured to support future or alternate GPU kernel execution alongside CPU host processing.

---

## 6. Key Developer Entry Points

| Task / Area | Starting Point | Primary References |
|-------------|----------------|--------------------|
| **Workspace Setup** | `./Bash/newage_go.sh` or `./configure.sh` | [`README.md`](README.md), [`docs/NewAgeEnvironment.md`](docs/NewAgeEnvironment.md) |
| **Native Library Build** | `cmake -S . -B cmake-build-debug && cmake --build cmake-build-debug` | [`CMakeLists.txt`](CMakeLists.txt), [`include/JWCEssentials/JWCEssentials.h`](include/JWCEssentials/JWCEssentials.h) |
| **Mercury Native Engine** | `C/Mercury/` | [`C/Mercury/README.md`](C/Mercury/README.md), [`docs/Mercury.md`](docs/Mercury.md) |
| **Mercury .NET / Tests** | `dotnet test Project/Mercury.net/Mercury.net.sln` | [`Project/Mercury.net/TESTING.md`](Project/Mercury.net/TESTING.md) |
| **FluentCommandLine / MethodPack** | `Project/MethodPack/` | [`Project/MethodPack/README.md`](Project/MethodPack/README.md), [`FluentCommandLine.md`](Project/MethodPack/FluentCommandLine.md) |
| **Development Docket** | [`Docket.md`](Docket.md) | Work tracking and upcoming tasks |
