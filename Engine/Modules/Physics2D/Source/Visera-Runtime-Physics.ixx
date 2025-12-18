module;
#include <Visera-Physics2D.hpp>
#include <box2d/box2d.h>
export module Visera.Physics2D;
#define VISERA_MODULE_NAME "Physics2D"
export import Visera.Physics2D.Common;
       import Visera.Physics2D.RigidBody;
       import Visera.Core.Global;
       import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_PHYSICS2D_API FPhysics : public IGlobalSingleton<FPhysics>
    {
    public:
        [[nodiscard]] FRigidBody2D
        CreateRigidBody2D(const FRigidBody2DCreateInfo& I_CreateInfo);

    private:
        b2WorldId World;

    public:
        FPhysics() : IGlobalSingleton("Physics") {}

        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_PHYSICS2D_API TUniquePtr<FPhysics>
    GPhysics = MakeUnique<FPhysics>();

    FRigidBody2D FPhysics::
    CreateRigidBody2D(const FRigidBody2DCreateInfo& I_CreateInfo)
    {
        VISERA_ASSERT(I_CreateInfo.IsValid());
        b2BodyDef CreateInfo = b2DefaultBodyDef();

        FRigidBody2D RigidBody2D {World, CreateInfo};
        if (!RigidBody2D.IsValid())
        { LOG_ERROR("Failed to create the RigitBody2D!"); }

        return RigidBody2D;
    }

    void FPhysics::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Physics.");

        b2WorldDef CreateInfo = b2DefaultWorldDef();
        // CreateInfo.gravity = {};
        // CreateInfo.restitutionThreshold = {};
        // CreateInfo.hitEventThreshold = {};
        // CreateInfo.contactHertz = {};
        // CreateInfo.contactDampingRatio = {};
        // CreateInfo.maxContactPushSpeed = {};
        // CreateInfo.maximumLinearSpeed = {};
        // CreateInfo.frictionCallback = {};
        // CreateInfo.restitutionCallback = {};
        // CreateInfo.enableSleep = {};
        // CreateInfo.enableContinuous = {};
        // CreateInfo.workerCount = {};
        // CreateInfo.enqueueTask = {};
        // CreateInfo.finishTask = {};
        // CreateInfo.userTaskContext = {};
        // CreateInfo.userData = {};
        // CreateInfo.internalValue = {};

        LOG_DEBUG("Creating Box2D World.");
        World = b2CreateWorld(&CreateInfo);
        if (!b2World_IsValid(World))
        { LOG_FATAL("Failed to create Box2D World!"); }

        Status = EStatus::Bootstrapped;
    }

    void FPhysics::
    Terminate()
    {
        LOG_TRACE("Terminating Physics.");

        Status = EStatus::Terminated;
    }
}