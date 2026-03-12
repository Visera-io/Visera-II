module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Null.Device;
#define VISERA_MODULE_NAME "Platform.Null"

export namespace Visera
{
    enum class ENullMouseButton : Int32
    {};

    enum class ENullKeyboardKey : Int32
    {};

    enum class ENullKeyboardAction : UInt8
    {
        Release = 0,
        Press   = 1,
    };

    enum class EKeyboardModifier : UInt8
    {
        None = 0,
    };

    constexpr EKeyboardModifier operator|(EKeyboardModifier I_A, EKeyboardModifier I_B)
    { return EKeyboardModifier::None; }
    constexpr EKeyboardModifier operator&(EKeyboardModifier I_A, EKeyboardModifier I_B)
    { return EKeyboardModifier::None; }

    inline constexpr Int32 NullMouseButtonLast  = 0;
    inline constexpr Int32 NullKeyboardKeyLast  = 0;
}
