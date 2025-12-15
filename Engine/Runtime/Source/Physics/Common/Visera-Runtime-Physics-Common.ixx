module;
#include <Visera-Runtime.hpp>
#include <box2d/box2d.h>
export module Visera.Runtime.Physics.Common;
#define VISERA_MODULE_NAME "Runtime.Physics"

export namespace Visera
{
    enum class ERigidBodyType
    {
        Undefined,

        Static,
        Dynamic,
    };

    struct VISERA_RUNTIME_API FRigidBody2DCreateInfo
    {
        ERigidBodyType Type = ERigidBodyType::Undefined;

        [[nodiscard]] VISERA_FORCEINLINE Bool
        IsValid() const
        {
            return Type != ERigidBodyType::Undefined;
        }
    };
}