module;
#include <Visera-Global.hpp>
export module Visera.Global.Service;
#define VISERA_MODULE_NAME "Global.Service"
export import Visera.Global.Log;
export import Visera.Global.Name;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Set;
       import Visera.Core.Delegate.Unicast;

export namespace Visera
{
    namespace EName
    {
        VISERA_GLOBAL_API inline const auto
        Platform  = FName{"platform",    0};
        VISERA_GLOBAL_API inline const auto
        Input     = FName{"input",       0};
        VISERA_GLOBAL_API inline const auto
        RHI       = FName{"rhi",         0};
        VISERA_GLOBAL_API inline const auto
        Graphics  = FName{"graphics",    0};
        VISERA_GLOBAL_API inline const auto
        Audio     = FName{"audio",       0};
        VISERA_GLOBAL_API inline const auto
        Shader    = FName{"shader",      0};
        VISERA_GLOBAL_API inline const auto
        Physics2D = FName{"physics2d",   0};
    }

    class VISERA_GLOBAL_API IGlobalService
    {
        static auto* GetRegistry()
        {
            static auto* Registry = new TMap<FName, IGlobalService*>();
            return Registry;
        }
    public:
        enum class EStatus { Pending, Bootstrapped, Terminated };

        template<typename T> [[nodiscard]] static T*
        Get(FName I_ServiceName)
        {
            auto   ServiceIter =  GetRegistry()->find(I_ServiceName);
            return ServiceIter == GetRegistry()->end() ?
                   nullptr : dynamic_cast<T*>(ServiceIter->second);
        }

        [[nodiscard]] Bool
        IsPending()      const { return Status == EStatus::Pending; }
        [[nodiscard]] Bool
        IsBootstrapped() const { return Status == EStatus::Bootstrapped; }
        [[nodiscard]] Bool
        IsTerminated()   const { return Status == EStatus::Terminated; }

        [[nodiscard]] FStringView
        GetDebugName() const { return Name.GetName(); }

    protected:
        TSet<FName> Dependencies;

        TUnicastDelegate<Bool(void)> OnBootstrap;
        TUnicastDelegate<Bool(void)> OnTerminate;

        // Can only be deleted inside the IGlobalService.
        virtual ~IGlobalService()
        {
            if (IsPending())
            { LOG_WARN("Service \"{}\" was NOT bootstrapped!", GetDebugName()); }
            else if (IsBootstrapped())
            { LOG_ERROR("Service \"{}\" was NOT terminated!", GetDebugName()); }
        }

    private:
        const   FName   Name;
        mutable EStatus Status = EStatus::Pending;

    public:
        IGlobalService() = delete;
        explicit IGlobalService(FName I_Name) : Name(I_Name)
        {
            VISERA_ASSERT(!GetRegistry()->contains(Name));
            LOG_DEBUG("Service ({}) : \"{}\".", GetRegistry()->size() + 1, Name.GetName());
            GetRegistry()->emplace(Name, this);
        }

        IGlobalService(const IGlobalService&)			   = delete;
        IGlobalService& operator=(const IGlobalService&) = delete;
        IGlobalService(IGlobalService&&)				   = delete;
        IGlobalService& operator=(IGlobalService&&)      = delete;
    };
}