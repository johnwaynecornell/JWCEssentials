# Fluent Design Methodology

## Purpose

`FluentCommandLine` is most useful when it is treated not merely as a command-line parser, but as a small typed language-construction framework.

A fluent command environment is built from ordinary .NET types and static methods. Method return types define the kinds of values the language can produce; parameter types define the kinds of values a method can consume. `FluentCommandLine` registers those methods by return type and recursively resolves parameters through the registry associated with the expected CLR type.

This document describes a methodology for designing applications on top of that mechanism. It is intentionally about **design practice**, not about requiring every consumer to share the same application-level base classes.

The central principle is:

> **FluentCommandLine supplies the composition mechanism; each application supplies its own vocabulary and execution identity.**

That distinction keeps the framework small while allowing applications to build expressive, strongly typed command languages.

---

## 1. Think in Types Before Tokens

A FluentCommandLine grammar should begin as a type design.

Do not start by asking only:

> What command words do I want?

First ask:

> What semantic state does each stage of the command represent?

For example, a clipboard utility might eventually expose commands such as:

```text
ClipFlow show
ClipFlow available
ClipFlow paste image to file image.png
ClipFlow paste files to directory .
ClipFlow copy image from file input.png
ClipFlow copy html from string "<h1>Hello World!</h1>"
```

The important structure is not merely the words `paste`, `image`, `to`, and `file`. Each word can represent a transition to a more specific semantic type:

```text
ClipFlowCommand
    -> PasteSelection
        -> ImagePaste
            -> ClipFlowCommand
```

or:

```text
ClipFlowCommand
    -> CopySelection
        -> HtmlCopy
            -> ClipFlowCommand
```

The CLR type system becomes part of the parser grammar.

### Design rule

Create a distinct fluent type when the user has crossed a meaningful semantic boundary.

Do not create a type merely to represent every token. A good fluent type represents a state in which a particular set of next operations is valid.

---

## 2. Return Types Are Grammar

In the current implementation, `FluentEnvironment.AddSource(...)` ensures a registry for the method's **return type**, and the method is registered into that return type's `FluentMethodRegistry`.

That has an important design consequence:

> **A method's return type determines the vocabulary in which the method participates.**

This is one of the strongest organizing ideas in FluentCommandLine.

Suppose an application defines:

```csharp
public abstract class ClipFlowCommand
{
    public abstract void Execute(ClipFlowContext context);
}
```

A root command should return exactly `ClipFlowCommand` when its result is intended to belong to the root command vocabulary:

```csharp
[FluentMethod("show")]
public static ClipFlowCommand Show()
    => new ClipFlowDelegateCommand(ctx => { /* ... */ });
```

A selector method should return the selector type whose vocabulary should become available next:

```csharp
[FluentMethod("paste")]
public static PasteSelection Paste()
    => new();
```

The next method can consume or produce another semantic type as the grammar narrows.

### Root-return convention

When a command completes construction of an executable root command, return the application's root command type explicitly.

This convention is not just aesthetic. Because FluentCommandLine registries are keyed by CLR type, returning a different derived type creates or targets a different registry.

Use that fact deliberately.

---

## 3. Prefer Domain Identity Over Universal Command Base Classes

A recurring application pattern is:

```text
DomainContext
DomainCommand
DomainDelegateCommand
```

JWCFarm currently demonstrates it as:

```text
FarmContext
FarmCommand
FarmDelegateCommand
```

This pattern should usually be **repeated by consumers rather than generalized into FluentCommandLine itself**.

For example, ClipFlow can define:

```csharp
public sealed class ClipFlowContext
{
    public TextWriter Output { get; set; } = Console.Out;
    public TextWriter ErrorOutput { get; set; } = Console.Error;
}

public abstract class ClipFlowCommand
{
    public abstract void Execute(ClipFlowContext context);
}

public sealed class ClipFlowDelegateCommand : ClipFlowCommand
{
    private readonly Action<ClipFlowContext> action;

    public ClipFlowDelegateCommand(Action<ClipFlowContext> action)
        => this.action = action;

    public override void Execute(ClipFlowContext context)
        => action(context);
}
```

This small amount of duplication is intentional.

