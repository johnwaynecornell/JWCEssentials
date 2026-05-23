# NewAge Environment

> The NewAge environment is the shared workspace, staging, and build convention used by the NewAge-family repositories.

## Purpose

This document describes the common environment assumptions shared by repositories such as `NewAge`, `JWCEssentials`, `JWCCommandSpawn`, and `CrystalCatalystLibrary`.

Individual repositories may have their own build instructions, but they should agree on the environment contract described here.

## Shared Repositories

*   `NewAge` — main workspace and higher-level library/application tree.
*   `JWCEssentials` — foundational native and managed utility layer.
*   `JWCCommandSpawn` — command/process execution support.
*   `CrystalCatalystLibrary` — native platform layer and generated managed bindings.

## Required Environment Variable

The primary shared variables are `NewAge`, `NewAge_Config`, and `NewAge_Lane`.

It should point to the root of the main NewAge workspace.

### Linux / Bash

```
export NewAge="$HOME/NewAge"
```

### Windows PowerShell

```
[Environment]::SetEnvironmentVariable("NewAge", "C:\src\NewAge", "User")
```

### Temporary PowerShell Session

```
$env:NewAge = "C:\src\NewAge"
```

## Staged Library Layout

Many .NET projects use a shared staged library path constructed from the `NewAge` environment variable.

```
$(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)
```

Example paths:

```
$NewAge/DotNet/Libs/lib/Debug/net8.0 $NewAge/DotNet/Libs/lib/Debug/net10.0
```

## Build Staging Script

Many projects use a post-build step that invokes:

```
bash NewAge_stage.sh
```

This means the build environment must provide:

*   `bash`
*   a resolvable `NewAge_stage.sh`
*   a valid `NewAge` environment variable
*   a writable staged library output directory

## .NET SDK Expectations

Current NewAge-family development uses modern .NET targets, especially:

*   `net8.0` for compatibility and older lanes
*   `net10.0` for active forward development

A fresh system should verify installed SDKs with:

```
dotnet --list-sdks dotnet --list-runtimes
```

## Native Build Expectations

Native projects may require:

*   CMake
*   a C or C++ compiler
*   platform-specific build tools
*   Bash-compatible staging scripts

On Windows, this may include Visual Studio Build Tools, CMake, Git, and a Bash provider such as Git Bash or MSYS2.

## Workspace Root

The `NewAge` environment variable points to the root of the shared NewAge-family workspace.

```
$NewAge/
  NewAge/
  JWCEssentials/
  JWCCommandSpawn/
  CrystalCatalystLibrary/
  include/
  bin/
  lib/
  DotNet/
    Libs/
      lib/
  NewAgeRepo.lst
```

Repositories are expected to live physically under `$NewAge`. Shared build-facing surfaces such as `include`, `bin`, `lib`, and `DotNet/Libs/lib` are workspace-level paths.

## Repository List

`$NewAge/NewAgeRepo.lst` is a simple reference list of repository directories relative to `$NewAge`.

```
# NewAgeRepo.lst
# Relative repository paths under $NewAge.

NewAge
JWCEssentials
JWCCommandSpawn
CrystalCatalystLibrary
```

This file is not a package manifest. It does not define URLs, branches, or dependency rules. It is only a lightweight workspace index.

## Shared Include Directory

`$NewAge/include` is part of the build contract. Repositories that expose native headers should register their public include directory there.

```
$NewAge/include/JWCEssentials
```

On Linux this is normally a symbolic link. On Windows this may be a junction or another supported live directory registration. It should not silently become a copied source tree unless a repository explicitly documents that as a build-only fallback.

## Command Script Directory

`$NewAge/bin` contains shared helper scripts such as staging and link helpers.

Scripts may be copied into `$NewAge/bin` because they are command utilities, not source-of-truth directories.

```
export PATH="$PATH:$NewAge/bin"
```

## Lane Environment Variables

While `NewAge` defines the workspace root, the specific build and runtime environment is managed through lane-specific variables.

*   **`NewAge_Config`** — The build configuration (e.g., `Debug`, `Release`).
*   **`NewAge_Lane`** — The platform-specific native lane (e.g., `Linux/x86_64/gcc`, `Windows/AMD64/msvc`).

These variables are used to construct the paths to staged native artifacts.

## Native Artifact Lanes

Native build outputs are staged by configuration and the platform lane.

