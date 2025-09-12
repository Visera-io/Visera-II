module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Driver.Interface;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{

    export class VISERA_RUNTIME_API IDriver : public IGlobalSingleton
    {
    public:
        enum class EType
        {
            Unknown,
            //Null,
            Vulkan,
        };
        
        inline EType
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

        explicit IDriver(EType I_Type)
        :
        IGlobalSingleton{GetTypeString(I_Type)},
        Type{ I_Type }
        {
            LOG_INFO("RHI Driver: {}", GetDebugName());
        }

    private:
        EType Type {EType::Unknown};

    private:
        static auto GetTypeString(EType I_Type) -> const char*
        {
            switch (I_Type)
            {
                case EType::Vulkan: return "Vulkan";
                default: return "Unknown";
            }
        }
    };
}
