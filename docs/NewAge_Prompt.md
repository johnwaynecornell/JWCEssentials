# NewAge Agent Context Prompt

This document provides condensed context for an AI agent working inside a NewAge workspace.

It is intended to be copied into a new agent session when the agent needs to understand repository layout, native build lanes, managed reference paths, linking, DllImport runtime discovery, and client development conventions.

---

## NewAge Workspace Context

You are working within a NewAge Workspace.

NewAge is an explicit, inspectable development workspace for cross-platform and multi-lane development. It supports both native C++ projects and managed .NET projects, with deliberate separation between source repositories, native build lanes, managed reference directories, and runtime discovery paths.

Do not treat NewAge as magic. Prefer reading scripts, project files, and purpose/context files when behavior is unclear.

---

## 1. Workspace Root

The environment variable \$NewAge on Unix-like systems, or %NewAge% on Windows, points to the workspace root.

Use this variable as the source of truth.

Do not infer the workspace root by walking parent directories unless a specific script intentionally does so.

Typical layout:

```
$NewAge/
bin/
include/
lib/
DotNet/
\<RepoName\>/
NewAgeRepo.lst
```
Common meanings:

```
$NewAge/bin/       Common scripts and installed tools.
$NewAge/include/   Workspace header/include rendezvous.
$NewAge/lib/       Workspace native library rendezvous.
$NewAge/DotNet/    Managed/.NET workspace area.
$NewAge/\<Repo\>/    Source repository checkout.
```

---

## 2. Purpose.NewAge

NewAge workspace folders are preferably given the extension .NewAge. So the form is \<Purpose\>.NewAge as in `Home.NewAge`

---

## 3. Native Build Lanes

Native build artifacts are segregated by lane to avoid toolchain and configuration collisions.

A native lane usually includes:

```
Config / Platform / Arch / Toolchain
```

Example:

```
Debug/Linux/x86_64/clang
```

A native lane may produce artifacts under paths similar to:

```
\NewAge/lib/Debug/Linux/x86_64/clang/
\NewAge/bin/Debug/Linux/x86_64/clang/
```

Do not hardcode one native lane unless the user explicitly asks for it. Prefer NewAge helper scripts and environment-aware build commands.

---

## 4. Native Linking Model

Native NewAge clients usually need three things:

headers
native libraries
runtime loader discovery

Headers are typically made available through:

```$NewAge/include/```

Native libraries are typically made available through:

```
$NewAge/lib/\<Config\>/\<Platform\>/\<Arch\>/\<Toolchain\>/
```

Client CMake projects commonly include:

```
include("$ENV{NewAge}/include/JWCEssentials/JWCEssentials.cmake")
```

and configure native output/discovery with:

newage_configure_native_paths(TargetName)

Native linking is still explicit. A client must link the libraries it uses.

Example shape:

```
find_library(LIBJWCEssentials JWCEssentials)
target_link_libraries(MyTarget PUBLIC \${LIBJWCEssentials})
```

If a native executable fails at runtime because a library cannot be found, check the active lane and runtime loader path.

On Linux, this usually involves LD_LIBRARY_PATH.

On Windows, this usually involves PATH.

Prefer NewAge lane/context scripts over manual path edits when available.

---

## 5. Managed / .NET Reference Directories

Managed projects are separate from native linking.

NewAge managed assemblies may be built or copied into a managed reference/output area such as:

```
\NewAge/DotNet/Libs/lib/Debug/net10.0
```

General shape:

```
\NewAge/DotNet/Libs/lib/\<Config\>/\<TargetFramework\>/
```

Examples:

```
\NewAge/DotNet/Libs/lib/Debug/net10.0
\NewAge/DotNet/Libs/lib/Release/net10.0
```

This is the managed assembly rendezvous. Managed clients may use this location through MyReferencePath or equivalent project/reference configuration.

A managed client may need to reference assemblies such as:

```
JWCEssentials.net.dll
JWCCommandSpawn.net.dll
CrystalCatalystLibrary.net.dll
```

Those references are managed compile-time references. They are not the same as native runtime library discovery.

---

## 6. MyReferencePath

MyReferencePath is the managed reference path convention used by NewAge managed clients.

Use it to help .NET projects find NewAge-managed assemblies.

Conceptual shape:

```
MyReferencePath=\$NewAge/DotNet/Libs/lib/Debug/net10.0
```
A managed project can then reference NewAge assemblies from that directory.

Important distinction:

MyReferencePath solves managed assembly/reference discovery.
`PATH` or `LD_LIBRARY_PATH` solves native DllImport runtime discovery.

A .NET project can build successfully through MyReferencePath and still fail at runtime if its native DllImport dependencies are not discoverable.

---

## 7. Managed Clients

When developing a managed client project:

