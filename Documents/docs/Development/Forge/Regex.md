# Forge Regex (Visera.Forge.Utils.Regex)

Thin wrapper around [RE2](https://github.com/google/re2) that uses Visera’s `FStringView` and `FString` everywhere. All APIs are **static** on `FRegex` (same style as RE2). `FStringView` is converted to `absl::string_view` internally (private).

## API overview

| API | Description |
|-----|--------------|
| **FRegex::FullMatch** | The entire text must match the pattern. |
| **FRegex::PartialMatch** | The pattern may match any substring of the text. |
| **FRegex::Replace** | Replace the first match (supports `\0`, `\1`, … in the replacement). |
| **FRegex::GlobalReplace** | Replace all non-overlapping matches; returns the number of replacements. |
| **FRegex::Extract** | If the pattern matches, write the replacement string (with `\0`, `\1`, …) into an `FString`. |
| **FRegex::QuoteMeta** | Escape regex metacharacters so the pattern matches the literal string. |
| **FRegexPattern** | Hold a compiled pattern for repeated use; use with `FRegex::FullMatch(text, pattern, ...)`. |

## Examples

### 1. Full match and partial match

```cpp
import Visera.Forge.Utils.Regex;

using namespace Visera::Forge;

// Full match: entire text must match the pattern
if (FRegex::FullMatch("hello", "h.*o"))
    ; // true

if (!FRegex::FullMatch("hello", "e"))
    ; // no match

// Partial match: any substring may match
if (FRegex::PartialMatch("hello world", "world"))
    ; // true
```

### 2. Capture groups

```cpp
FString name;
int id;
if (FRegex::FullMatch("ruby:1234", R"((\w+):(\d+))", &name, &id))
{
    // name == "ruby", id == 1234
}

// Capture only the first group
FString first;
if (FRegex::PartialMatch("x*100 + 20", R"((\d+))", &first))
{
    // first == "100"
}
```

### 3. Replace

```cpp
FString s("yabba dabba doo");
FRegex::Replace(&s, "b+", "d");       // first match only → "yada dabba doo"
FRegex::GlobalReplace(&s, "b+", "d"); // all matches      → "yada dada doo"
```

### 4. Extract (rewrite with captures)

```cpp
FString out;
if (FRegex::Extract("key = 42", R"((\w+)\s*=\s*(\d+))", "\\1 is \\2", &out))
{
    // out == "key is 42"
}
```

### 5. Quote meta-characters

```cpp
FString escaped = FRegex::QuoteMeta("1.5-2.0?");
// escaped == "1\\.5\\-2\\.0\\?"
// Using escaped as the pattern matches the literal "1.5-2.0?"
```

### 6. Precompiled pattern (reuse)

```cpp
FRegexPattern re(R"((\w+):(\d+))");
if (!re.Ok()) return;

FString name;
int id;
if (FRegex::FullMatch("ruby:1234", re, &name, &id)) { ... }
if (FRegex::FullMatch("go:1", re, &name, &id))      { ... }
```

## Syntax

- Syntax follows RE2 / Perl-style; see [RE2 Syntax](https://github.com/google/re2/wiki/Syntax).
- Common: `\d` digit, `\w` word character, `\s` whitespace, `\b` word boundary, `(?i)` case-insensitive.
- Not supported: backreferences, lookaround. Submatch capture and `\0`, `\1`, … in replacement are supported.

## Dependencies

- `Visera.Core.Types.String` (FStringView / FString)
- RE2 and Abseil (pulled in via Forge’s `install_re2` / `install_abseil`)
