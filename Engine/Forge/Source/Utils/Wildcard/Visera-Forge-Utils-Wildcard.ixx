module;
#include <Visera-Forge.hpp>
export module Visera.Forge.Utils.Wildcard;
#define VISERA_MODULE_NAME "Forge.Utils.Wildcard"
import Visera.Core.Types.String;
import Visera.Forge.Utils.Regex;

/**
 * Forge wildcard pattern utilities.
 * Converts wildcard patterns (e.g., "*.slang") to regex patterns and provides matching functions.
 */
export namespace Visera::Forge
{
    /**
     * Convert wildcard pattern to regex pattern.
     * Supports:
     *   - '*' matches any sequence of characters
     *   - '?' matches any single character
     *   - Special regex characters are automatically escaped
     * 
     * @param I_Pattern Wildcard pattern (e.g., "*.slang", "test?.txt")
     * @return Regex pattern string
     */
    [[nodiscard]] inline FString
    WildcardToRegex(FStringView I_Pattern)
    {
        FString Result;
        Result.Reserve(I_Pattern.GetSize() * 2);

        for (const char Ch : I_Pattern)
        {
            switch (Ch)
            {
            case '*':
                Result.Append(".*");
                break;
            case '?':
                Result.Append(".");
                break;
            case '.':
            case '+':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '|':
            case '\\':
                Result.Append("\\");
                Result.Append(Ch);
                break;
            default:
                Result.Append(Ch);
                break;
            }
        }
        return Result;
    }

    /**
     * Check if a string matches a wildcard pattern.
     * 
     * @param I_Text Text to match
     * @param I_Pattern Wildcard pattern (e.g., "*.slang")
     * @return True if text matches the pattern
     */
    [[nodiscard]] inline Bool
    WildcardMatch(FStringView I_Text, FStringView I_Pattern)
    {
        const FString RegexPattern = WildcardToRegex(I_Pattern);
        return FRegex::FullMatch(I_Text, RegexPattern);
    }
}
