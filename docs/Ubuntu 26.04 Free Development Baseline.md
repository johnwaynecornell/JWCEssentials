# Ubuntu 26.04 Free Development Baseline

This setup verifies that NewAge-family repositories can be configured and built on Ubuntu 26.04 using free system packages, the Ubuntu package manager, GitHub CLI, CMake, native compilers, and .NET SDKs.

This document is intended to be validated by a fresh Ubuntu dry run, a repository clone test, and at least one native and managed compile pass.

## Relationship to the NewAge Environment

This document describes the Ubuntu toolchain baseline. Shared workspace assumptions such as the `NewAge` environment variable, `$NewAge/include`, staged native artifact lanes, staged managed libraries, and Bash-based staging scripts are described by the NewAge environment contract.

## Core Tools

*   **Git**

    Used to clone NewAge-family repositories.

*   **Bash**

    Used by configure and staging scripts.

*   **g++**

    Provides the GNU C++ compiler lane.

*   **CMake**

    Used to configure and build native projects.

*   **make**

    Useful for generated native build systems and general build support.

*   **clang**

    Optional secondary compiler lane for native compatibility testing.

*   **GitHub CLI**

    Used for authenticated GitHub operations such as pushing changes from repo-test VMs.

*   **.NET SDK 8**

    Used for compatibility and older target lanes.

*   **.NET SDK 10**

    Used for active forward development in NewAge-family repositories.


## Install Native Development Tools

```
sudo apt update

sudo apt install g++ cmake make clang
sudo apt install gh
```

## Install .NET SDKs

Ubuntu 26.04 requires the .NET backports PPA for the dotnet 8.0 SDK version used by some NewAge-family repositories at this time.

```
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository ppa:dotnet/backports
sudo apt update
```

Install your dotnet versions or version

```
sudo apt install -y dotnet-sdk-8.0 dotnet-sdk-10.0
```

Verify installed SDKs:

```
dotnet --list-sdks
dotnet --list-runtimes
```

## GitHub Authentication

Install `gh` in the dev-tools baseline, but authenticate only in disposable repo-test VMs or user-specific working VMs.

```
gh auth login
gh auth status
```

This keeps the reusable **Ubuntu 26.04 - Dev tools** image free of user credentials.

## Install optional development GUI

```
sudo apt update
sudo apt install -y wget gpg apt-transport-https

wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
sudo install -D -o root -g root -m 644 packages.microsoft.gpg /etc/apt/keyrings/packages.microsoft.gpg
rm packages.microsoft.gpg

echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" |
  sudo tee /etc/apt/sources.list.d/vscode.list >/dev/null

sudo apt update
sudo apt install -y code

#Then install the useful extensions without signing in:

code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-dotnettools.csharp
code --install-extension ms-dotnettools.csdevkit
code --install-extension GitHub.vscode-github-actions


```

## NewAge Workspace Quickstart

The following sequence was validated on a fresh Ubuntu 26.04 repo-test VM.

```
cd ~
mkdir NewAge
cd NewAge

git clone https://github.com/johnwaynecornell/JWCEssentials
cd JWCEssentials

#if you need to get a branch do so here
#git checkout scratch/integration-refactor

export NewAge="$HOME/NewAge"
./configure.sh
```

## Persistent Shell Environment

After `configure.sh` prints path advice, I added the NewAge workspace paths to the user shell profile.

Example for `~/.profile`:

```
export NewAge="$HOME/NewAge"

export PATH="$PATH:$NewAge/bin"
export PATH="$PATH:$NewAge/bin/Debug/Linux/x86_64/gcc"
export LD_LIBRARY_PATH="$NewAge/lib/Debug/Linux/x86_64/gcc:${LD_LIBRARY_PATH:-}"
```

After editing `~/.profile`, restart the desktop session or open a fresh login shell before testing builds that depend on the updated environment.

## Linux Runtime Library Path

Native executables and managed projects that load native libraries need the staged native library lane to be visible to the Linux dynamic loader.

For terminal-only testing, `LD_LIBRARY_PATH` is sufficient:

```
export LD_LIBRARY_PATH="$NewAge/lib/Debug/Linux/x86_64/gcc:${LD_LIBRARY_PATH:-}"
```

For IDEs and desktop-launched tools, a system loader configuration is often more reliable:

```
sudo bash -c "echo \"$NewAge/lib/Debug/Linux/x86_64/gcc\" > /etc/ld.so.conf.d/newage.conf"    
sudo ldconfig
```

Verify that the staged library is visible:

```
ldconfig -p | grep JWCEssentials
```

If switching native lanes, such as from `gcc` to `clang`, update `/etc/ld.so.conf.d/newage.conf` and rerun `sudo ldconfig`.

## Native Build Verification

From the JWCEssentials repository root:

```
cd "$NewAge/JWCEssentials"

cmake CMakeLists.txt
cmake --build .
```

A successful native pass should build JWCEssentials and stage native artifacts into the configured native lane.

```
$NewAge/bin/Debug/Linux/x86_64/gcc
$NewAge/lib/Debug/Linux/x86_64/gcc
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
echo "$NewAge"
cat "$NewAge/NewAgeRepo.lst"

git --version
gh --version
bash --version
cmake --version
g++ --version
clang --version
dotnet --list-sdks
dotnet --list-runtimes
```

## VM Baseline Recommendation

Keep the reusable dev-tools VM unauthenticated and use disposable repo-test clones for credentialed GitHub work and repository certification.

```
Ubuntu 26.04
  clean OS base

Ubuntu 26.04 - Dev tools
  full clone
  Git, GitHub CLI, CMake, compilers, .NET SDKs
  no GitHub login

Ubuntu 26.04 - Repo test 1
  disposable clone from Dev tools
  repository clone/configure/build verification
  GitHub authentication only if pushing changes
```

## Certification Notes

This baseline should not be considered complete until it passes a fresh Ubuntu dry run, a repository clone test, and at least one native and managed compile pass.

The initial JWCEssentials Ubuntu 26.04 dry run passed native compilation with GCC and managed compilation with .NET SDK 10 after the NewAge workspace was configured.