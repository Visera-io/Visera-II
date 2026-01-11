module;
#include <Visera-Game.hpp>
#include <entt/entt.hpp>
export module Visera.Game.World;
#define VISERA_MODULE_NAME "Game.World"
       import Visera.Global.Log;
export import Visera.Global.Name;
       import Visera.Global.Service;
       import Visera.Core.Types.Map;
       import Visera.Core.Math.Random.Seed;
       import Visera.Game.World.Entity;
export import Visera.Game.World.Component;
export import Visera.Game.World.System;

namespace Visera
{
    export class VISERA_ENGINE_API FWorld : public IGlobalService<FWorld>
    {
    public:
        void inline
        Tick(Float I_DeltaTime);

        VISERA_NOINLINE FEntity
        CreateEntity(FName I_Name);

    private:
        UInt32                  Seed {0};

        entt::registry          GlobalRegistry{};
        TMap<FName, FEntity>    Entities{};
        SMovement               MovementSystem;
        SPhysics2D              Physics2DSystem;

    public:
        FWorld() : IGlobalService(FName{"World"}) {}
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

    /*void FWorld::
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
    }*/
}
