# Bash Commands

> NewAge Bash commands provide the orchestration layer for workspace management, repository acquisition, and cross-repo builds.

## Overview

The NewAge substrate uses a set of standardized Bash scripts to maintain a coherent workspace across multiple repositories. These scripts are typically installed to `$NewAge/bin` during the `JWCEssentials` configuration.

## Core Workspace Commands

### newage_go.sh

The primary entry point for bootstrapping or updating a NewAge workspace.
This file is indempotent, meaning it can be run multiple times without causing issues. And does not require NewAge to be run.


```bash
newage_go.sh <target-dir> [RepoName...] [--build [build args...]]
```

*   **Behavior**: Creates the target directory, clones `JWCEssentials`, runs `configure.sh`, and then acquires any specified repositories and their dependencies.
*   **Update mode**: If run against an existing workspace, it ensures all repositories are up to date and re-configures the environment.
*   **Build Integration**: The `--build` flag triggers a coordinated build of the entire workspace after acquisition.

## Agent and Session Support

### NewAge Agent Prompt

For working with AI agents (like Junie), a condensed context prompt is available in `docs/NewAge_Prompt.md`. This can be used at the start of a new session to "set the stage" with workspace knowledge.

### newage_get.sh

Acquires or updates a repository within the `$NewAge` workspace.

```bash
newage_get.sh RepoName GitUrl [Branch]
newage_get.sh 'RepoName|GitUrl[|Branch]'
```

*   **Behavior**: Clones the repository if it doesn't exist, or fetches and pulls if it does.
*   **Default Branch**: `main`.

### newage_dep_sort.sh

Produces a topological sort of repositories based on their dependencies.

```bash
newage_dep_sort.sh [REPO_LIST_FILE]
```

*   **Input**: Defaults to `$NewAge/NewAgeRepo.lst`.
*   **Logic**: Reads `Dev/NewAgeDeps.lst` from each repository to determine order.

### newage_get_deps.sh

Automatically acquires all dependencies for the current workspace.

```bash
newage_get_deps.sh [REPO_LIST_FILE]
```

*   **Logic**: Uses `newage_dep_sort.sh` to find dependencies and `newage_get.sh` to acquire them.

### newage_repo_list.sh

Lists all repository relative paths (names) in the workspace in dependency order.

```bash
newage_repo_list.sh [REPO_LIST_FILE]
```

*   **Usage**: Can be used to filter `newage_dep_sort.sh` output or run independently.
  *   **Example**: `newage_repo_list.sh | xargs -I{} in_dir.sh "$NewAge/{}" git pull`

## Orchestration Commands

### newage_all_configure.sh

Runs the `configure.sh` script for every repository in the workspace in dependency order.

```bash
newage_all_configure.sh [REPO_LIST_FILE]
```

### newage_all_build_native.sh

Executes native builds for all repositories in the workspace.

```bash
newage_all_build_native.sh [BuildArgs...]
```

*   **BuildArgs**: Typically `Debug`, `Release`, or `Both`.
*   **Logic**: Calls `Bash/newage_build_native.sh` front-door in each repo.

### newage_all_build_managed.sh

Executes managed builds for all repositories in the workspace.

```bash
newage_all_build_managed.sh [BuildArgs...]
```

### newage_all_build_coordinated.sh

Executes coordinated (both native and managed) builds for all repositories in the workspace.

```bash
newage_all_build_coordinated.sh [BuildArgs...]
```

## Collection and Staging

### newage_collect.sh

Collects built artifacts from a workspace into a standalone, portable package root.

```bash
newage_collect.sh [Options] SOURCE_NEWAGE PACKAGE_ROOT [Config...]
```

*   **Behavior**: Creates a NewAge-shaped directory structure and populates it with headers, bins, and libs.
*   **Options**:
    *   `--clone`: Dereference symlinks to create a self-contained package (replaces symlinks with real copies).
    *   `--repo-includes`: Also collect the `include` directory from each repository.
    *   `--bash`, `--script`, `-bash`, `-script`: Populate the package `bin` directory with scripts from `$SOURCE_NEWAGE/bin` (excluding `newage_` and `dev_` prefixed ones).
    *   `--no-include`, `--no-native`, `--no-managed`, `--no-dotnet-libs`: Skip specific collection surfaces.

### NewAge_stage.sh

A project-level helper typically called during post-build steps to stage managed artifacts into the workspace.

## Context Management

### newage_run_in_context.sh

Executes a command within a specific NewAge lane environment.

```bash
newage_run_in_context.sh [--newage PATH] [CONFIG] [TOOLCHAIN] -- COMMAND [ARG...]
```

*   **Logic**: Sources development helpers and uses `set_lane_environment` to configure the native lane. This sets the `NewAge_Config` and `NewAge_Lane` environment variables and updates `PATH`, `LD_LIBRARY_PATH`, `CC`, and `CXX` before execution.
*   **Availability**: Installed to `$NewAge/bin` and typically available on the shell `PATH`.

### in_this_context.sh

A workspace-root convenience script that allows executing commands within a specific lane environment.

```bash
./in_this_context.sh [CONFIG] [TOOLCHAIN] -- COMMAND [ARG...]
```

*   **Logic**: A self-contained version of the context wrapper installed to the root of the `$NewAge` workspace. It is functionally similar to `newage_run_in_context.sh` but designed to be independent of the `JWCEssentials` repository structure.

## Repository Front Doors

Each repository is expected to provide its own front-door scripts in its `Bash/` directory:

*   `newage_build_native.sh`
*   `newage_build_managed.sh`
*   `newage_build_coordinated.sh`

These scripts handle repository-specific build logic while respecting the global `$NewAge` environment.

### Internal Delegation (The "Official Source")

The front-door scripts are designed to delegate to the "official source" for build logic within each repository, typically located in `Dev/`:

*   `Dev/build_native.sh`
*   `Dev/build_managed.sh`
*   `Dev/build_coordinated.sh`

This pattern ensures that build logic is maintained in a single place (the `Dev/` scripts) while the `Bash/` front-doors provide the standardized interface for the NewAge orchestration layer.
