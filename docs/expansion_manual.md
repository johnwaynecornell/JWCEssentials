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

## Execution Modes

- **Dry-run (`--dry-run`)**: The default mode. Analyzes the composition, identifies enabled capabilities, and prints the scripts that *would* be executed. No actions are taken.
- **Apply (`--apply`)**: Executes the discovered scripts that exist on disk.

## Example

```
JWCEssentials + JWCCommandSpawn |= has_cs -> Dev/build_has_cs
```

If both `JWCEssentials` and `JWCCommandSpawn` are present in `$NewAge`, the expansion tool will look for:
- `Dev/build_has_cs_native.sh`
- `Dev/build_has_cs_managed.sh`
