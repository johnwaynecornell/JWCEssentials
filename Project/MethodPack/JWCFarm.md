# JWCFarm Documentation

## 1. Overview

`JWCFarm` is a high-performance, modular data processing and metrics computation framework for .NET.

It is designed for stream processing, batch calculations, time-series analysis, and analytical data projection. The framework decouples data enumeration, pipeline transformation, statistical metric binding, and evaluation into clear, composable abstractions.

### Key Features

- **Streaming Pipeline Architecture**: Process arbitrary data streams with structured lifecycle hooks (`Begin`, `Process`, `End`, `Abort`).
- **Declarative Metric Declarations**: Expose properties and calculation methods as metrics using attributes (`[Metric]`, `[MetricHelp]`).
- **Dynamic Metric Expression Engine**: Bind and evaluate query expressions containing property paths, scalar math, collection aggregations, and multi-variable statistics (e.g. `sum#X`, `mean#X`, `pearson#X,Y`).
- **Separation of Plan and State**: Compile reusable, immutable query plans (`MetricProjection`) that execute across isolated, thread-safe evaluation sessions (`MetricEvaluationSession`).
- **Hidden Metric Dependencies**: Declare dependent calculations (`SourceExpressions`) that are automatically resolved and accumulated without cluttering output columns.
- **Rich Standard Function Catalog**: Out-of-the-box support for scalar arithmetic, statistical moments, covariance, correlation, and signal processing metrics.

---

## 2. Pipeline Architecture

`JWCFarm` organizes operations around a stream-oriented pipeline.

```text
+-------------------------------------------------------------------------+
|                               FarmProcess                               |
|                                                                         |
|  StatType:  Type of output/record inspected for metrics                 |
|  InputType: Type of raw incoming stream elements                        |
|                                                                         |
|  1. EnumerateItems(context)  ---> Yields stream items                   |
|  2. ProcessItem(item)        ---> Accumulates metric samples & actions  |
|  3. Projection.CreateSession() -> Evaluates bound query expressions     |
+-------------------------------------------------------------------------+
```

### 2.1 Core Types

| Type | Purpose |
| :--- | :--- |
| `FarmCommand` | Base executable abstraction for Farm operations (`Execute(FarmContext context)`). |
| `FarmDelegateCommand` | Lightweight command wrapping an `Action<FarmContext>`. |
| `FarmContext` | Execution context providing standard I/O streams (`Output`, `ErrorOutput`). |
| `FarmProcess` | Core pipeline process that streams data, manages metric binding (`BindFields`), and drives execution. |
| `ProcessActions` | Lifecycle callbacks for processes (`Begin`, `Process`, `End`, `Abort`). |

### 2.2 Process Lifecycle

When `FarmProcess.Execute(context)` is called:

1. **Initialization**: Creates a `MetricEvaluationSession` from the configured `MetricProjection` (if set).
2. **`BeginProcess(context)`**: Invokes `Actions.Begin` to initialize resources.
3. **Stream Enumeration**: Calls `EnumerateItems(context)` and passes each item to `ProcessItem(context, item)`.
4. **`EndProcess(context)`**: Invokes `Actions.End` to finalize calculations and flush output.
5. **Exception Handling**: If an exception occurs, `AbortProcess(context, ex)` is invoked before re-throwing.

---

## 3. The Metrics Subsystem

The metrics engine enables dynamic extraction and calculation of values from data items without hardcoding output formats.

### 3.1 Metadata Attributes

Metrics are declared on classes, properties, and methods using metadata attributes:

```csharp
using JWCFarm.Metrics;

public class SensorData
{
    [Metric]
    [MetricHelp("Recorded timestamp.")]
    public DateTime Timestamp { get; init; }

    [Metric("temp_c")]
    [MetricHelp("Temperature in Celsius.")]
    public double TemperatureCelsius { get; init; }

    // Calculated metric with source dependency
    [Metric(sourceExpressions: new[] { "temp_c" })]
    [MetricHelp("Temperature in Fahrenheit.")]
    public double TempFahrenheit => (TemperatureCelsius * 9.0 / 5.0) + 32.0;
}
```

### 3.2 Catalogs and Registries

- **`MetricCatalog`**: A dictionary of `MetricDescriptor` instances associated with a specific type.
- **`MetricCatalogs`**: Central registry managing type-to-catalog mappings.

#### Catalog Discovery Helpers

- `MetricCatalogs.DefaultReflect(Type type)`: Reflects all public instance properties and methods marked with `[Metric]` on the type.
- `MetricCatalogs.MetricLoadStaticFromType(Type staticType, MetricCatalog? target)`: Reflects static metric functions from helper classes (e.g., `MathFunctions`, `AggregateFunctions`).

