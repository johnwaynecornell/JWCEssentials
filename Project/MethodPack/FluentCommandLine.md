# FluentCommandLine Documentation

## 1. Overview

`FluentCommandLine` is a typed, compositional command-line parsing and execution framework for .NET. 

Unlike traditional command-line parsers that rely on flat flags, switches, or option bags (e.g., `-f file.txt --window 100`), `FluentCommandLine` models command-line invocations as **functional expressions and pipeline graphs**. Commands and arguments are resolved as strongly-typed strategies that construct and compose objects, evaluate pipelines, and execute target operations.

### Key Capabilities

- **Compositional Grammar**: Methods return typed objects that seamlessly serve as inputs to downstream strategies.
- **Type-Safe Strategy Registries**: Dedicated registries per target CLR type (`FluentMethodRegistry`), allowing modular registration of commands and sub-strategies.
- **Automatic Sub-Strategy Resolution**: When a strategy parameter expects a registered type, the parser automatically delegates argument parsing to that type's registry.
- **Ambient Context Management**: Scoped execution contexts (`FluentEnvironmentScope`, `FluentEnvironment.Current`) and strongly-typed state bags (`FluentContextData`).
- **Rich Parameter Binding**: Supports primitives, numbers, enums, `TimeSpan`, `DateTimeOffset`, custom converters, default values, and variadic (`params`) arrays.
- **Self-Documenting CLI**: Built-in interactive help, strategy listing (`list`), type exploration (`info`) support, and clear diagnostic error reporting.

---

## 2. Core Architecture

The framework consists of several cooperating components:

```text
+-------------------------------------------------------------+
|                      FluentEnvironment                      |
|                                                             |
|  +---------------------+        +------------------------+  |
|  |  FluentContextData  |        |       ServeTypes       |  |
|  |  (Typed State Bag)  |        |  (Top-Level Commands)  |  |
|  +---------------------+        +------------------------+  |
|                                                             |
|  +-------------------------------------------------------+  |
|  |         Registries: Dictionary<Type, Registry>        |  |
|  |                                                       |  |
|  |   Type A -> FluentMethodRegistry (Strategies for A)   |  |
|  |   Type B -> FluentMethodRegistry (Strategies for B)   |  |
|  +-------------------------------------------------------+  |
+-------------------------------------------------------------+
```

### 2.1 `FluentEnvironment`

The root orchestrator of the CLI engine. It holds:
- **`Registries`**: Mapping of CLR `Type` to `FluentMethodRegistry`.
- **`ServeTypes`**: Array of types that can be invoked directly as top-level commands (e.g., `FarmCommand`, `IAction`).
- **`Context`**: An instance of `FluentContextData` storing ambient runtime services, configuration, and dependencies.
- **Parsing API**: Methods like `ParseOne(...)`, and `ListCapture(...)`.

### 2.2 `FluentEnvironmentScope`

Provides thread-safe access to the active `FluentEnvironment` during strategy invocation via `FluentEnvironment.Current`. It uses a thread-static stack so nested or concurrent evaluations remain isolated.

```csharp
using (env.EnterScope())
{
    // FluentEnvironment.Current is accessible here
}
```

### 2.3 `FluentContextData`

A type-indexed dependency container for storing ambient contextual data:

```csharp
var context = env.Context;

// Storing state
context.Set(new DatabaseConnection("Server=..."));

// Retrieving state
if (context.TryGet<DatabaseConnection>(out var db))
{
    // Use db
}

// Or throwing if absent
var requiredDb = context.Get<DatabaseConnection>();
```

### 2.4 `FluentMethodRegistry`

Manages strategy definitions for creating instances of a specific `RegistryType`.
- Each entry in `Strategies` is a `FluentMethod`.
- Implements recursive parsing (`TryParse`) to resolve command tokens and argument sub-trees.
- Supports default fallback strategies when no explicit strategy name is provided.

### 2.5 `FluentMethod` and `Parameter`

Represents a callable strategy:
- Wraps a static `MethodInfo` or `Delegate`.
- Contains metadata: `Name`, `Help`, `IsDefault`, and a list of `Parameter` descriptors (with names, types, default values, and help text).

---

## 3. Metadata Attributes & Registration

Commands and strategies are defined using standard C# static methods decorated with attributes.

### 3.1 Attributes

| Attribute | Target | Purpose |
| :--- | :--- | :--- |
| `[FluentMethod("name", def: bool)]` | Method | Registers a static method as a named CLI strategy. `def: true` marks it as the default strategy for its return type. |
| `[KV_FA(FluentAttribute.Help, "description")]` or `[Description("...")]` | Method / Parameter | Provides human-readable help documentation. |
| `[KV_FA(FluentAttribute.Def, "value")]` | Parameter | Defines a string fallback default value if the parameter is omitted. |

### 3.2 Defining a Module

