JWCEssentials
=============

JWCEssentials is a foundational utility library intended to support platform-agnostic modular software design.

The repository contains both native C/C++ components and .NET bindings/utilities. It is designed to be usable as a public dependency while also serving as part of a larger private(for now!) architecture.

In the broader platform layout, higher-level projects may clone or consume JWCEssentials as a foundational module. The repository is therefore intentionally structured around portable build outputs, explicit staging, and reusable shell tooling.

Repository Layout
-----------------

    JWCEssentials/
    ├── Bash/                 # Shared shell tooling used by builds and staging
    ├── include/              # Public C/C++ headers
    ├── lib/                  # Staged native library outputs
    ├── Project/
    │   └── JWCEssentials.net # .NET project and solution files
    ├── src/                  # Native C/C++ source
    ├── CMakeLists.txt        # Native C/C++ build entry point
    └── Docket.md             # Development docket / work tracking


Design Goals
------------

JWCEssentials is intended to support:

*   Platform-agnostic native utilities
*   Modular reuse across repositories
*   Predictable build output locations
*   Native and managed interop scenarios
*   Reusable shell-based staging commands
*   Forward-compatible .NET targeting

The project is deliberately split between native C/C++ code, shared public headers, Bash tooling, and .NET project files.

Native Build
------------

The native component is built using CMake.

### Requirements

*   CMake 3.28 or newer
*   A C++17-capable compiler
*   Bash-compatible shell tooling
*   Platform support currently differentiated for Linux and Windows

### Configure and Build for a NewAge Workspace

JWCEssentials defines the foundational NewAge workspace layout. Running `configure.sh` creates the required workspace directories, exposes JWCEssentials headers through `$NewAge/include`, exposes Bash helper tools through `$NewAge/bin`, and registers the current checkout under the shared workspace.

```
export NewAge="$HOME/NewAge"
./configure.sh
```

On Windows Git Bash:

```
./configure.sh --newage "/c/src/NewAge"
```

From the repository root:

    cmake -S . -B cmake-build-debug
    cmake --build cmake-build-debug


The native build creates the `JWCEssentials` shared library and utility executables such as:

*   `feffect`
*   `rand_identifier`
*   `split_arg`
*   `test`

Build outputs may be staged using scripts from the `Bash/` directory.

Bash Tooling
------------

The `Bash/` directory contains reusable shell scripts used by the build and staging process.

Examples include:

*   `NewAge_stage.sh`
*   `shuttle_to.sh`
*   `verbose.sh`
*   `resolve_path.sh`
*   `create_symlink.sh`
*   `time_stamp.sh`

These scripts are part of the build assumptions for this repository and for repositories that consume it.

If a build command invokes one of these scripts directly, ensure that the script is available on the current working path or that the build environment calls it from the correct directory.

.NET Project
------------

The .NET project is located at:

    Project/JWCEssentials.net/JWCEssentials.net/


The project currently targets:

    <TargetFrameworks>net8.0;net10.0</TargetFrameworks>


This ensures that consumers build against the intended target framework rather than relying on implicit or default framework selection.

### .NET Requirements

*   .NET SDK supporting `net8.0`
*   .NET SDK supporting `net10.0` when building that target
*   Bash available to MSBuild post-build steps
*   The `NewAge` environment variable set when using the default staging layout

Environment Variables
---------------------

### `NewAge`

JWCEssentials expects an environment variable named `NewAge` in some build paths.

This variable should point to the root of the broader NewAge workspace layout.

Example on Linux or macOS:

    export NewAge=/path/to/NewAge


Example on Windows PowerShell:

    $env:NewAge = "C:\Path\To\NewAge"


The .NET project uses this variable to construct `MyReferencePath`.

`MyReferencePath`
-----------------

The .NET project defines:

    <MyReferencePath>$(NewAge)\DotNet\Libs\lib\$(Configuration)\$(TargetFramework)</MyReferencePath>


This property is used as a conventional reference/staging path across related repositories.

The path varies by:

*   Build configuration, such as `Debug` or `Release`
*   Target framework, such as `net8.0` or `net10.0`

For example, a Debug build targeting `net8.0` may stage or reference assemblies from a path similar to:

    $NewAge/DotNet/Libs/lib/Debug/net8.0


Depending on platform and shell behavior, path separator handling may vary. Consumers should ensure their environment resolves the path correctly.

.NET Post-Build Staging
-----------------------

The .NET project includes a post-build target that calls a Bash staging script:

    <Target Name="PostBuild" AfterTargets="PostBuildEvent">
        <Exec Command="bash NewAge_stage.sh '$(MSBuildProjectName)' '$(MyReferencePath)' '$(OutputPath)/'" />
    </Target>


This stages the built project output using:

1.  The MSBuild project name
2.  The configured `MyReferencePath`
3.  The project output path

This means the .NET build assumes that:

*   `bash` is available
*   `NewAge_stage.sh` is resolvable from the build context
*   `NewAge` is defined if `MyReferencePath` is used unchanged

If building outside the larger NewAge workspace, you may need to adjust the script path or provide the expected environment variables.

Building the .NET Project
-------------------------

From the .NET solution directory:

    cd Project/JWCEssentials.net
    dotnet build


To build a specific framework:

    dotnet build -f net8.0


or:

    dotnet build -f net10.0


If the post-build step fails, verify:

*   Bash is installed
*   `NewAge_stage.sh` is available to the command
*   `NewAge` is set
*   The destination path exists or can be created
*   The current shell can resolve path separators correctly

Platform Notes
--------------

JWCEssentials is intended to support multiple platforms. Native build logic currently branches on the CMake system name:

*   Linux
*   Windows

Platform-specific native implementations are organized under directories such as:

    src/Platform_Linux/
    src/Platform_Windows/


This separation allows shared public APIs while keeping system-specific implementation details isolated.

Public Consumption
------------------

As a public repository, JWCEssentials should be cloneable and buildable independently, but some advanced staging behavior assumes the larger workspace layout.

### Standalone Use

1.  Clone the repository.
2.  Build the native project with CMake.
3.  Build the .NET project with `dotnet build`.
4.  If staging fails, configure or disable the post-build staging step for your local environment.

### Use Inside a Larger Workspace

1.  Set the `NewAge` environment variable.
2.  Ensure Bash scripts are available.
3.  Build native outputs.
4.  Build .NET outputs.
5.  Allow post-build staging to place artifacts in the shared library layout.

Development Tracking
--------------------

See `Docket.md` for current implementation status and planned work.

Current areas of work include:

*   `JWCEssentials`
*   `JWCEssentials.net`
*   Error system improvements
*   Tooling utilities

Maintainer Notes
----------------

JWCEssentials is intended to remain a small, foundational, platform-conscious module. Changes should preserve:

*   Clear native/managed separation
*   Explicit platform-specific implementation boundaries
*   Stable public headers
*   Predictable output staging
*   Compatibility with consuming repositories

License
-------

MIT License

Copyright (c) 2026 John W Cornell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.