### Why not `FluentCommand<TContext>`?

A generic framework base class could reduce the number of repeated lines, but it would also introduce a new abstraction that every application must understand. It would make the context type part of a framework inheritance hierarchy even though the context is fundamentally an application concern.

The local three-type pattern has useful properties:

- The command's execution context is statically obvious.
- Farm commands cannot accidentally become ClipFlow commands.
- Each domain can evolve its context independently.
- A reader does not need to understand a generic command hierarchy before understanding the application.
- Consumers that do not need a delegate-backed command do not inherit one merely because the framework provides it.

### Methodology rule

> **Duplicate small domain vocabulary when the duplication preserves strong semantic identity. Abstract shared mechanism only when the abstraction removes meaningful complexity.**

The command/context/delegate-command trio is a design pattern enabled by FluentCommandLine, not a required FluentCommandLine type hierarchy.

---

## 4. Keep the Root Vocabulary Deliberate

`FluentEnvironment.ServeTypes` defines the types whose registries are considered when parsing a top-level command.

This makes root exposure explicit.

A healthy environment should keep its root vocabulary intentional rather than registering every reachable type as a root type.

For example:

```csharp
var env = new FluentEnvironment();
env.AddModule<ClipFlowModule>();
env.ServeTypes = new[] { typeof(ClipFlowCommand) };
```

Additional served types are valid when the application genuinely has multiple top-level vocabularies, but each one increases the possibility of command-name overlap and reader ambiguity.

### Root discipline

- Prefer one clearly named domain root type for a focused executable.
- Add more root types only when they represent genuine peer command families.
- Treat root-name collisions as design feedback.
- Do not solve an overcrowded root environment by weakening type identity.

FluentCommandLine can support broad environments, but an application designer is responsible for keeping them understandable.

---

## 5. Use Intermediate Types as Semantic Gates

Intermediate fluent types are one of the best ways to prevent invalid command forms.

If `paste image` can only sensibly be followed by image destinations, represent that state with a type whose registry contains only those destinations.

Conceptually:

```csharp
public sealed class PasteSelection;
public sealed class PasteImageSelection;
public sealed class PasteFilesSelection;
```

Then methods can express grammar transitions:

```csharp
[FluentMethod("paste")]
public static PasteSelection Paste() => new();

[FluentMethod("image")]
public static PasteImageSelection Image(PasteSelection selection) => new();

[FluentMethod("files")]
public static PasteFilesSelection Files(PasteSelection selection) => new();
```

The exact signatures should fit the application's desired grammar, but the principle remains:

> **If two states permit meaningfully different next operations, consider giving them different CLR types.**

This is preferable to one large command method containing switches and runtime validation for combinations that the type grammar could have made impossible.

---

## 6. Let Parameters Describe Composition

`FluentEnvironment.TryParseParameter(...)` first checks whether the expected parameter type has a registered `FluentMethodRegistry`. If it does, parsing recursively delegates to that registry.

This means a parameter can itself be a fluent expression.

For example:

```csharp
[FluentMethod("window")]
public static ReportCommand Window(WindowSelector selector)
    => ...;
```

If `WindowSelector` has registered fluent methods such as `by_total`, the command line can naturally compose them.

This is the core compositional pattern:

```text
outer-method inner-method inner-arguments
```

which corresponds to ordinary function composition:

```text
Outer(Inner(arguments))
```

### Design rule

When an argument has its own meaningful construction vocabulary, make it a registered fluent type instead of parsing it manually as a string.

This improves:

- discoverability,
- type safety,
- help generation,
- reuse,
- testability,
- and readability of the command language.

---

## 7. Use Plain Values for Plain Values

Not every concept deserves a fluent type.

FluentCommandLine already supports direct parsing paths for strings and types with suitable parsing behavior, custom `TypeParseHandlers`, registered sub-strategies, and arrays.

Use simple CLR parameters when the semantic value is already naturally represented by a CLR value:

```csharp
string path
int count
TimeSpan duration
DateTime timestamp
SomeEnum mode
```

Create a new fluent type when construction itself has meaningful vocabulary.

A useful distinction is:

