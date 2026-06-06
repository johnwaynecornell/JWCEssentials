# Reference.md

## Purpose
This document serves as an orientation guide for AI assistants and maintainers of **JWCEssentials**. It maps the repository's structure, exported entry points, Bash workspace tooling, architectural seams, and safe extension points. This is a practical map, not exhaustive API documentation.

## Repository Overview
**JWCEssentials** is the foundation of the NewAge workspace. It has two distinct responsibilities:

1. **C++ utility library** (`libJWCEssentials.so` / `.dll`) providing hashing, random generation, thread-local storage (TLS), terminal effect codes, UTF-8 string handling, command-line escaping, and path utilities — with managed .NET wrappers for all major subsystems.
2. **NewAge workspace Bash tooling** — the canonical source for scripts that are copied to `$NewAge/bin`, and the `configure.sh` that establishes the collective workspace layout.
3. **CSProj managed reference tooling** — a suite of .NET CLI tools for manipulating `.csproj` files, with `csproj_add_my_references` as the primary entry point for wiring downstream projects to consume managed assemblies from the NewAge reference path.

- **Primary Languages**: C++, Bash, C#/.NET 10.0
- **Target Platforms**: Windows and Linux
- **Major Projects**:
    - **Native Core**: C++ library (`CMakeLists.txt`) with subsystems: Hashing, Random, TLS, feffect/Terminal, Escapes, Paths, BufferHelper, utf8_string_struct.
    - **Managed .NET Wrapper**: `Project/JWCEssentials.net/JWCEssentials.net.sln` — P/Invoke bindings for native subsystems.
    - **AnsiEffectSniffer**: `Project/AnsiEffectSniffer/AnsiEffectSniffer.sln` — standalone ANSI escape sequence parser.
    - **Standalone Utilities**: `feffect`, `rand_identifier`, `split_arg` (from `src/Utils/`); `spawn_bash_probe` (from `Tools/`).
    - **Bash Workspace Tooling**: `Bash/` — all scripts; `configure.sh` — workspace bootstrap.
    - **CSProj Tooling**: `Project/CSProj/CSProj.slnx` — `.csproj` manipulation tools; staged to `$NewAge/bin/`.
- **Primary Solution/Project Files**:
    - `CMakeLists.txt` (Native build)
    - `Project/JWCEssentials.net/JWCEssentials.net.sln` (.NET wrapper)
    - `Project/AnsiEffectSniffer/AnsiEffectSniffer.sln` (AnsiEffectSniffer)
    - `Project/CSProj/CSProj.slnx` (CSProj tooling)

## Orientation for AI Assistants
- **Bash scripts are not live source**: `$NewAge/bin/` is populated by copying from `Bash/` during `configure.sh`. Edit scripts in `Bash/`, then re-run configure to propagate changes.
- **Include headers are registered, not duplicated**: `include/JWCEssentials/` is registered into `$NewAge/include/JWCEssentials` via symlink by configure. Edit headers in `include/JWCEssentials/`, not in the workspace symlink target.
- **_EXPORT_ must stay single-line**: The `_EXPORT_` convention is grep-friendly by design. Do not split across lines.
- **Check existing helpers before adding**: Consult `HasherFactory`, `BufferHelper`, `utf8_string_struct` before introducing new abstractions.
- **Repo-relative paths only**: In notes, documentation, and when referencing symbols, always use paths relative to the repository root.
- **Platform specificity**: `src/Platform_Linux/` and `src/Platform_Windows/` are platform-specific. Do not assume neutrality within these paths.

