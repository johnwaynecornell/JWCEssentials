# NewAge Expansions Manual

NewAge expansions are conditional developer capabilities that become available when a repository composition satisfies a declared relation.

Composition enables capability; capability expands into named actions.

The NewAge workspace root is resolved from $NewAge. Expansions must not discover repositories by walking upward from the current directory. This keeps expansion behavior stable when invoked from tools, agents, shells, or nested build directories.

## What are Futures/Expansions?

Futures are optional conditional capabilities that extend the development environment based on which repositories are present in the current NewAge workspace. They allow for cross-repository integration without creating hard dependencies in baseline builds.

## Safety and Principles

1. **Explicit Workspace Root**: All operations are relative to `$NewAge`. No hidden parent searching.
2. **Optionality**: Futures do not alter baseline builds. Normal build scripts remain unaffected.
3. **No Silent Mutation**: Project files are not modified by the expansion process.
4. **Resilience**: A missing `Dev/futures.lst` is not an error. Missing expansion scripts only warn unless a capability was explicitly requested.

## Declaration Syntax

Capabilities are declared in `Dev/futures.lst` using a symbolic relation:

```
RepoA + RepoB |= capability -> prefix
```

This means: If the current NewAge composition contains both `RepoA` and `RepoB`, then the capability `capability` is available, and it maps to expansion scripts starting with `prefix`.

## Script Expansion Convention

When a capability is enabled, the expansion tool looks for optional scripts in the repository root using the following suffixes:

- `prefix_native.sh`: For native build/setup tasks.
- `prefix_managed.sh`: For managed (e.g. .NET) build/setup tasks.

Expansion scripts are repo-local endpoints. They are invoked with the current working directory set to the target repository root.

## Stateful Management (`newage_futures.sh`)

The `newage_futures.sh` tool provides stateful management of futures.

### Enabled Futures
`Dev/enabled_futures.lst` records which futures are active for automatic build participation. This file contains just the capability names, one per line.

### Available vs Enabled
- A future is **available** when its dependency declaration in `Dev/futures.lst` is satisfied by the current repository composition.
- A future is **enabled** when its capability name is present in `Dev/enabled_futures.lst`.
- A future is **blocked** if its dependencies are not met.
- A future is **stale-enabled** if it is enabled but no longer available or declared.

### Build Rule
`newage_futures.sh <repo> build all native` builds **enabled ∩ available** futures only. This is the default behavior when calling global build scripts.

### Explicit Build
A specific available future may be built even if it is not enabled:
```bash
newage_futures.sh JWCEssentials build has_cs native
```
If the future is blocked, the build will fail.

## Example

```
JWCEssentials + JWCCommandSpawn |= has_cs -> Dev/build_has_cs
```

If both `JWCEssentials` and `JWCCommandSpawn` are present in `$NewAge`, the expansion tool will look for:
- `Dev/build_has_cs_native.sh`
- `Dev/build_has_cs_managed.sh`