- **Value**: `100`, `image.png`, `00:05:00`
- **Choice/state**: `image`, `files`, `by_total`, `from file`

Do not turn every value into a strategy merely to make the syntax appear more fluent.

---

## 8. Keep Parsing Mechanics Separate From Application Execution

A fluent method may produce an object that represents work rather than performing all work immediately.

The Farm command pattern is a good example: a fluent method can construct a `FarmCommand`, and the application can execute that command later with a `FarmContext`.

The same pattern is useful for other applications:

```text
parse -> construct command -> execute with context
```

instead of:

```text
parse -> perform irreversible side effect immediately
```

Delayed execution provides several benefits:

- Parsing can be tested without carrying out external effects.
- Commands can capture validated arguments before execution.
- Output/error services can be supplied through a domain context.
- Applications can add logging, dry-run behavior, transaction boundaries, or lifecycle handling around execution.

### Recommendation

For commands that mutate files, clipboard state, network state, or other external resources, strongly consider returning an executable domain command rather than performing the side effect inside the fluent factory method itself.

Pure selector/construction methods do not need this indirection.

---

## 9. Keep Runtime Services in the Right Context

FluentCommandLine provides two useful context concepts with different jobs.

### `FluentEnvironment.Context`

`FluentContextData` is a type-indexed ambient service/configuration bag associated with the parsing environment.

It is appropriate for services needed while fluent methods are being discovered or invoked, such as:

- configuration,
- registries,
- shared parser services,
- factories,
- host-level dependencies.

Modules can initialize this state through the optional public static:

```csharp
FluentModuleInitialize(FluentEnvironment env)
```

which `AddModule(...)` invokes after method registration unless initialization is suppressed.

### Domain execution context

A `FarmContext`, `ClipFlowContext`, or similar application-defined type belongs to the command execution model.

It can contain things such as:

- output and error writers,
- clipboard service abstractions,
- filesystem services,
- execution options,
- cancellation or lifecycle state.

Do not collapse these two concepts simply because both are called contexts.

A useful distinction is:

> **Fluent environment context helps build/resolve the command; domain context helps execute the command.**

---

## 10. Use `FluentEnvironment.Current` Sparingly and Intentionally

During parsing and fluent method invocation, `FluentEnvironment` enters a thread-local scope. `FluentEnvironment.Current` therefore exposes the active environment to a fluent method without requiring it as an explicit method parameter.

This is useful when a fluent method needs environment-level services:

```csharp
var service = FluentEnvironment.Current.Context.Get<MyService>();
```

The scope implementation uses a thread-static stack and verifies LIFO disposal, allowing nested environment scopes on a thread.

### Guidance

Use `FluentEnvironment.Current` for genuinely ambient environment concerns. Prefer explicit method parameters for semantic inputs that belong in the command grammar.

If a value should appear in help, affect command composition, or be supplied by the user, it is probably a parameter rather than ambient context.

---

## 11. Modules Should Group Vocabulary, Not Become God Objects

`FluentEnvironment.AddModule<T>()` scans public static methods decorated with `[FluentMethod]` and registers them. It can also invoke `FluentModuleInitialize`.

A module is therefore a convenient unit for grouping fluent vocabulary.

Good reasons to create separate modules include:

- distinct semantic areas,
- optional features,
- platform-specific implementations,
- different sets of commands that hosts may choose to expose.

Avoid making one enormous module simply because registration is easy.

Conversely, do not split every fluent type into a separate module unless that division helps comprehension or configuration.

### Host policy

The host application should decide which modules are loaded.

This keeps a low-level library responsible for **mechanism**, while the executable remains responsible for **policy and exposed surface area**.

---

## 12. Attributes Are Metadata, Not Architecture

`[FluentMethod]` marks methods for registration. Help and default metadata can be supplied through FluentCommandLine metadata and, for help, `DescriptionAttribute`.

These attributes should describe the command surface; they should not become a substitute for good type design.

A method with many attributes but an unclear signature still produces an unclear grammar.

Start with:

1. correct return type,
2. correct parameter types,
3. clear method name,
4. then metadata.

Metadata should make a good grammar easier to discover, not rescue a weak grammar.

---