## Build / Runtime Assumptions
- **Native library**: Built via CMake. Output (`libJWCEssentials.so` / `JWCEssentials.dll`) is staged to `$NewAge/lib/<Config>/<Lane>/`.
- **Managed wrapper**: Targets `net10.0`. Relies on `DllImport` to call `libJWCEssentials`. Must find the native library at runtime (set `LD_LIBRARY_PATH` on Linux or stage both outputs side-by-side).
- **Workspace bootstrap**: `configure.sh` requires `$NewAge` to be set before running. It creates the workspace directory structure, installs `in_this_context.sh`, registers include headers, and copies all Bash scripts to `$NewAge/bin`.
- **Lane system**: Build outputs are staged under `$NewAge/lib/<Config>/<Lane>` (e.g., `Debug/Linux_x86_64`). `in_this_context.sh` enters a specific lane environment.
- **No managed DotNet/Libs registration**: `$NewAge/DotNet/Libs/JWCEssentials.net` is intentionally not registered. Managed artifacts stage directly to `$NewAge/DotNet/Libs/lib`.

## Native / Exported Entry Points

### Hashing

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ HasherFactory_Get` | `include/JWCEssentials/HasherFactory.h` | Returns a HasherClass instance by name. | Caller owns; must call `HasherClass_delete`. |
| `_EXPORT_ HasherClass_delete` | `include/JWCEssentials/HasherClass.h` | Destroys a HasherClass instance. | |
| `_EXPORT_ HasherClass_Hash_Begin` | `include/JWCEssentials/HasherClass.h` | Begins an incremental hash operation. | |
| `_EXPORT_ HasherClass_Compute` | `include/JWCEssentials/HasherClass.h` | Feeds data into the current hash. | |
| `_EXPORT_ HasherClass_Compute_Raw` | `include/JWCEssentials/HasherClass.h` | Feeds data without endian adjustment. | |
| `_EXPORT_ HasherClass_Compute_Rev` | `include/JWCEssentials/HasherClass.h` | Feeds data in reversed byte order. | |
| `_EXPORT_ HasherClass_Hash_End` | `include/JWCEssentials/HasherClass.h` | Finalizes the hash. | |
| `_EXPORT_ HasherClass_get_identifier` | `include/JWCEssentials/HasherClass.h` | Returns hasher's name/identifier string. | |
| `_EXPORT_ HasherClass_get_bits` | `include/JWCEssentials/HasherClass.h` | Returns hash output width in bits. | |
| `_EXPORT_ HasherClass_get_endian` | `include/JWCEssentials/HasherClass.h` | Returns current endian setting. | |
| `_EXPORT_ HasherClass_set_endian` | `include/JWCEssentials/HasherClass.h` | Sets endian mode for hash computation. | |
| `_EXPORT_ HasherClass_get_Swap` | `include/JWCEssentials/HasherClass.h` | Returns byte-swap flag. | |
| `_EXPORT_ HasherClass_set_Swap` | `include/JWCEssentials/HasherClass.h` | Sets byte-swap flag. | |
| `_EXPORT_ HasherClass32_get_value` | `include/JWCEssentials/HasherClass32.h` | Returns current 32-bit hash value. | |
| `_EXPORT_ HasherClass32_get_seed` | `include/JWCEssentials/HasherClass32.h` | Returns seed for 32-bit hasher. | |
| `_EXPORT_ HasherClass32_set_seed` | `include/JWCEssentials/HasherClass32.h` | Sets seed for 32-bit hasher. | |
| `_EXPORT_ HasherClass64_get_value` | `include/JWCEssentials/HasherClass64.h` | Returns current 64-bit hash value. | |
| `_EXPORT_ HasherClass64_get_seed` | `include/JWCEssentials/HasherClass64.h` | Returns seed for 64-bit hasher. | |
| `_EXPORT_ HasherClass64_set_seed` | `include/JWCEssentials/HasherClass64.h` | Sets seed for 64-bit hasher. | |

### Random Generation

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ Random_MT19937_Create` | `include/JWCEssentials/Random_MT19937.h` | Creates a Mersenne Twister RNG with given seed. | Caller owns; must call `Random_Destroy`. |
| `_EXPORT_ Random_Destroy` | `include/JWCEssentials/Random_Generator.h` | Destroys a Random_Generator instance. | |
| `_EXPORT_ Random_Generator_Reset` | `include/JWCEssentials/Random_Generator.h` | Resets the RNG to initial state. | |
| `_EXPORT_ Random_Generator_SetSeed` | `include/JWCEssentials/Random_Generator.h` | Sets the RNG seed. | |
| `_EXPORT_ Random_Generator_GetSeed` | `include/JWCEssentials/Random_Generator.h` | Returns the current seed. | |
| `_EXPORT_ Random_Generator_Get_uint32_t` | `include/JWCEssentials/Random_Generator.h` | Returns a random uint32. | |
| `_EXPORT_ Random_Generator_Get_uint64_t` | `include/JWCEssentials/Random_Generator.h` | Returns a random uint64. | |
| `_EXPORT_ Random_Generator_Get_double` | `include/JWCEssentials/Random_Generator.h` | Returns a random double in [0,1). | |
| `_EXPORT_ Random_Generator_GetByte` | `include/JWCEssentials/Random_Generator.h` | Returns a random byte. | |
| `_EXPORT_ Random_Generator_cstyle_identifier` | `include/JWCEssentials/Random_Generator.h` | Generates a random C-style identifier of given length. | |
| `_EXPORT_ Random_Generator_get_state` | `include/JWCEssentials/Random_Generator.h` | Serializes RNG state to a byte array. | |
| `_EXPORT_ Random_Generator_set_state` | `include/JWCEssentials/Random_Generator.h` | Restores RNG state from a byte array. | |

