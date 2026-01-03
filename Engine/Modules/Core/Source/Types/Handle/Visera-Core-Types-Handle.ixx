module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Handle;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    namespace Concepts
    {
        template<class H> concept
        Handle = requires(H h)
        {
            { h.GetIndex()      } -> std::convertible_to<UInt32>;
            { h.GetGeneration() } -> std::convertible_to<UInt32>;
            { h.IsNull()        } -> std::convertible_to<Bool>;
        };
    }

    class VISERA_CORE_API FHandle
    {
    public:
        static FHandle Null;

        [[nodiscard]] constexpr Bool
        IsNull() const { return *this == Null; }
        [[nodiscard]] constexpr UInt32
        GetIndex() const { return Index; }
        [[nodiscard]] constexpr UInt32
        GetGeneration() const { return Generation; }
        [[nodiscard]] constexpr UInt64
        GetValue() const { return Value; }

    private:
        union
        {
            struct { UInt32 Generation, Index; };
            UInt64 Value = ~0ULL;
        };

    public:
        constexpr FHandle() = default;
        constexpr FHandle(UInt32 I_Generation, UInt32 I_Index) : Generation{I_Generation}, Index {I_Index} {}

        friend constexpr bool operator==(FHandle I_A, FHandle I_B) { return I_A.Value == I_B.Value; }
        friend constexpr bool operator!=(FHandle I_A, FHandle I_B) { return I_A.Value != I_B.Value; }
    };
    FHandle FHandle::Null {0, 0};
}
VISERA_MAKE_HASH(Visera::FHandle, return I_Object.GetValue(););
VISERA_MAKE_FORMATTER(Visera::FHandle, {}, "<Gen:{},Idx:{}>", I_Formatee.GetGeneration(), I_Formatee.GetIndex());