```csharp
var catalogs = new MetricCatalogs();

// Register item metrics via reflection
catalogs.Catalogs[typeof(SensorData)] = MetricCatalogs.DefaultReflect(typeof(SensorData));

// Register static math & aggregate functions
MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions), catalogs.Catalogs[typeof(MetricFunctions)]);
```

---

## 4. Metric Expression Grammar

Metric expressions are parsed by `MetricBinder` into execution paths.

### 4.1 Syntax Rules

1. **Direct Fields**: References a property or metric by name.
   ```text
   Timestamp
   TemperatureCelsius
   ```
2. **Dotted Property Paths**: Traverses nested metric objects.
   ```text
   Begin.Timestamp
   End.TemperatureCelsius
   ```
3. **Function Invocations (`#` and `,`)**:
   Functions use `#` to open parameter lists and `,` to separate arguments:
   ```text
   functionName#arg1,arg2,...
   ```
4. **Scalar Operations**:
   ```text
   add#5.0,10.0
   scale#TemperatureCelsius,1.8
   clamp#TemperatureCelsius,-40.0,85.0
   ```
5. **Aggregate Functions**:
   Aggregates consume a stream collection (`List<double>` or `List<T>`) across the entire sample set:
   ```text
   count#TemperatureCelsius
   sum#TemperatureCelsius
   mean#TemperatureCelsius
   stddev_sample#TemperatureCelsius
   pearson#TemperatureCelsius,Pressure
   ```
6. **Nested / Compound Expressions**:
   ```text
   scale#mean#TemperatureCelsius,1.8
   ratio#sum#X,sum#Y
   ```

### 4.2 Error Diagnostics

When expression binding fails, `MetricBinder.Bind` returns a structured `MetricBindError` pinpointing the exact character offset and diagnostic message:

```csharp
if (!process.BindFields(catalogs, new[] { "add#5.0" }, out var error))
{
    Console.WriteLine(error.FormatDiagnostic());
    // Output:
    // add#5.0
    //    ^ Function 'add' requires 2 arguments, but received 1.
}
```

---

## 5. Execution Model: Projections & Sessions

`JWCFarm` cleanly separates **query plans** from **evaluation state**.

```text
               Bind Phase
[Field Strings] -------> MetricProjection (Immutable Query Plan)
                               |
                               | CreateSession()
                               v
                     MetricEvaluationSession (Evaluation Lifetime)
                               |
                   +-----------+-----------+
                   |                       |
            Inspect(item)           Field.Get(session, item)
         (Accumulate stats)           (Retrieve values)
```

### 5.1 `MetricProjection`

Represents the compiled, reusable query plan:
- **`Fields`**: List of requested output `MetricPath` instances.
- **`Dependencies`**: Hidden dependency paths pre-bound from `SourceExpressions`.
- Safe to reuse across multiple executions or concurrent sessions.

### 5.2 `MetricEvaluationSession`

Owns transient state during a run:
- **Aggregate Samples**: Stores accumulated values in a `ConditionalWeakTable<object, ProductMetricState>`.
- **Custom Session State**: Provides `GetState<T>(key, factory)` for stateful custom metrics across stream items.
- **`Inspect(process, segment, state)`**: Feeds items into all bound output paths and dependent source expressions.

### 5.3 `MetricEvaluationContext`

Passed to custom metric getters and function delegates:
- `ctx.Session`: Active evaluation session.
- `ctx.Stats`: Current receiver/target object.
- `ctx.Get<T>("source_expr")`: Evaluates a pre-bound dependency.
- `ctx.GetState<T>(key, factory)`: Accesses session-scoped custom state.

---

## 6. Built-in Function Libraries (`MetricFunctions`)

`JWCFarm.Metrics` provides comprehensive mathematical and statistical function suites.

### 6.1 `MathFunctions` (Scalar Math)

| Function | Signature | Description |
| :--- | :--- | :--- |
| `abs` | `(sample, value)` | Absolute value of `value`. |
| `negate` | `(sample, value)` | Negates `value` (`-value`). |
| `square` | `(sample, value)` | Squares `value` (`value * value`). |
| `sqrt` | `(sample, value)` | Square root of `value`. |
| `ln` | `(sample, value)` | Natural logarithm of `value`. |
| `add` | `(sample, a, b)` | Addition (`a + b`). |
| `sub` | `(sample, a, b)` | Subtraction (`a - b`). |
| `mul` | `(sample, a, b)` | Multiplication (`a * b`). |
| `div` | `(sample, a, b)` | Division (`a / b`). |
| `root` | `(sample, value, n)` | Calculates nth root ($value^{1/n}$). |
| `pow` | `(sample, value, exp)` | Power function ($value^{exp}$). |
| `log` | `(sample, value, base)` | Logarithm of `value` at specified base. |
| `offset` | `(sample, value, amount)` | Offsets `value` by `amount`. |
| `offset50`| `(sample, value)` | Offsets `value` from 50% baseline (`value - 50.0`). |
| `scale` | `(sample, value, factor)` | Scales `value` by `factor`. |
| `ratio` | `(sample, num, den)` | Calculates ratio (`num / den`). |
| `smaller`| `(sample, a, b)` | Returns `Math.Min(a, b)`. |
| `larger` | `(sample, a, b)` | Returns `Math.Max(a, b)`. |
| `clamp` | `(sample, val, min, max)`| Clamps `val` to `[min, max]`. |
| `lerp` | `(sample, a, b, amount)` | Linear interpolation between `a` and `b`. |