### Thread-Local Storage (TLS)

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ TLS_Alloc` | `include/JWCEssentials/TLS.h` | Allocates a TLS slot with optional initializer and destructor. | Caller owns; must call `TLS_Free`. |
| `_EXPORT_ TLS_get` | `include/JWCEssentials/TLS.h` | Returns the thread-local value for this slot. | |
| `_EXPORT_ TLS_Free` | `include/JWCEssentials/TLS.h` | Frees the TLS slot. | |

### Terminal / feffect

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ JWCEssentials_EnableTerminalEffects` | `include/JWCEssentials/JWCEssentials.h` | Enables ANSI terminal effects for the process. | |
| `_EXPORT_ feffect_list` | `include/JWCEssentials/feffect.h` | Returns list of all known effect names. | |
| `_EXPORT_ feffect_code` | `include/JWCEssentials/feffect.h` | Returns the ANSI escape code for a named effect. | |
| `_EXPORT_ feffect_name` | `include/JWCEssentials/feffect.h` | Returns the effect name for a given code. | |
| `_EXPORT_ feffect` | `include/JWCEssentials/feffect.h` | Returns effect code formatted for output, with optional escape. | |

### Escapes / String

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ escapeStringForCommandLine` | `include/JWCEssentials/Escapes.h` | Escapes a string for safe command-line use (platform-appropriate). | |
| `_EXPORT_ escapeStringForCommandLine_Linux` | `include/JWCEssentials/Escapes.h` | Linux-specific command-line escaping. | |
| `_EXPORT_ escapeStringForCommandLine_Windows` | `include/JWCEssentials/Escapes.h` | Windows-specific command-line escaping. | |
| `_EXPORT_ StartingWith` | `include/JWCEssentials/utf8_string_struct.h` | Tests if a utf8_string_struct starts with a given prefix. | |
| `_EXPORT_ PlatformLineEnding` | `include/JWCEssentials/Paths.h` | Returns the line ending string for the current platform. | |

### Error System

| Symbol | File | Purpose | Notes |
|---|---|---|---|
| `_EXPORT_ testErrorSystem` | `include/JWCEssentials/ErrorSystem.h` | Diagnostic test for the error system. | Test/diagnostic use only. |

## Managed Class Map

### Core Interop (JWCEssentials.net)

#### `JWCEssentials`
File: `Project/JWCEssentials.net/JWCEssentials.net/JWCEssentials.cs`
Purpose: Entry point; exposes `EnableTerminalEffects` and core library initialization.
Collaborates with: Native `libJWCEssentials`.

#### `IOwnedInteropStruct`
File: `Project/JWCEssentials.net/JWCEssentials.net/IOwnedInteropStruct.cs`
Purpose: Ownership interface for managed wrappers around native-allocated structures. Defines the lifetime contract for disposable interop objects.
Notes: Implement this on any managed wrapper that owns a native pointer.

#### `utf8_string_struct`
File: `Project/JWCEssentials.net/JWCEssentials.net/utf8_string_struct.cs`
Purpose: Managed representation of the native `utf8_string_struct`. Handles marshalling between .NET strings and native UTF-8 string buffers.
Collaborates with: P/Invoke call sites across the library.
Notes: Essential for cross-boundary string passing. Handle lifetime carefully.

#### `struct_array_struct`
File: `Project/JWCEssentials.net/JWCEssentials.net/struct_array_struct.cs`
Purpose: Managed representation of `struct_array_struct<T>` — a typed native array with element count.
Notes: Used for RNG state serialization and other bulk data transfer.

### Hashing (JWCEssentials.net)

#### `HasherClass`
File: `Project/JWCEssentials.net/JWCEssentials.net/HasherClass.cs`
Purpose: Managed P/Invoke wrapper for the native HasherClass hierarchy. Exposes hash computation, endian/swap configuration, and value retrieval.
Collaborates with: Native `HasherFactory_Get`, `HasherClass_*` exports.
Notes: Must be disposed to trigger `HasherClass_delete`.

### Random (JWCEssentials.net)

#### `RandomGenerator`
File: `Project/JWCEssentials.net/JWCEssentials.net/RandomGenerator.cs`
Purpose: Managed P/Invoke wrapper for `Random_Generator` / `Random_MT19937`. Exposes seeding, state save/restore, and value generation.
Collaborates with: `Random_MT19937_Create`, `Random_Destroy`, `Random_Generator_*` exports.
Notes: Must be disposed to trigger `Random_Destroy`.

### TLS (JWCEssentials.net)

#### `TLS`
File: `Project/JWCEssentials.net/JWCEssentials.net/TLS.cs`
Purpose: Managed P/Invoke wrapper for thread-local storage slots.
Collaborates with: `TLS_Alloc`, `TLS_get`, `TLS_Free` exports.
Notes: Must be disposed to trigger `TLS_Free`.

### AnsiEffectSniffer

#### `AnsiEffectSniffer`
File: `Project/AnsiEffectSniffer/AnsiEffectSniffer/AnsiEffectSniffer.cs`
Purpose: Parses and detects ANSI escape sequences in text streams. Standalone library — does not depend on native JWCEssentials.

## NewAge Reference Management (CSProj Tooling)

JWCEssentials provides a suite of `.csproj` manipulation tools built on the `CSProj` shared library (`VSXml` namespace). These tools are staged to `$NewAge/bin/` and are available to any project in the workspace.

### The MyReferencePath Convention

Managed libraries in the NewAge ecosystem stage their built DLLs to:
```
$NewAge/DotNet/Libs/lib/$(Configuration)/$(TargetFramework)/
```
This path is exposed as the MSBuild property `MyReferencePath`. Consuming projects set this property and use it in `<Reference>` `Include` attributes, so all managed library references resolve through a single workspace-relative path regardless of machine layout.

### Wiring a Project to Consume NewAge References

The recommended one-step command:
```bash
csproj_add_my_references MyProject/MyProject.csproj Assembly1 [Assembly2 ...]
```
This:
1. Backs up the `.csproj` with a timestamp.
2. Injects `<MyReferencePath>$(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)</MyReferencePath>` if not already present.
3. Adds `<Reference Include="$(MyReferencePath)\AssemblyN.dll" />` for each named assembly.

Example:
```bash
csproj_add_my_references src/MyApp/MyApp.csproj JWCEssentials.net JWCCommandSpawn
```
Use `-u` to update existing entries.

### Making a Library Consumable

A managed library becomes consumable via `csproj_add_my_references` by adding `MyReferencePath` and a `NewAge_stage.sh` PostBuild to its `.csproj`:
```xml
<PropertyGroup>
  <MyReferencePath>$(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)</MyReferencePath>
