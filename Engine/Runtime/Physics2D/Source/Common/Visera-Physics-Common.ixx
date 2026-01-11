module;
#include <Visera-Physics2D.hpp>
#include <box2d/box2d.h>
export module Visera.Physics2D.Common;
#define VISERA_MODULE_NAME "Physics2D.Common"

export namespace Visera
{
    enum class ERigidBodyType
    {
        Undefined,

        Static,
        Dynamic,
    };

    struct VISERA_PHYSICS2D_API FRigidBody2DCreateInfo
    {
        ERigidBodyType Type = ERigidBodyType::Undefined;

        [[nodiscard]] VISERA_FORCEINLINE Bool
        IsValid() const
        {
            return Type != ERigidBodyType::Undefined;
        }
    };
}