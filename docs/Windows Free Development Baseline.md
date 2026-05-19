# Windows Free Development Baseline

This setup verifies that the NewAge-family repositories can be configured and built on Windows without requiring a commercial IDE.

This document is intended to be validated by a fresh Windows dry run, a repository fork or clone test, and at least one compile pass through the expected native and managed build lanes.

## Relationship to the NewAge Environment

This document describes the Windows toolchain baseline. Shared workspace assumptions such as the `NewAge` environment variable, `NewAge_Lane`, staged library paths, and Bash-based staging scripts are described in `NewAgeEnvironment.md`.

## Native Build Environment

Native Windows builds require a configured C++ toolchain and CMake-compatible build environment. This can be provided by Visual Studio, Visual Studio Build Tools, CLion configured with a Windows toolchain, or a Developer Command Prompt.

The free baseline described here uses Microsoft C++ Build Tools and a Developer Command Prompt because that path is reproducible without requiring a commercial IDE. Users of Visual Studio or CLion may build through their IDE if the same compiler, Windows SDK, and CMake requirements are satisfied.

Separately, NewAge-family .NET projects may invoke Bash-based post-build staging commands such as `bash NewAge_stage.sh`. For those builds, `bash` should be available on the inherited `PATH`, commonly through Git for Windows.

## Developer Prompt and Bash

Native Windows builds that require MSVC should be configured from a **Developer Command Prompt** or another terminal that has loaded the Visual Studio build environment.

NewAge-family .NET projects also expect `bash` to be available for post-build staging commands such as `bash NewAge_stage.sh`. Installing Git for Windows and allowing it to add Git/Bash to the user `PATH` is the recommended baseline path.

These requirements are complementary: the Developer Command Prompt supplies the Microsoft compiler environment, while Git for Windows supplies `bash`. A Developer Command Prompt can still run `bash` if Git for Windows is on `PATH`.

This should remain compatible with commercial IDEs such as Visual Studio, Rider, or CLion. Those IDEs may provide their own compiler environment, but the project post-build steps still need `bash` to be discoverable through the inherited environment.

### Verification

```
where bash
bash --version
where cl
cl
```

## Core Tools

*   **Visual Studio Code**

    Used as the free editor/source browser and as a practical default editor for Git for Windows.

    [Download Visual Studio Code - Mac, Linux, Windows](https://code.visualstudio.com/download)

*   **Git for Windows**

    Provides `git` and `bash`, both of which are useful for NewAge-family repositories.

    [Git for Windows](https://gitforwindows.org/)

*   **.NET SDK 8**

    Used for compatibility and older target lanes.

*   **.NET SDK 10**

    Used for active forward development in the NewAge-family repositories.

*   **CMake**

    Tested with CMake 4.3.2 during Windows certification.

*   **Visual Studio Build Tools**

    Install the Desktop development with C++ workload, including MSVC, MSBuild, CMake tools, and the Windows SDK.

*   **Optional LLVM/Clang**

    Useful as a secondary compiler compatibility check.


## Install .NET for VS Code

VS Code requires external SDKs to be installed on the system before .NET development can begin.

*   **Download:** Install the SDKs from the [official .NET site](https://dotnet.microsoft.com/download/dotnet).
*   **VS Code Extension:** Install the **C# Dev Kit** or the Microsoft **C#** extension from the Extensions marketplace.
*   **Check:** Run `dotnet --list-sdks` in a terminal to verify installed SDKs.

## Install the Windows Native Build Tools

Microsoft Visual Studio Build Tools are commonly used for the Windows native build lane, but users are responsible for complying with Microsoft's current Visual Studio and Build Tools license terms.

Developers who qualify for Visual Studio Community may use that as the free Microsoft toolchain path. Otherwise, verify that your usage of the standalone Build Tools is permitted for your project and organization.

*   **Download:** Install **Visual Studio Build Tools** from Microsoft's official downloads page.

    [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/)

*   **Workload:** Select **Desktop development with C++**.
*   **Components:** Include MSVC, MSBuild, CMake tools for Windows, and the Windows SDK.
*   **VS Code Extension:** Install the Microsoft **C/C++** extension.

If `cl` is not available in a normal terminal, open a **Developer Command Prompt** or load the Visual Studio build environment before configuring native projects.

## Shell Requirement

NewAge-family projects may invoke Bash-based post-build commands such as `bash NewAge_stage.sh`. If `bash` is not available from PowerShell, add Git for Windows' `bin` directory to the user `PATH`.

```
$gitBin = "C:\Program Files\Git\bin"

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")

if ($userPath -notlike "*$gitBin*") {
    [Environment]::SetEnvironmentVariable(
        "Path",
        "$userPath;$gitBin",
        "User"
    )
}

Write-Host "Updated user PATH. Restart PowerShell, Git Bash, VS Code, and IDE terminals before testing."
```

Verify from a new terminal:

After setting environment variables such as `NewAge` and `Path`, restart PowerShell, Git Bash, VS Code, and any build terminals before testing.

## Native Build Verification

From the JWCEssentials repository root:

```
cd "$NewAge/JWCEssentials"

cmake CMakeLists.txt
cmake --build .
```

A successful native pass should build JWCEssentials and stage native artifacts into the configured native lane.

The preferred way to run native artifacts is through the context wrapper:

```bash
./in_this_context.sh Debug -- bin/Debug/Windows/AMD64/msvc/JWCEssentialsTest.exe
```

Staged lane example:

```
$NewAge/bin/Debug/Windows/AMD64/msvc
$NewAge/lib/Debug/Windows/AMD64/msvc
```

## Managed Build Verification

From the JWCEssentials repository root:

```
dotnet build Project/JWCEssentials.net/
```

Run the managed test output:

```
Project/JWCEssentials.net/test/bin/Debug/net10.0/test
```

## Verification Commands

```
git --version
bash --version
dotnet --list-sdks
dotnet --list-runtimes
cmake --version
cl
clang --version
```

## Certification Notes

This baseline should not be considered complete until it passes a fresh Windows dry run, a repository fork or clone test, and at least one managed and native compile pass.