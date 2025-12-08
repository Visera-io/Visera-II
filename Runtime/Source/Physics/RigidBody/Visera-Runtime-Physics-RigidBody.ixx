module;
#include <Visera-Runtime.hpp>
#include <box2d/box2d.h>
export module Visera.Runtime.Physics.RigidBody;
#define VISERA_MODULE_NAME "Runtime.Physics"

namespace Visera
{
    export class VISERA_RUNTIME_API FRigidBody2D
    {
    public:
        [[nodiscard]] VISERA_FORCEINLINE const b2BodyId&
        GetHandle() const { return Handle; }
        [[nodiscard]] VISERA_FORCEINLINE Bool
        IsValid() const { return b2Body_IsValid(Handle); }

    private:
        b2BodyId Handle;

    public:
        FRigidBody2D() = delete;
        FRigidBody2D(const b2WorldId& I_World, const b2BodyDef& I_CreateInfo);
        ~FRigidBody2D();
    };

    FRigidBody2D::
    FRigidBody2D(const b2WorldId& I_World,
                 const b2BodyDef& I_CreateInfo)
    {
        VISERA_ASSERT(b2World_IsValid(I_World));

        Handle = b2CreateBody(I_World, &I_CreateInfo);
    }

    FRigidBody2D::
    ~FRigidBody2D()
    {
        b2DestroyBody(Handle);
    }
}