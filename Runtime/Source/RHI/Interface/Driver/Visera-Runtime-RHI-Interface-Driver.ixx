module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Driver;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.AssetHub.Shader;
import Visera.Runtime.RHI.Interface.CommandBuffer;
import Visera.Runtime.RHI.Interface.Fence;
import Visera.Runtime.RHI.Interface.ShaderModule;
import Visera.Runtime.RHI.Interface.RenderPass;
import Visera.Core.Log;

namespace Visera::RHI
{
    export using ECommandType = ICommandBuffer::EType;

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
        CreateShaderModule(TSharedPtr<FShader> I_Shader) = 0;
        [[nodiscard]] virtual TSharedPtr<IRenderPass>
        CreateRenderPass(TSharedPtr<IShaderModule> I_VertexShader,
                         TSharedPtr<IShaderModule> I_FragmentShader) = 0;
        [[nodiscard]] virtual TUniquePtr<IFence>
        CreateFence(Bool I_bSignaled) = 0;
        [[nodiscard]] virtual TSharedPtr<ICommandBuffer>
        CreateCommandBuffer(ECommandType I_Type) = 0;

        [[nodiscard]] virtual const void*
        GetNativeInstance() const = 0;
        [[nodiscard]] virtual const void*
        GetNativeDevice()   const = 0;

        [[nodiscard]] inline EType
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