### 6.2 `AggregateFunctions` (Collection Statistics)

| Function | Signature | Description |
| :--- | :--- | :--- |
| `count` | `(sample, values)` | Count of values in sample. |
| `sum` | `(sample, values)` | Sum of sample values. |
| `mean` | `(sample, values)` | Arithmetic mean. |
| `min` | `(sample, values)` | Minimum value. |
| `max` | `(sample, values)` | Maximum value. |
| `median` | `(sample, values)` | Median value. |
| `variance_population` | `(sample, values)` | Population variance ($\sigma^2$). |
| `variance_sample` | `(sample, values)` | Sample variance ($s^2$). |
| `stddev_population` | `(sample, values)` | Population standard deviation ($\sigma$). |
| `stddev_sample` | `(sample, values)` | Sample standard deviation ($s$). |
| `rms` | `(sample, values)` | Root Mean Square. |
| `mean_abs` | `(sample, values)` | Mean of absolute values. |
| `covariance_population`| `(sample, x, y)` | Population covariance between $X$ and $Y$. |
| `covariance_sample` | `(sample, x, y)` | Sample covariance between $X$ and $Y$. |
| `pearson` | `(sample, x, y)` | Pearson correlation coefficient ($r$). |

### 6.3 `StatisticalFunctions` & `MetricFunctions`

- **`StatisticalFunctions`**: Dedicated class exposing the statistical subset (`mean`, variances, standard deviations, RMS, covariances, pearson).
- **`MetricFunctions`**: Unified class aggregating math, aggregates, and helper registration methods.

---

## 7. End-to-End Example

Here is a complete example defining a custom data item, setting up a process, binding metric expressions, and evaluating results:

```csharp
using System;
using System.Collections.Generic;
using JWCFarm;
using JWCFarm.Metrics;

public sealed class TransactionItem
{
    [Metric]
    public double Amount { get; init; }

    [Metric]
    public double Fee { get; init; }
}

public sealed class TransactionProcess : FarmProcess
{
    private readonly List<TransactionItem> _items;

    public TransactionProcess(List<TransactionItem> items)
    {
        _items = items;
    }

    public override Type StatType => typeof(MetricFunctions);
    public override Type InputType => typeof(TransactionItem);

    protected override IEnumerable<object> EnumerateItems(FarmContext context)
        => _items;
}

public class Program
{
    public static void Main()
    {
        // 1. Prepare sample data
        var items = new List<TransactionItem>
        {
            new() { Amount = 100.0, Fee = 2.5 },
            new() { Amount = 250.0, Fee = 5.0 },
            new() { Amount = 400.0, Fee = 8.0 }
        };

        // 2. Set up metric catalogs
        var catalogs = new MetricCatalogs();
        catalogs.Catalogs[typeof(TransactionItem)] = MetricCatalogs.DefaultReflect(typeof(TransactionItem));
        
        var statCat = new MetricCatalog();
        MetricCatalogs.MetricLoadStaticFromType(typeof(MathFunctions), statCat);
        MetricCatalogs.MetricLoadStaticFromType(typeof(AggregateFunctions), statCat);
        catalogs.Catalogs[typeof(MetricFunctions)] = statCat;

        // 3. Create process and bind query fields
        var process = new TransactionProcess(items);
        string[] fields = new[]
        {
            "count#Amount",
            "sum#Amount",
            "mean#Amount",
            "max#Fee",
            "pearson#Amount,Fee"
        };

        if (!process.BindFields(catalogs, fields, out var error))
        {
            Console.WriteLine($"Binding error: {error?.FormatDiagnostic()}");
            return;
        }

        // 4. Evaluate stream
        var session = new MetricEvaluationSession(process.Projection!);
        object dummyState = new();

        foreach (var item in items)
        {
            session.Inspect(process, dummyState, item);
        }

        // 5. Output projected values
        for (int i = 0; i < process.Projection!.Fields.Count; i++)
        {
            var field = process.Projection.Fields[i];
            object value = field.Get(session, dummyState);
            Console.WriteLine($"{fields[i]} = {value}");
        }
    }
}
```

Output:
```text
count#Amount = 3
sum#Amount = 750
mean#Amount = 250
max#Fee = 8
pearson#Amount,Fee = 0.99988...
```
