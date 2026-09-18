# MethodPack

MethodPack is the group of .NET projects in JWCEssentials for building typed command languages and processing their results. [FluentCommandLine](FluentCommandLine.md) turns registered C# methods and their parameter types into a composable command grammar. This is useful when an argument is itself a meaningful choice or construction, rather than another flag in an option bag. [JWCFarm](JWCFarm.md) provides reusable streaming, metrics, and projection machinery that an application can pair with a FluentCommandLine command surface. JWCFarm does not itself reference the FluentCommandLine project. [Fluent Design Methodology](FluentDesignMethodology.md) explains how to choose the types and vocabulary.

## Start here

1. Read the [FluentCommandLine overview and composition example](FluentCommandLine.md#1-overview).
2. Browse [FluentCommandLine source](FluentCommandLine/) and [MethodTests](MethodTests/) for registration and parsing examples. The [solution](MethodPack.sln) contains FluentCommandLine, JWCFarm, and MethodTests.
3. For a working application, see [ClipFlow](https://github.com/johnwaynecornell/CrystalCatalystLibrary/tree/main/Project/ClipFlowPack), whose [module](https://github.com/johnwaynecornell/CrystalCatalystLibrary/blob/main/Project/ClipFlowPack/ClipFlow.Format/ClipFlow_Fluent.cs) defines typed clipboard commands, or [TruthInTheFlip](https://github.com/johnwaynecornell/TruthInTheFlip) for an analytical consumer.
4. Read the [methodology](FluentDesignMethodology.md) and [JWCFarm architecture](JWCFarm.md) when you want to design your own vocabulary or processing pipeline.

## Build and workspace

The projects target **.NET 10**. The current solution also expects a NewAge managed reference directory and Bash staging tools: FluentCommandLine source uses `JWCEssentials.Metadata`, JWCFarm references a staged `JWCEssentials.net.dll`, and both library projects run `NewAge_stage.sh` after building. A fresh clone does **not** currently have a verified one-command standalone `dotnet test Project/MethodPack/MethodPack.sln` path. NewAge is a workspace and staging convention, not a requirement of the fluent grammar itself; a consumer can arrange its own references and staging, but the checked-in project files do not yet provide that standalone recipe.

For the supported integrated route, from the **JWCEssentials repository root** on a machine with Git, Bash, CMake, a C++17 compiler, and a .NET 10 SDK:

```bash
bash Bash/newage_go.sh "$HOME/NewAge" --build Debug
```

The script creates/configures the workspace at the target path, clones JWCEssentials there if needed, installs workspace tools into `$NewAge/bin`, resolves requested repositories/dependencies, and invokes the coordinated build. It does not require `NewAge` to be set before invocation; `--build Debug` builds the workspace, including MethodPack through JWCEssentials' managed build script. To fetch another repository, place its name before `--build`, for example `CrystalCatalystLibrary`. See [workspace setup](../../docs/NewAgeEnvironment.md) and the [script reference](../../docs/Bash%20commands.md).

Once configured, the solution and tests are at `$HOME/NewAge/JWCEssentials/Project/MethodPack/MethodPack.sln` and `.../MethodTests/`. Run tests inside the configured workspace after the coordinated build:

```bash
cd "$HOME/NewAge/JWCEssentials"
dotnet test Project/MethodPack/MethodTests/MethodTests.csproj --no-restore
```

This test invocation is inferred from the solution and project layout; it has not been executed in this audit environment. If your workspace path differs, substitute the target path given to `newage_go.sh`.
