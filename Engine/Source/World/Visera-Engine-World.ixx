module;
#include <Visera-Engine.hpp>
#include <entt/entt.hpp>
export module Visera.Engine.World;
#define VISERA_MODULE_NAME "Engine.World"
       import Visera.Core.Log;
       import Visera.Core.Global;
       import Visera.Core.Types.Name;
       import Visera.Core.Types.Map;
       import Visera.Engine.World.Entity;
export import Visera.Engine.World.Component;
export import Visera.Engine.World.System;

namespace Visera
{
    export class VISERA_ENGINE_API FWorld : public IGlobalSingleton<FWorld>
    {
    public:
        [[nodiscard]] FEntity // Managed by Registry rather than SharedPtr
        CreateEntity(FName I_Name);

    private:
        entt::registry          GlobalRegistry{};
        TMap<FName, FEntity>    Entities{};

    public:
        FWorld() : IGlobalSingleton("World") {}
        void Bootstrap() override;
        void Terminate() override;
    };

    export inline VISERA_ENGINE_API TUniquePtr<FWorld>
    GWorld = MakeUnique<FWorld>();

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

        Status = EStatus::Bootstrapped;
    }

    void FWorld::
    Terminate()
    {
        LOG_DEBUG("Terminating World");

        Status = EStatus::Terminated;
    }
}
