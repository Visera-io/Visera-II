module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Driver.Interface;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{

    export class VISERA_RUNTIME_API IDriver
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
        [[nodiscard]] virtual UInt32
        GetFrameCount() const = 0;

        [[nodiscard]] virtual const void*
        GetNativeInstance() const = 0;
        [[nodiscard]] virtual const void*
        GetNativeDevice()   const = 0;

        explicit IDriver() = delete;
        explicit IDriver(EType I_Type) : Type{ I_Type }
        { LOG_INFO("RHI Driver: {}", GetTypeString(Type)); }
        virtual ~IDriver() = default;

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
