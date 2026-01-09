module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Handle;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    namespace Concepts
    {
        template<class H> concept
        Handle = requires(H h, UInt32 Generation, UInt32 Index)
        {
            { h.GetIndex()            } -> std::convertible_to<UInt32>;
            { h.GetGeneration()       } -> std::convertible_to<UInt32>;
            { h.IsNull()              } -> std::convertible_to<Bool>;
            { H{Generation, Index} } -> std::same_as<H>;
            requires H{}.IsNull(); // Default Value is Null
        };
    }

    class VISERA_CORE_API FHandle
    {
    public:
        [[nodiscard]] constexpr Bool
        IsNull() const { return Value == 0ULL; }
        [[nodiscard]] constexpr UInt32
        GetIndex() const { return static_cast<UInt32>(Value & 0xFFFFFFFFU); }
        [[nodiscard]] constexpr UInt32
        GetGeneration() const { return static_cast<UInt32>(Value >> 32); }
        [[nodiscard]] constexpr UInt64
        GetValue() const { return Value; }

    protected:
        UInt64 Value = 0ULL;

    public:
        constexpr FHandle() = default;
        constexpr FHandle(UInt32 I_Generation, UInt32 I_Index)
        : Value { (static_cast<UInt64>(I_Generation) << 32) | I_Index } {}

        friend constexpr bool operator==(FHandle I_A, FHandle I_B) { return I_A.Value == I_B.Value; }
        friend constexpr bool operator!=(FHandle I_A, FHandle I_B) { return I_A.Value != I_B.Value; }
    };
    static_assert(Concepts::Handle<FHandle>);
}
VISERA_MAKE_HASH(Visera::FHandle, return I_Object.GetValue(););
VISERA_MAKE_FORMATTER(Visera::FHandle, {}, "<Gen:{},Idx:{}>", I_Formatee.GetGeneration(), I_Formatee.GetIndex());