## 13. Design for `list` From the Beginning

FluentCommandLine's listing support is more valuable when types and methods were designed for discovery.

A user should ideally be able to enter:

```text
list
```

and understand the top-level vocabulary, then inspect argument-type strategies without already knowing the implementation.

That suggests several naming practices:

- Prefer short command words with concrete meaning.
- Make intermediate type descriptions meaningful.
- Supply concise help for non-obvious operations.
- Avoid multiple root commands whose names differ only subtly.
- Prefer explicit semantic words (`image`, `html`, `files`) over modes inferred from file extensions or runtime content when explicitness improves predictability.

A fluent CLI should reward exploration.

---

## 14. Explicit Semantic Types Beat Clever Inference

When a command deals with multiple data representations, explicit syntax can be a feature rather than verbosity.

For a clipboard controller, these forms are strong:

```text
copy image from file input.png
copy html from string "<h1>Hello World!</h1>"
paste files to directory .
```

The explicit words `image`, `html`, and `files` make several things easier:

- the user knows which clipboard representation is requested,
- the parser does not need to infer intent from extensions,
- `available` can report vocabulary that maps directly to commands,
- scripts remain stable when content is ambiguous,
- future representations can be added without changing old inference rules.

### Methodology rule

> **Prefer explicit semantic selection when inference would hide a meaningful choice.**

Fluency is not the same thing as minimal token count.

---

## 15. Composition Should Remain Locally Understandable

FluentCommandLine can express deeply nested typed constructions. That power should be used with restraint.

A command is healthy when a reader can understand each phrase from nearby context:

```text
paste files to directory .
```

A command becomes harder to maintain when understanding an early token requires remembering distant hidden state or global mode changes.

Prefer grammars that compose left-to-right through visible semantic states.

### Warning signs

Reconsider the design when:

- a type has dozens of unrelated strategies,
- a command name changes meaning depending on distant earlier choices,
- most methods accept `object`,
- many methods manually inspect token strings,
- runtime validation repeatedly rejects combinations that types could prohibit,
- root `ServeTypes` grows simply to make internal types callable,
- consumers must know implementation class names to use the CLI.

These are signals that the type grammar has stopped carrying enough meaning.

---

## 16. Duplicate Patterns, Not Bugs

Two FluentCommandLine applications may legitimately contain very similar code:

```text
FarmContext             ClipFlowContext
FarmCommand             ClipFlowCommand
FarmDelegateCommand     ClipFlowDelegateCommand
```

Do not automatically treat this as failed reuse.

The duplication is healthy when:

- each type carries a different domain identity,
- the context types are expected to evolve differently,
- the code is tiny,
- sharing it would require generics, casts, or a vague common abstraction,
- the repeated shape teaches consumers how to structure their own fluent application.

The methodology itself can be the reusable artifact.

This is an important distinction:

> **A framework can standardize a pattern without owning every type that participates in the pattern.**

---

## 17. A Recommended Application Shape

For a substantial FluentCommandLine application, a clean project split is often:

```text
MyTool
    executable host

MyTool.Format
    fluent vocabulary and semantic types

MyTool.Tests
    grammar, binding, and execution tests
```

The `.Format` assembly contains the reusable command language. The executable remains thin and host-specific. Tests can exercise the vocabulary without launching the final executable process.

This shape has several advantages:

- the fluent language can be reused by another host,
- startup logic stays separate from grammar design,
- tests can directly configure `FluentEnvironment`,
- the project becomes a useful example of FluentCommandLine usage,
- the executable can remain intentionally boring.

A generic executable that dynamically loads every possible `.Format` assembly may be technically powerful, but it should not be introduced merely to avoid small executable hosts. User-facing tools benefit from concrete names and clear entry points.

Reuse the hosting mechanism when useful; do not force users to understand plugin architecture to run a simple tool.

---

## 18. Test the Language at Its Semantic Boundaries

Tests should validate more than individual static methods. They should validate the grammar that users actually invoke.

Useful test categories include:

### Registration

- module methods are discovered,
- module initialization runs,
- expected return-type registries exist,
- root types expose the intended strategies.

### Composition