1. Confirm \$NewAge or %NewAge% is set.
2. Confirm MyReferencePath points to the intended managed reference directory.
3. Confirm required managed assemblies are built into the expected config/framework folder.
4. Confirm native libraries required by DllImport/PInvoke are built.
5. Confirm the runtime loader can find those native libraries.
6. Run dotnet build.
7. Run a smoke test, because successful build does not prove native runtime discovery.

Useful mental model:

Managed compile/reference discovery:
```$NewAge/DotNet/Libs/lib/Debug/net10.0```
MyReferencePath

Native runtime discovery:
```$NewAge/lib/Debug/Linux/x86_64/clang```
PATH / LD_LIBRARY_PATH

These are related but not interchangeable.

---

## 8. Managed Projects With Native Dependencies

A managed NewAge project that P/Invokes a native NewAge library has two layers of dependency:

managed assembly reference
native library runtime discovery

Example:

JWCCommandSpawn.net
managed side:
referenced by .NET clients through MyReferencePath or project reference

native side:
DllImport("JWCCommandSpawn")
requires native JWCCommandSpawn library discoverable at runtime

transitive native side:
JWCCommandSpawn may also require JWCEssentials

So a managed client may need:

JWCEssentials.net.dll or JWCCommandSpawn.net.dll available through MyReferencePath
native JWCEssentials and JWCCommandSpawn libraries available through PATH or LD_LIBRARY_PATH

Do not assume that managed references imply native runtime availability.

---

## 9. Common Commands

Common scripts may be available from \$NewAge/bin.

Examples:

```
newage_go.sh
newage_get.sh <repo>
newage_build_native.sh <repo|path>
newage_build_managed.sh <repo|path>
newage_futures.sh <repo> <cmd>
in_this_context.sh <config> <lane> -- <cmd>
```
Before assuming exact arguments, inspect the script or run its help behavior if available.

NewAge favors inspectable scripts over hidden behavior.

---

## 10. Context Execution

Use in_this_context.sh when a command must run inside a specific NewAge lane/context.

Example shape:

in_this_context.sh Debug Linux -- newage_build_native.sh SomeRepo

The exact lane arguments may vary by local convention. Prefer existing examples in the repo or workspace scripts.

A validated mixed-shell shape may look like:

Windows Developer Prompt
-\> bash
-\> in_this_context.sh
-\> cmd.exe
-\> bash -c newage_build_native.sh

This kind of chain is valid when the active context, PATH, managed references, native lanes, and shell escapement all agree.

---

## 11. Futures

Some repositories support optional capabilities called futures.

They are commonly declared in:

Dev/futures.lst

and enabled through:

Dev/enabled_futures.lst

A future should not be assumed active merely because it exists. Check whether it is enabled.

Use newage_futures.sh when available.

---

## 12. Agent Behavior Rules

When acting as an AI development agent in NewAge:

- Prefer explicit paths using \$NewAge.
- Do not invent repository layout.
- Do not hardcode one native build lane unless instructed.
- Keep native and managed dependency models separate.
- Use MyReferencePath or project references for managed assembly discovery.
- Use PATH or LD_LIBRARY_PATH for native runtime discovery.
- Preserve DllImport/export names unless an export mismatch proves they are wrong.
- Check list_exports.sh or equivalent export inspection tools when changing native/managed seams.
- For new repos, look for or create Purpose.NewAge rather than assuming JWCEssentials is the conceptual entry point.
- If behavior is unclear, inspect \$NewAge/bin, Dev/, CMake files, .csproj files, Purpose.NewAge, and repository context files.

---

## 13. Quick Native Client Checklist

[ ] \$NewAge is set.
[ ] Headers are discoverable.
[ ] CMake includes the NewAge/JWCEssentials helper if appropriate.
[ ] Target links required native libraries.
[ ] Native libraries are built in the active lane.
[ ] Runtime loader path can find the lane libraries.
[ ] Native smoke test runs.

---

## 14. Quick Managed Client Checklist

```
[ ] \$NewAge is set.
[ ] MyReferencePath is set if the project expects it.
[ ] MyReferencePath points to the intended config/framework folder.
[ ] Example: \$NewAge/DotNet/Libs/lib/Debug/net10.0.
[ ] Required managed assemblies are built there or referenced by project.
[ ] Required native libraries are built.
[ ] PATH or LD_LIBRARY_PATH can find native DllImport dependencies.
[ ] dotnet build succeeds.
[ ] Runtime smoke test succeeds.
```
---

## 15. Summary

NewAge is a workspace coordination system.

Native side:

headers + native libraries + lane + runtime loader path

Managed side:

project/assembly references + MyReferencePath + target framework output directory + native runtime discovery

Managed reference example:

\$NewAge/DotNet/Libs/lib/Debug/net10.0

Repository orientation:

Purpose.NewAge

Use scripts, purpose files, project files, and lane/context helpers as source of truth.
