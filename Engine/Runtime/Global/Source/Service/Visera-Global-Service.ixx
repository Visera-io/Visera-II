module;
#include <Visera-Global.hpp>
export module Visera.Global.Service;
#define VISERA_MODULE_NAME "Global.Service"
export import Visera.Global.Name;
       import Visera.Core.Types.Map;
       import Visera.Core.Delegate.Unicast;

export namespace Visera
{
    template<typename T>
    class VISERA_GLOBAL_API IGlobalService
    {
        //static TMap<FName, TUniquePtr<IGlobalService>> Services;
    public:
        enum class EStatus { Disabled, Bootstrapped, Terminated };

        [[nodiscard]] static T*
        Get(FName I_ServiceName)
        {
            //auto   ServiceIter = Services.find(I_ServiceName);
            //return ServiceIter == Services.end() ?
            //       nullptr : ServiceIter->second.get();
        }

        [[nodiscard]] Bool
        IsBootstrapped() const { return Status == EStatus::Bootstrapped; }
        [[nodiscard]] Bool
        IsTerminated() const   { return Status == EStatus::Terminated; }

        [[nodiscard]] FStringView
        GetDebugName() const { return Name.GetName(); }

        virtual ~IGlobalService()
        {
            if (IsBootstrapped())
            { printf("%s",Format("{} NOT terminated properly!", GetDebugName()).data()); }
        }

    protected:
        const   FName   Name;
        mutable EStatus Status = EStatus::Disabled;

        TUnicastDelegate<Bool(void)> OnBootstrap;
        TUnicastDelegate<Bool(void)> OnTerminate;

    public:
        IGlobalService() = delete;
        explicit IGlobalService(FName I_Name) : Name(I_Name)
        {
            //VISERA_ASSERT(!Services.contains(I_Name));
            //Services.insert({I_Name, MakeUnique<IGlobalService>(this))};
        }

        IGlobalService(const IGlobalService&)			   = delete;
        IGlobalService& operator=(const IGlobalService&) = delete;
        IGlobalService(IGlobalService&&)				   = delete;
        IGlobalService& operator=(IGlobalService&&)      = delete;
    };
}