```
$NewAge/bin/<NewAge_Config>/<NewAge_Lane>
$NewAge/lib/<NewAge_Config>/<NewAge_Lane>
```

Examples:

```
$NewAge/bin/Debug/Windows/AMD64/msvc
$NewAge/lib/Debug/Windows/AMD64/msvc
$NewAge/bin/Debug/Linux/x86_64/gcc
$NewAge/lib/Debug/Linux/x86_64/gcc
$NewAge/bin/Debug/Linux/x86_64/clang
$NewAge/lib/Debug/Linux/x86_64/clang
```

A standalone repository may support multiple native toolchains. A full NewAge suite build should consume one selected native lane consistently.

## Context Management

The preferred way to work within a specific lane is to use a context wrapper script. These scripts set `NewAge_Config`, `NewAge_Lane`, `PATH`, and `LD_LIBRARY_PATH` (on Linux) correctly for the chosen context.

### in_this_context.sh

A self-contained script installed to the root of the `$NewAge` workspace.

```bash
cd "$NewAge"
./in_this_context.sh Debug -- bash
```

### newage_run_in_context.sh

A global helper installed to `$NewAge/bin`, which should be on your `PATH`.

```bash
newage_run_in_context.sh Release -- dotnet build ...
```

## Runtime Path Notes

Manual path configuration is possible but discouraged in favor of the context wrappers.

On Windows, native executables may need both the staged bin lane and staged lib lane on `PATH` so dependent DLLs can be found.

```
export PATH="$NewAge/bin:$PATH"
export PATH="$NewAge/bin/Debug/Windows/AMD64/msvc:$PATH"
export PATH="$$NewAge/lib/Debug/Windows/AMD64/msvc:$PATH"
```

On Linux, staged command executables belong on `PATH`, while staged shared libraries may need `LD_LIBRARY_PATH`, an `ld.so.conf.d` entry, or a later rpath/RUNPATH policy.

```
export PATH="$NewAge/bin:$PATH"
export PATH="$NewAge/bin/Debug/Linux/x86_64/gcc:$PATH"
export LD_LIBRARY_PATH="$NewAge/lib/Debug/Linux/x86_64/gcc:${LD_LIBRARY_PATH:-}"
```

## Managed Artifact Staging

Managed outputs stage under:

```
$NewAge/DotNet/Libs/lib/<Configuration>/<TargetFramework>
```

Repository project directories are no longer required to be linked into `$NewAge/DotNet/Libs`. The repositories themselves live directly under `$NewAge`.

## Repository-Specific Notes

This document defines only the shared environment. Repository-specific build instructions should remain in each repository's own README or docs folder.

*   `NewAge` should document bootstrap/build order.
*   `JWCEssentials` should document native and managed utility builds.
*   `JWCCommandSpawn` should document process/pipe/runtime assumptions.
*   `CrystalCatalystLibrary` should document native C++ builds, generated bindings, platform layers, and managed interop.

## Bootstrapping a Workspace

The `newage_go.sh` tool (located in `JWCEssentials/Bash/` and installed to `$NewAge/bin/`) is designed to simplify the creation and maintenance of a NewAge workspace.

### Standard Bootstrap

To create a new workspace and fetch core repositories:

```bash
./JWCEssentials/Bash/newage_go.sh ~/NewAge JWCCommandSpawn CrystalCatalystLibrary
```

This file remains useful when copied out of the repo.

### Bootstrap and Build

To bootstrap and immediately run a coordinated build:

```bash
./JWCEssentials/Bash/newage_go.sh ~/NewAge JWCCommandSpawn --build Debug
```

This command will:
1. Ensure `JWCEssentials` is present and configured.
2. Fetch specified repositories (and their dependencies).
3. Run `newage_all_configure.sh` to set up all repositories.
4. Run `newage_all_build_coordinated.sh` to build the entire suite.

## Fresh-System Verification

A fresh machine or VM should verify the shared environment before attempting full builds.

```
echo "$NewAge" 
dotnet --list-sdks 
bash --version 
cmake --version
```

On Windows PowerShell:

```
echo $env:NewAge 
dotnet --list-sdks 
bash --version 
cmake --version
```

## Design Note

The NewAge environment is a shared development substrate. It is intended to make multiple repositories behave as one coherent workspace without forcing all source code into a single repository.

```
NewAgeEnvironment → shared paths → staged libraries → Bash tooling → native and managed build continuity → reproducible fresh-system setup
```