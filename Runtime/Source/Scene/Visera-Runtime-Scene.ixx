module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scene;
#define VISERA_MODULE_NAME "Runtime.Scene"
export import Visera.Runtime.Scene.ECS;
export import Visera.Runtime.Scene.Object;
       import Visera.Core.Log;
       import Visera.Core.Types.Name;

namespace Visera
{
    class VISERA_RUNTIME_API FScene : IGlobalSingleton
    {
    public:
        template<Concepts::Object T, typename... Args> TSharedPtr<T>
        CreateObject(const FName& I_Name, Args&&... I_Args)
        {
            if (Objects.contains(I_Name))
            {
                LOG_ERROR("Failed to create the existed object \"{}\".", I_Name);
                return nullptr;
            }

            auto Object = MakeShared<T>(std::forward<Args>(I_Args)...);
            Object->SetName(I_Name);

            Objects[I_Name] = Object;
            Object->Awake();
            return Object;
        }

        template<Concepts::Object T> TSharedPtr<T>
        GetObject(const FName& I_Name) const
        {
            if (!Objects.contains(I_Name))
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<T>(Objects.at(I_Name));
        }

        Bool
        DestroyObject(const FName& I_Name)
        {
            auto It = Objects.find(I_Name);
            if (It != Objects.end())
            {
                Objects.erase(It);
                return True;
            }
            return False;
        }

    private:
        TMap<FName, TSharedPtr<VObject>> Objects;

    public:
        FScene() : IGlobalSingleton{"Scene"} {}
        void Bootstrap() override;
        void Terminate() override;
    };

    export VISERA_RUNTIME_API TUniquePtr<FScene>
    GScene = MakeUnique<FScene>();

    UInt64 VObject::UUID = 0;

    void VObject::
    ResetUUID() { UUID = 0; }

    void FScene::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Scene.");

        VObject::ResetUUID();

        Status = EStatues::Bootstrapped;
    }

    void FScene::
    Terminate()
    {
        LOG_TRACE("Terminating Scene.");

        Objects.clear();
        Status = EStatues::Terminated;
    }
}