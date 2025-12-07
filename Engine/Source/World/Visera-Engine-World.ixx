module;
#include <Visera-Engine.hpp>
#include <entt/entt.hpp>
export module Visera.Engine.World;
#define VISERA_MODULE_NAME "Engine.World"
       import Visera.Core.Log;
       import Visera.Core.Global;
       import Visera.Core.Types.Name;
       import Visera.Core.Types.Map;
       import Visera.Core.Math.Random.Seed;
       import Visera.Engine.World.Entity;
export import Visera.Engine.World.Component;
export import Visera.Engine.World.System;

namespace Visera
{
    export class VISERA_ENGINE_API FWorld : public IGlobalSingleton<FWorld>
    {
    public:
        void inline
        Tick(Float I_DeltaTime);

        // Managed by Registry rather than SharedPtr
        VISERA_NOINLINE FEntity
        CreateEntity(FName I_Name);

    private:
        UInt32                  Seed {0};

        entt::registry          GlobalRegistry{};
        TMap<FName, FEntity>    Entities{};
        SMovement               MovementSystem;
        SPhysics2D              Physics2DSystem;

    public:
        FWorld() : IGlobalSingleton("World") {}
        void Bootstrap() override;
        void Terminate() override;
    };

    export inline VISERA_ENGINE_API TUniquePtr<FWorld>
    GWorld = MakeUnique<FWorld>();

    void FWorld::
    Tick(Float I_DeltaTime)
    {
        MovementSystem.Tick(GlobalRegistry, I_DeltaTime);
        Physics2DSystem.Tick();
    }

    FEntity FWorld::
    CreateEntity(FName I_Name)
    {
        FEntity Entity { GlobalRegistry, GlobalRegistry.create() };
        LOG_DEBUG("Entity created (name:{}, id:{}).",
                  I_Name, static_cast<ENTT_ID_TYPE>(Entity.GetID()));

        Entities.emplace(I_Name, Entity);
        return Entity;
    }

    void FWorld::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping World");

        Seed = FSeedPool{}.Get();
        LOG_INFO("World Seed: {}.", Seed);

        Status = EStatus::Bootstrapped;
    }

    void FWorld::
    Terminate()
    {
        LOG_DEBUG("Terminating World");

        Status = EStatus::Terminated;
    }
}
