# Visera Coding Standard

This document summarizes the **style and conventions** used in the Visera engine codebase. Following it keeps the project consistent and easier to read and maintain.

---

## 1. Naming

### 1.1 Type prefixes

Use a single-letter prefix plus **PascalCase** for type names:

| Prefix | Use for | Examples |
|--------|---------|----------|
| **F** | Classes and structs | `FGraphics`, `FTexture2D`, `FRHI`, `FTaskScheduler` |
| **I** | Interfaces (abstract, protocol-like types) | `ITaskScheduler`, `IGlobalService` |
| **E** | Enumerations | `EThreadTag`, `ERHIFormat`, `EName` |
| **T** | Template types (type aliases, containers) | `TMap`, `TArray`, `TSharedPtr`, `TUniquePtr` |

Type aliases that are not templates often use **F** (e.g. `FString`, `FStringView`) or no prefix for primitives (`Bool`, `Int32`, `Float`).

### 1.2 Parameters

Use the **I_** prefix for function parameters (input / argument):

```cpp
CreateTexture2D(TSharedRef<FImage> I_Image);
DoEnqueue(TFunction<void()> I_Task, EThreadTag I_ThreadTag);
```

So: `I_` = “in” / input. Use clear names after that (e.g. `I_TextureHandle`, `I_CommandList`).

### 1.3 Members and locals

- **Member variables**: **PascalCase**, no prefix (e.g. `RenderGraph`, `CommandList`, `Textures`, `InFlightFrames`).
- **Local variables**: **PascalCase** for non-trivial names; short names like `i`, `buf` are fine where scope is small.

### 1.4 Macros and constants

- **Macros**: **UPPER_SNAKE_CASE**, with a **VISERA_** (or module) prefix to avoid clashes (e.g. `VISERA_ASSERT`, `VISERA_CORE_API`, `VISERA_MODULE_NAME`).
- **Global / constexpr** in Core: **PascalCase** for booleans used like keywords (e.g. `True`, `False`).

### 1.5 Namespaces

- Root namespace is **`Visera`**.
- Nested namespaces use **PascalCase** (e.g. `Visera::Concepts`).
- No extra indirection unless a submodule is split (e.g. `Visera.Graphics.Texture` as a module name, still under `namespace Visera`).

---

## 2. Files and modules

### 2.1 File names

- **Kebab-case** with hyphens: `Visera-Module-Submodule.ixx` or `Visera-Module-Submodule.hpp`.
- One primary type or cohesive group per file; name matches the main type or module (e.g. `Visera-Graphics-Texture.ixx` for texture types).

### 2.2 C++ modules

- **Module name**: dot-separated, **PascalCase**: `Visera.Graphics`, `Visera.Graphics.Texture`, `Visera.Tasks.Interface`.
- **Module source**: start with `module;`, then global includes, then `export module Visera.Module.Submodule;`.
- Define **`VISERA_MODULE_NAME`** for logging/debug: `#define VISERA_MODULE_NAME "Graphics.Texture"`.
- Prefer **`export import`** for re-exported modules and plain **`import`** for internal use.

### 2.3 Headers

- Use **`#pragma once`** in headers.
- Include order: project headers first (e.g. `#include <Visera-Graphics.hpp>`), then standard or third-party.

---

## 3. Formatting

### 3.1 Indentation

- Use **tabs** for indentation (no spaces for indent).
- Keep line length readable; break long lines rather than one very long line.

### 3.2 Braces

- **K&R / 1TBS**: opening brace on the same line for control flow and short blocks; closing brace on its own line.
- For classes and namespaces, opening brace on the next line is also used; be consistent within a file.

```cpp
if (condition)
{
    doSomething();
}

void FGraphics::
CreateTexture2D(TSharedRef<FImage> I_Image)
{
    // ...
}
```

### 3.3 Function declarations

- **Return type on its own line** is common for readability, especially when the name or parameter list is long:

```cpp
[[nodiscard]] FTextureID
CreateTexture2D(TSharedRef<FImage> I_Image);

void
Submit(const FRHICommandList& I_CommandList);
```

- **`[[nodiscard]]`** is used for functions whose return value should not be ignored (handles, IDs, optional results).

### 3.4 Class layout

Order sections inside a class as follows:

1. **Public API** — at the top (public member functions, public type aliases, etc.).
2. **Private members** — data and private helpers that callers do not need to see first.
3. **Constructors and helpers** — at the bottom (constructors, destructors, and small helper logic), so the “what this type does” stays at the top and “how it’s built” follows.