- nested registered parameter types resolve correctly,
- cursor consumption stops at the correct token,
- multiple semantic branches remain distinct.

### Values

- enums and primitive values parse correctly,
- custom type handlers receive the correct token range,
- arrays/variadic arguments stop at `.END.` as expected.

### Context

- `FluentEnvironment.Current` is available during invocation,
- environment context services are visible to modules,
- domain execution commands receive the correct domain context.

### Identity

- root commands return the intended root type,
- intermediate selectors return the intended intermediate type,
- unrelated command families do not leak into each other's registries.

### Execution

- command construction can be tested separately from side effects,
- delegate-backed commands invoke their domain context correctly,
- output/error behavior can be redirected in tests.

A fluent grammar should be treated like an API: changing its accepted command forms deserves regression coverage.

---

## 19. Evolve From Consumer Pressure

FluentCommandLine should remain small enough that real applications can tell it what is missing.

When a consumer appears to need a new framework abstraction, ask:

1. Is this parsing/composition machinery shared by multiple consumers?
2. Or is this simply a useful pattern that each domain can express clearly itself?
3. Does centralizing it reduce complexity for the reader?
4. Does centralizing it weaken domain type identity?
5. Can the requirement be met through a module, a type parse handler, or an application-specific helper instead?

Promote behavior into FluentCommandLine when it is clearly part of fluent-language mechanics.

Leave behavior in the consumer when it expresses domain policy or execution identity.

This prevents the framework from gradually absorbing every successful pattern used by an application.

---

## 20. Current Mechanisms and Methodological Interpretation

The current FluentCommandLine snapshot provides several mechanisms that map naturally onto this methodology:

| Mechanism | Methodological role |
| --- | --- |
| `FluentMethodAttribute` | Marks methods as vocabulary available for registration. |
| Return-type registry | Makes CLR return types act as grammar/vocabulary identities. |
| Typed parameters | Define compositional expectations for subsequent expressions. |
| `ServeTypes` | Defines the top-level vocabularies exposed by an environment. |
| `FluentContextData` | Holds environment-level typed services and configuration. |
| `FluentEnvironment.Current` | Provides scoped ambient access during fluent method invocation. |
| `FluentModuleInitialize` | Lets a module configure services or environment state after registration. |
| `TypeParseHandlers` | Extends parsing for domain value types without redesigning the grammar engine. |
| `list` / registry listing | Makes the type grammar discoverable to users. |
| `.END.` array delimiter | Provides an explicit boundary for variadic parsing. |

The methodology intentionally does **not** require a universal `FluentCommand`, `FluentContext`, or `FluentDelegateCommand` base family. Those remain application design choices.

---

## 21. Compact Design Checklist

Before adding a fluent command, ask:

- What type of thing does this method produce?
- Is that return type the vocabulary I want the parser to enter?
- If it completes an executable command, should it return the exact domain root command type?
- Are its parameters plain values or semantic sub-strategies?
- Would an intermediate type prevent invalid combinations?
- Does this operation belong at root, or only after another semantic choice?
- Is a side effect happening too early during parsing?
- Does the method need environment context, execution context, or an explicit parameter?
- Would adding this module overcrowd the root vocabulary?
- Am I introducing a framework abstraction merely to eliminate a few lines of healthy domain-specific duplication?
- Can a user discover this command naturally through `list`?
- Can the grammar be tested without invoking the full executable?

If those answers are clear, the implementation usually becomes straightforward.

---

## 22. Closing Principle

FluentCommandLine works best when its type system and the application's language design reinforce each other.

The framework should not attempt to own every command concept. Its job is to provide reliable typed registration, recursive composition, context access, parsing, and discovery. The application's job is to define a vocabulary whose types make semantic sense.

A healthy FluentCommandLine ecosystem therefore has two forms of consistency at once:

- **shared mechanics**, supplied by the framework;
- **specific identities**, supplied by each domain.

That balance allows Farm to remain recognizably Farm, ClipFlow to remain recognizably ClipFlow, and future consumers to follow the same methodology without being forced into either application's type hierarchy.

In short:

> **Share the fluent mechanism. Repeat the domain pattern when repetition makes meaning clearer. Let types carry the grammar.**
