module;
#include <Visera-Forge.hpp>
#include <re2/re2.h>
#include <absl/strings/string_view.h>
export module Visera.Forge.Utils.Regex;
#define VISERA_MODULE_NAME "Forge.Utils.Regex"
import Visera.Core.Types.String;

/**
 * Forge regex API: FRegex wraps RE2 by composition; no RE2 in the public interface.
 * All methods are static; pattern is either FStringView (compiled on the fly) or
 * FRegexPattern (precompiled). FStringView is converted to absl::string_view internally.
 *
 * See docs: Development/Forge (GitPage).
 */
export namespace Visera::Forge
{
    class FRegexPattern;

    class FRegex
    {
        friend class FRegexPattern;

    private:
        [[nodiscard]] static absl::string_view ToAbseil(FStringView I_View) noexcept
        {
            const std::string_view n = I_View.GetNative();
            return absl::string_view(n.data(), n.size());
        }

        static const RE2& GetRe(const FRegexPattern& I_Pattern);

    public:
        // ---------------------------------------------------------------------
        // Full match
        // ---------------------------------------------------------------------
        [[nodiscard]] static Bool FullMatch(FStringView I_Text, FStringView I_Pattern)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::FullMatch(ToAbseil(I_Text), re);
        }

        [[nodiscard]] static Bool FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern);

        template<typename T>
        [[nodiscard]] static Bool FullMatch(FStringView I_Text, FStringView I_Pattern, T* I_Out)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::FullMatch(ToAbseil(I_Text), re, I_Out);
        }

        template<typename T>
        [[nodiscard]] static Bool FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out);

        template<typename T, typename... A>
        [[nodiscard]] static Bool FullMatch(FStringView I_Text, FStringView I_Pattern, T* I_Out, A&&... I_Args)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::FullMatch(ToAbseil(I_Text), re, I_Out, std::forward<A>(I_Args)...);
        }

        template<typename T, typename... A>
        [[nodiscard]] static Bool FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out, A&&... I_Args);

        [[nodiscard]] static Bool FullMatch(FStringView I_Text, FStringView I_Pattern, FString* I_Out)
        {
            return FullMatch(I_Text, I_Pattern, &I_Out->GetNative());
        }

        [[nodiscard]] static Bool FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern, FString* I_Out)
        {
            return FullMatch(I_Text, I_Pattern, &I_Out->GetNative());
        }

        // ---------------------------------------------------------------------
        // Partial match
        // ---------------------------------------------------------------------
        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, FStringView I_Pattern)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::PartialMatch(ToAbseil(I_Text), re);
        }

        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern);

        template<typename T>
        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, FStringView I_Pattern, T* I_Out)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::PartialMatch(ToAbseil(I_Text), re, I_Out);
        }

        template<typename T>
        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out);

        template<typename T, typename... A>
        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, FStringView I_Pattern, T* I_Out, A&&... I_Args)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::PartialMatch(ToAbseil(I_Text), re, I_Out, std::forward<A>(I_Args)...);
        }

        template<typename T, typename... A>
        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out, A&&... I_Args);

        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, FStringView I_Pattern, FString* I_Out)
        {
            return PartialMatch(I_Text, I_Pattern, &I_Out->GetNative());
        }

        [[nodiscard]] static Bool PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern, FString* I_Out)
        {
            return PartialMatch(I_Text, I_Pattern, &I_Out->GetNative());
        }

        // ---------------------------------------------------------------------
        // Replace / Extract / QuoteMeta
        // ---------------------------------------------------------------------
        static Bool Replace(FString* I_Str, FStringView I_Pattern, FStringView I_Rewrite)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::Replace(&I_Str->GetNative(), re, ToAbseil(I_Rewrite));
        }

        static Bool Replace(FString* I_Str, const FRegexPattern& I_Pattern, FStringView I_Rewrite);

        [[nodiscard]] static int GlobalReplace(FString* I_Str, FStringView I_Pattern, FStringView I_Rewrite)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() ? RE2::GlobalReplace(&I_Str->GetNative(), re, ToAbseil(I_Rewrite)) : 0;
        }

        [[nodiscard]] static int GlobalReplace(FString* I_Str, const FRegexPattern& I_Pattern, FStringView I_Rewrite);

        [[nodiscard]] static Bool Extract(FStringView I_Text, FStringView I_Pattern, FStringView I_Rewrite, FString* I_Out)
        {
            const RE2 re(ToAbseil(I_Pattern));
            return re.ok() && RE2::Extract(ToAbseil(I_Text), re, ToAbseil(I_Rewrite), &I_Out->GetNative());
        }

        [[nodiscard]] static Bool Extract(FStringView I_Text, const FRegexPattern& I_Pattern, FStringView I_Rewrite, FString* I_Out);

        [[nodiscard]] static FString QuoteMeta(FStringView I_Unquoted)
        {
            return FString(RE2::QuoteMeta(ToAbseil(I_Unquoted)));
        }
    };

    /// Precompiled pattern (holds RE2 internally). Use with FRegex::FullMatch(text, pattern, ...).
    class FRegexPattern
    {
        friend class FRegex;

    public:
        explicit FRegexPattern(FStringView I_Pattern)
            : Re(FRegex::ToAbseil(I_Pattern))
        {
        }

        [[nodiscard]] Bool Ok() const { return Re.ok(); }

    private:
        [[nodiscard]] const RE2& GetRe() const { return Re; }

        RE2 Re;
    };

    // -------------------------------------------------------------------------
    // FRegex implementations that use FRegexPattern (no RE2 in signature)
    // -------------------------------------------------------------------------
    inline const RE2& FRegex::GetRe(const FRegexPattern& I_Pattern)
    {
        return I_Pattern.GetRe();
    }

    inline Bool FRegex::FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern)
    {
        return RE2::FullMatch(ToAbseil(I_Text), GetRe(I_Pattern));
    }

    template<typename T>
    inline Bool FRegex::FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out)
    {
        return RE2::FullMatch(ToAbseil(I_Text), GetRe(I_Pattern), I_Out);
    }

    template<typename T, typename... A>
    inline Bool FRegex::FullMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out, A&&... I_Args)
    {
        return RE2::FullMatch(ToAbseil(I_Text), GetRe(I_Pattern), I_Out, std::forward<A>(I_Args)...);
    }

    inline Bool FRegex::PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern)
    {
        return RE2::PartialMatch(ToAbseil(I_Text), GetRe(I_Pattern));
    }

    template<typename T>
    inline Bool FRegex::PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out)
    {
        return RE2::PartialMatch(ToAbseil(I_Text), GetRe(I_Pattern), I_Out);
    }

    template<typename T, typename... A>
    inline Bool FRegex::PartialMatch(FStringView I_Text, const FRegexPattern& I_Pattern, T* I_Out, A&&... I_Args)
    {
        return RE2::PartialMatch(ToAbseil(I_Text), GetRe(I_Pattern), I_Out, std::forward<A>(I_Args)...);
    }

    inline Bool FRegex::Replace(FString* I_Str, const FRegexPattern& I_Pattern, FStringView I_Rewrite)
    {
        return RE2::Replace(&I_Str->GetNative(), GetRe(I_Pattern), ToAbseil(I_Rewrite));
    }

    inline int FRegex::GlobalReplace(FString* I_Str, const FRegexPattern& I_Pattern, FStringView I_Rewrite)
    {
        return RE2::GlobalReplace(&I_Str->GetNative(), GetRe(I_Pattern), ToAbseil(I_Rewrite));
    }

    inline Bool FRegex::Extract(FStringView I_Text, const FRegexPattern& I_Pattern, FStringView I_Rewrite, FString* I_Out)
    {
        return RE2::Extract(ToAbseil(I_Text), GetRe(I_Pattern), ToAbseil(I_Rewrite), &I_Out->GetNative());
    }
}