Use explicit **`public:`** / **`private:`** / **`protected:`** labels and keep each section contiguous.

### 3.5 Declaration vs implementation

- **Prefer separating declaration and implementation**: declare member functions in the class, implement them below the class (or in a separate implementation section) unless the function is very short.
- **Short functions** (e.g. one-liners, simple getters) may be defined inline in the class.
- For non-member functions and out-of-line member definitions, put the **function name** (and parameters) on the line after the return type so the name is easy to spot:

```cpp
void FGraphics::
CreateTexture2D(TSharedRef<FImage> I_Image)
{
    // ...
}
```

### 3.6 Template functions

For **template** functions, keep **`template<...> ReturnType`** on the **first line**, and the **function name and signature** (and body if inline) on the **second line**. Do not put the return type on its own line.

**Preferred** — template and return type together, then function name:

```cpp
template<typename T> T
Round(T I_Number) { return /* ... */; }

template<Concepts::Task TaskType> void
ITaskScheduler::Enqueue(TaskType&& I_Task, EThreadTag I_ThreadTag)
{
    DoEnqueue(TFunction<void()>(std::forward<TaskType>(I_Task)), I_ThreadTag);
}
```

**Avoid** — return type on a separate line:

```cpp
template<typename T>
T
Round(T I_Number) { /* ... */ }
```

### 3.7 Alignment

- Optional: align `=` in grouped type aliases or declarations for readability. Do not align across unrelated lines or if it hurts diffs.

---

## 4. C++ usage

### 4.1 Visera types

Prefer engine type aliases over raw standard library or primitive names where they exist:

- **Primitives**: `Bool`, `Int32`, `UInt32`, `Float`, `UInt8`, etc. (from `Visera-Core.hpp`).
- **Strings**: `FString`, `FStringView`, `FWideString`, etc.
- **Containers / smart pointers**: `TArray`, `TMap`, `TSharedPtr`, `TUniquePtr`, `TSharedRef`, etc.

### 4.2 Assertions and logging

- **`VISERA_ASSERT(expression)`**: runtime checks in debug; no-op in release when `VISERA_RELEASE_MODE` is set.
- **`LOG_DEBUG`**, **`LOG_WARN`**, **`LOG_FATAL`**: use for diagnostics and fatal errors (e.g. with spdlog/fmt-style formatting).
- **`LOG_FATAL`** for unrecoverable errors; **`VISERA_ASSERT`** for “this must never happen” invariants.

### 4.3 Comments

- **Doxygen-style** for public APIs: `/** ... */` with optional `@param`, `@return`, etc.
- **Single-line**: `//` for brief notes.
- **TODOs**: e.g. `// [TODO]: Implement thread tag-based scheduling` or `//[TODO]: Remove`.

---

## 5. JSON and config

### 5.1 Keys

- Use **PascalCase** for JSON keys: `Version`, `Name`, `Shader`, `Surface`, `Textures`, `BaseColor`.
- Be consistent with existing schemas (e.g. `Engine/Schemas/Material.schema.json`, `Agent.schema.json`).

### 5.2 Schemas

- **Schema `$id`**: use the **`visera://schemas/...`** scheme (e.g. `visera://schemas/Material.schema.json`).
- **Schema `title`**: e.g. `"Visera Material (Minimal)"`, `"Visera AI Agent"`.
- Keep **`additionalProperties`** and **`required`** explicit so configs stay validated and predictable.

---

## 6. Summary checklist

When writing or reviewing code, check:

- [ ] Types use **F** / **I** / **E** / **T** prefixes and **PascalCase**.
- [ ] Parameters use **I_** and a clear name.
- [ ] Files and modules use **kebab-case** and **Visera.Module.Submodule**-style names.
- [ ] Indentation is **tabs**; braces follow **K&R / 1TBS**.
- [ ] **Class layout**: public API first, then private members, then constructors/helpers at the bottom.
- [ ] **Declarations vs implementations**: separated except for very short functions; function name on the line after return type for out-of-line definitions.
- [ ] **Template functions**: first line is `template<...> ReturnType`, second line is function name and signature (not return type on its own line).
- [ ] **Visera** type aliases and **VISERA_** macros are used where defined.
- [ ] **`[[nodiscard]]`** is used for important return values.
- [ ] JSON/config keys are **PascalCase** and match the relevant schema.

These rules focus on **style**; for design, architecture, and safety guidelines, see other project docs or team guidance.
