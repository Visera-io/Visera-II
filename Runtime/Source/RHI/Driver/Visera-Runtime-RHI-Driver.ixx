module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Driver;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera::RHI
{
    export enum class EDriverType
    {
        Unknown,
        //Dummy,

        Vulkan,
    };

    export class VISERA_RUNTIME_API IDriver : public IGlobalSingleton
    {
    public:
        inline EDriverType
        GetType() const { return Type; }

        virtual void
        BeginFrame() = 0;
        virtual void
        EndFrame()   = 0;
        virtual void
        Present()    = 0;

        virtual void*
        GetNativeInstance() = 0;
        virtual void*
        GetNativeDevice()   = 0;

        explicit IDriver(EDriverType I_DriverType)
        :
        IGlobalSingleton{I_DriverType == EDriverType::Vulkan? "Vulkan" : "Unknown RHI Driver"},
        Type{ I_DriverType }
        {  }

    private:
        EDriverType Type {EDriverType::Unknown};
    };
}
