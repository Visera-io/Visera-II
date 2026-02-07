module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Char;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Optional;

export namespace Visera
{
    class FChar;
    template<> inline constexpr Bool HasIntrusiveUnsetOptionalState<FChar> = True;

    class VISERA_CORE_API FChar
    {
    public:
        FChar() = default;
        constexpr FChar(char I_Value) noexcept : Value{I_Value} {}

        [[nodiscard]] constexpr Bool IsEmpty() const noexcept { return Value == '\0'; }
        [[nodiscard]] constexpr operator char() const noexcept { return Value; }

        FChar(FIntrusiveUnsetOptionalState) noexcept : Value{'\0'} {}
        VISERA_CORE_API
        friend Bool operator==(const FChar& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;

    private:
        char Value{'\0'};
    };
    inline Bool operator==(const FChar& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.Value == '\0'; }
    static_assert(sizeof(TOptional<FChar>) == sizeof(FChar));
}