A module is a class containing one or more static strategy methods:

```csharp
using System.ComponentModel;
using FluentCommandLine;

public static class MathModule
{
    // Module initialization hook (called automatically by AddModule)
    public static void FluentModuleInitialize(FluentEnvironment env)
    {
        env.Context.Set(new CalculatorConfig { Precision = 4 });
    }

    [FluentMethod("add")]
    [Description("Adds two floating-point numbers.")]
    public static double Add(
        [Description("First operand")] double a,
        [Description("Second operand")] double b)
    {
        return a + b;
    }

    [FluentMethod("scale", def: true)]
    [Description("Multiplies a value by a factor.")]
    public static double Scale(
        [Description("Input value")] double value,
        [KV_FA(FluentAttribute.Def, "1.0")] double factor)
    {
        return value * factor;
    }
}
```

### 3.3 Registering Modules and Serve Types

```csharp
var env = new FluentEnvironment();

// Register the module
env.AddModule<MathModule>();

// Designate double as a top-level executable type
env.ServeTypes = new[] { typeof(double) };
```

---

## 4. Grammar and Composition

`FluentCommandLine` treats command arguments as expressions in a typed grammar.

### 4.1 Composing Strategies

Suppose you have the following types and strategies:

```csharp
public record DataSource(string Path);
public record Filter(string Expression);
public record ProcessPipeline(DataSource Source, Filter Filter);

public static class PipelineModule
{
    [FluentMethod("file")]
    public static DataSource FromFile(string path) => new(path);

    [FluentMethod("where")]
    public static Filter Where(string expression) => new(expression);

    [FluentMethod("pipeline")]
    public static ProcessPipeline Create(DataSource source, Filter filter) 
        => new(source, filter);

    [FluentMethod("run")]
    public static void Run(ProcessPipeline pipeline)
    {
        Console.WriteLine($"Running pipeline for {pipeline.Source.Path} with filter {pipeline.Filter.Expression}");
    }
}
```

When invoking the CLI with:

```text
run pipeline file "data.csv" where "Age > 30"
```

The parser constructs the execution tree:

```text
Run(
    pipeline: Create(
        source: FromFile("data.csv"),
        filter: Where("Age > 30")
    )
)
```

Each token routes to the strategy matching the expected parameter type at that position.

### 4.2 Parameter Parsing Rules

1. **Primitives and Built-in Types**:
   - `string`, `bool`, `byte`, `sbyte`, `short`, `ushort`, `int`, `uint`, `long`, `ulong`, `float`, `double`, `decimal`.
   - `TimeSpan` (e.g., `00:01:30`, `100ms`, `5s`).
   - `DateTimeOffset` / `DateTime`.
   - `Enum` values (matched case-insensitively).
2. **Sub-Strategies (`RegistryType`)**:
   - If a parameter type is registered in `env.Registries`, the parser recursively consumes tokens to construct that object.
3. **Variadic / Params Arguments**:
   - Methods accepting `params string[]` or `params T[]` consume subsequent arguments until end-of-stream or until encountering the special delimiter `.END.`.

---

## 5. Built-in Commands & Help System

`FluentCommandLine` automatically generates syntax guides and interactive inspection commands.

### 5.1 Interactive Commands

- **`list`**: Displays all top-level commands and their parameter signatures.
  ```text
  Available Commands
      add <a> <b> — Adds two floating-point numbers.
      scale <value> [factor] — Multiplies a value by a factor.
  ```
- **`-help` or `help`**: Displays general help and command usage.

### 5.2 Diagnostic Errors

If a command contains syntax or type conversion errors, the parser prints descriptive diagnostics highlighting missing arguments or invalid conversions:

```text
Error: Expected type 'Double' for parameter 'factor' in strategy 'scale', but received 'abc'.
```

---

## 6. Complete Example

```csharp
using System;
using FluentCommandLine;

public class Program
{
    public record FileInput(string Path);
    public record Operation(string Name);

    public static class CliModule
    {
        [FluentMethod("file")]
        public static FileInput LoadFile(string path) => new(path);

        [FluentMethod("op")]
        public static Operation SetOp(string name) => new(name);

        [FluentMethod("process")]
        public static void Process(FileInput input, Operation op)
        {
            Console.WriteLine($"Processing file '{input.Path}' with operation '{op.Name}'");
        }
    }

    public static int Main(string[] args)
    {
        var env = new FluentEnvironment();
        env.AddModule<CliModule>();
        env.ServeTypes = new[] { typeof(void) };

        int cursor = 0;
        var parsed = env.ParseOne(args, ref cursor);
        //Normaly parsed would represent delayed execution and would be invoked here.
        return 0;
    }
}
```

Invocations:
```bash
# Display available commands
dotnet run -- list

# Run the process strategy
dotnet run -- process file "data.json" op "Transform"
```