</PropertyGroup>
<Target Name="PostBuild" AfterTargets="PostBuildEvent">
  <Exec Command="bash NewAge_stage.sh '$(MSBuildProjectName)' '$(MyReferencePath)' '$(OutputPath)/'" />
</Target>
```
`NewAge_stage.sh` symlinks the DLL, PDB, and deps.json into `MyReferencePath` after each build.

### CSProj Tool Reference

| Tool | Usage | Purpose |
|---|---|---|
| `csproj_add_my_references` | `[-u] file.csproj Asm1 [Asm2 ...]` | **Primary entry point.** Ensures `MyReferencePath` and adds named assembly references in one pass. |
| `csproj_add_reference` | `[-u] file.csproj xml_reference` | Adds a single raw XML `<Reference>` element. |
| `csproj_add_property` | `[-u] file.csproj xml_property` | Adds a single raw XML property to the second `<PropertyGroup>`. |
| `csproj_list_references` | `file.csproj` | Lists all `<Reference>` and staged `<ProjectReference>` assembly names. |

Source: `Project/CSProj/` — `CSProj` library (`VSXml.CSProj`) is also staged as a DLL to `$NewAge/DotNet/Libs/lib/` for future managed consumers.

### CSProj Class Map

#### `CSProj` (`VSXml`)
File: `Project/CSProj/CSProj/CSProj.cs`
Purpose: Core XML manipulation class for `.csproj` files. Handles opening, saving, backup, property management, and reference management.
Key methods: `Open`, `EnsureProperty`, `AddMyReference`, `BackupWithTimestamp`, `ListReferences`, `GetPropertyValue`, `GetReferences`.
Notes: Staged to `$NewAge/DotNet/Libs/lib/` via `NewAge_stage.sh`. Extend here when new csproj manipulation operations are needed.

## Standalone Utilities

| Tool | Source | Purpose |
|---|---|---|
| `feffect` | `src/Utils/feffect.cpp` | CLI tool: look up and emit ANSI terminal effect codes by name. |
| `rand_identifier` | `src/Utils/rand_identifier.cpp` | CLI tool: generate random C-style identifiers. |
| `split_arg` | `src/Utils/split_arg.cpp` | CLI tool: split argument strings. |
| `spawn_bash_probe` | `Tools/spawn_bash_probe/` | Probes Bash spawn behavior; used for cross-platform shell reliability testing. |

## Bash Workspace Tooling
All scripts in `Bash/` are copied to `$NewAge/bin/` by `configure.sh`. Edit sources in `Bash/`, not in `$NewAge/bin/`.

### Workspace Bootstrap
| Script | Purpose |
|---|---|
| `configure.sh` | Establishes the $NewAge workspace layout, registers includes/headers, copies Bash scripts to bin, installs `in_this_context.sh`. Central entry point for workspace setup. |
| `Bash/newage_all_configure.sh` | Runs `configure.sh` for every repo in `$NewAge/NewAgeRepo.lst`. |
| `Bash/in_this_context_Package.sh.src` | Source template for `$NewAge/in_this_context.sh` — the workspace context wrapper installed at the $NewAge root. |

### Build Scripts
| Script | Purpose |
|---|---|
| `Bash/newage_all_build_native.sh` | Builds native targets across all workspace repos. |
| `Bash/newage_all_build_managed.sh` | Builds managed (.NET) targets across all workspace repos. |
| `Bash/newage_all_build_coordinated.sh` | Coordinated (native + managed sequenced) build across all repos. |
| `Bash/newage_build_native.sh` | Builds native target for a single repo. |
| `Bash/newage_build_managed.sh` | Builds managed target for a single repo. |
| `Bash/newage_build_coordinated.sh` | Coordinated build for a single repo. |
| `Dev/build_native.sh` | Repo-local native build helper (not installed to bin). |
| `Dev/build_managed.sh` | Repo-local managed build helper (not installed to bin). |

### Repo / Dependency Management
| Script | Purpose |
|---|---|
| `Bash/newage_get.sh` | Fetches/clones a repo into the workspace. |
| `Bash/newage_get_deps.sh` | Fetches all declared dependencies for a repo. |
| `Bash/newage_repo_list.sh` | Lists repos from `NewAgeRepo.lst`. |
| `Bash/newage_dep_sort.sh` | Topologically sorts repo dependencies for build ordering. |
| `Bash/newage_named_path.sh` | Resolves a named path within the workspace. |
| `Bash/newage_futures.sh` | Manages forward-looking repo declarations. |

### Git Utilities
| Script | Purpose |
|---|---|
| `Bash/branchinfo.sh` | Displays current branch and status summary. |
| `Bash/git_identity_relay.sh` | Relays git identity configuration. |
| `Bash/git_snapshot_worktree.sh` | Snapshots current worktree state. |
| `Bash/worktree_dirty.sh` | Exits non-zero if worktree has uncommitted changes. |
| `Bash/if_worktree_dirty.sh` | Conditionally runs a command if worktree is dirty. |

### Context / Environment
| Script | Purpose |
|---|---|
| `Bash/in_dir.sh` | Runs a command in a specified directory, then returns. |
| `Bash/newage_run_in_context.sh` | Runs a command inside a specific workspace context. |
| `Bash/newage_go.sh` | Navigates to a named workspace location. |
| `Bash/NewAge_forward.sh` | PostBuild staging for executable tools. For .NET tools (DLL present), emits a bash wrapper and a `.bat` wrapper both using a script-relative path to the DLL (`dotnet "$SCRIPT_DIR/../Repo/.../Project.dll"`), making `$NewAge/bin/` self-contained and portable across collections and platforms (Linux, Windows CMD/PS, Git Bash). For native binaries, emits a bash wrapper (absolute path) and a `.bat` for `.exe` builds. |
| `Bash/NewAge_stage.sh` | PostBuild staging for managed libraries. Symlinks DLL, PDB, and deps.json into `$(MyReferencePath)` (`$NewAge/DotNet/Libs/lib/`). |
| `Bash/verbose.sh` | Controls verbose output mode. |
| `Bash/colors.sh` | ANSI color helpers for shell scripts. |

### Utility
| Script | Purpose |
|---|---|
| `Bash/SearchFile.sh` | Searches for a file by name pattern within the workspace. |
| `Bash/resolve_path.sh` | Resolves a path (handles symlinks, relative paths). |
| `Bash/grep_nonempty.sh` | Greps and filters out empty matches. |
| `Bash/bash_header.sh` | Standard header sourced by all JWCEssentials Bash scripts. |
| `Bash/bash_new_script.sh` | Scaffolds a new Bash script from template. |
| `Bash/time_stamp.sh` | Emits a formatted timestamp. |
| `Bash/zip_file_list.sh` | Zips a list of files. |
| `Bash/create_symlink.sh` | Platform-aware symlink/junction creation. |
| `Bash/cygpath.sh.linux` | Linux shim for cygpath (installed as `$NewAge/bin/cygpath` on Linux). |

### Remote / Shuttle
| Script | Purpose |
|---|---|
| `Bash/on_remote.sh` | Executes a command on a configured remote. |
| `Bash/remote_info.sh` | Displays remote configuration info. |
| `Bash/remotes.sh` | Lists configured remotes. |
| `Bash/remotes_info.sh` | Detailed remote info. |
| `Bash/shuttle_to.sh` | Shuttles files or repos to a remote workspace. |

## Architectural Seams
- **Bash source → $NewAge/bin**: Scripts are copied, not linked. `Bash/` is the authoritative source; `$NewAge/bin/` is an output. Re-run `configure.sh` (or `newage_all_configure.sh`) after editing.
- **Include symlink**: `include/JWCEssentials/` is registered as a symlink at `$NewAge/include/JWCEssentials`. The repo is the source of truth; the workspace symlink is a registration.
- **Native/Managed boundary**: Defined by `_EXPORT_` in C++ headers; consumed by P/Invoke wrappers in `Project/JWCEssentials.net/JWCEssentials.net/`. The native DLL/SO must be discoverable at managed runtime.
- **Platform abstraction**: `src/Platform_Linux/` and `src/Platform_Windows/` provide platform implementations for shared interfaces. Do not assume platform neutrality within these paths.
- **in_this_context.sh**: The workspace context wrapper is installed from `Bash/in_this_context_Package.sh.src`. It is the standard entry point for entering a lane-specific environment.
- **Dev/ scripts**: `Dev/NewAge.dev.sh`, `Dev/build_native.sh`, `Dev/build_managed.sh` are repo-local development helpers, not installed to `$NewAge/bin`. They are sourced internally by configure and build scripts.

## Extension Points
- **New native subsystem**: Add headers to `include/JWCEssentials/`, implementation to `src/`, and `_EXPORT_` declarations using the existing single-line convention.
- **New managed wrapper**: Add a `.cs` file to `Project/JWCEssentials.net/JWCEssentials.net/` following the `IOwnedInteropStruct` ownership pattern.
- **New Bash tool**: Add to `Bash/` and re-run `configure.sh`. New scripts are automatically picked up for all repos that run `newage_all_configure.sh`.
- **New standalone utility**: Add a `.cpp` to `src/Utils/` and update `CMakeLists.txt`.
- **New csproj operation**: Add a method to `Project/CSProj/CSProj/CSProj.cs` (`VSXml.CSProj`) and expose it via a new or existing tool in `Project/CSProj/`.
- **Making a new managed library consumable**: Add `MyReferencePath` property and `NewAge_stage.sh` PostBuild to the library's `.csproj`. See the NewAge Reference Management section above.

## Known Cautions
- **Single-line _EXPORT_**: Required for grep-based tooling. Do not split declarations across lines.
- **Bash copies, not links**: Editing `$NewAge/bin/<script>` directly loses the change on the next configure run. Always edit `Bash/<script>`.
- **Native library discovery**: On Linux, `LD_LIBRARY_PATH` must include the staged native lib lane, or `ldconfig` must be configured. The `in_this_context.sh` wrapper handles this for development sessions.
- **TLS destructor lifetime**: The destructor passed to `TLS_Alloc` must remain valid for the lifetime of the TLS slot.
- **utf8_string_struct lifetime**: Native-backed string structs must not outlive the native buffer they reference. Managed wrappers must handle marshalling explicitly.
- **configure.sh is idempotent**: Safe to re-run. Existing symlinks and copies are replaced cleanly.

## Generated / Reflected / Derived Files
- **`$NewAge/in_this_context.sh`**: Generated from `Bash/in_this_context_Package.sh.src` by `configure.sh`. Do not edit the installed copy.
- **`$NewAge/bin/<scripts>`**: Copied from `Bash/` by `configure.sh`. Do not edit the installed copies.
- **`$NewAge/include/JWCEssentials`**: Symlink registered by `configure.sh` to `include/JWCEssentials/`. Do not modify through the workspace symlink.
- **`obj/` and `bin/` build artifacts**: Standard .NET build outputs. Not tracked in version control.

## Maintenance Notes
- **Refresh policy**: Update this document after adding native entry points, new Bash tools, new managed classes, or changing workspace layout conventions.
- **Path consistency**: Always use repo-relative paths.
- **_EXPORT_ grep-friendliness**: Keep all `_EXPORT_` entries single-line.
- **configure.sh is the contract**: Any change to what gets installed, registered, or linked must be reflected in `configure.sh` and documented here under Architectural Seams or Extension Points.
