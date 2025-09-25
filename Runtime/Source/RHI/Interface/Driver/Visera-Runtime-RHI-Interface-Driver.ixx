module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Driver;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.AssetHub.Shader;
import Visera.Runtime.RHI.Interface.CommandPool;
import Visera.Runtime.RHI.Interface.Fence;
import Visera.Runtime.RHI.Interface.ShaderModule;
import Visera.Runtime.RHI.Interface.RenderPass;
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
        virtual void
        BeginFrame() = 0;
        virtual void
        EndFrame()   = 0;
        virtual void
        Present()    = 0;
        [[nodiscard]] virtual UInt32
        GetFrameCount() const = 0;
        [[nodiscard]] virtual TSharedPtr<IShaderModule>
        CreateShaderModule(TSharedPtr<FShader> I_Shader) const = 0;
        [[nodiscard]] virtual TUniquePtr<IRenderPass>
        CreateRenderPass(TSharedPtr<IShaderModule> I_VertexShader,
                         TSharedPtr<IShaderModule> I_FragmentShader) const = 0;
        [[nodiscard]] virtual TUniquePtr<IFence>
        CreateFence(Bool I_bSignaled) const = 0;

        [[nodiscard]] virtual const void*
        GetNativeInstance() const = 0;
        [[nodiscard]] virtual const void*
        GetNativeDevice()   const = 0;

        inline EType
        GetType() const { return Type; }

    private:
        EType Type {EType::Unknown};

    public:
        explicit IDriver() = delete;
        explicit IDriver(EType I_Type) : Type{ I_Type }
        { LOG_INFO("RHI Driver: {}", GetTypeString(Type)); }
        virtual ~IDriver() = default;

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
