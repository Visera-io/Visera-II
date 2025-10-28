module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
       import Visera.Runtime.RHI.SPIRV;
       import Visera.Runtime.RHI.Vulkan;
       import Visera.Core.Types.Name;
       import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        inline void
        BeginFrame()  const { Driver->BeginFrame(); }
        inline void
        EndFrame()    const { Driver->EndFrame(); }
        inline void
        Present()     const { Driver->Present(); }

        [[nodiscard]] inline Bool
        CreateRenderPass(const FName&                  I_Name,
                         TSharedRef<RHI::FSPIRVShader> I_VertexShader,
                         TSharedRef<RHI::FSPIRVShader> I_FragmentShader)
        {
            VISERA_ASSERT(I_VertexShader->IsVertexShader());
            VISERA_ASSERT(I_FragmentShader->IsFragmentShader());
            //LOG_DEBUG("Creating a Vulkan Render Pass (name:{}, vertex:{}, fragment:{}).",
             //         I_Name, I_VertexShader->GetPath(), I_FragmentShader->GetPath());
        //VISERA_ASSERT(VertexShader->IsVertexShader());
        //(FragmentShader->IsFragmentShader());
            return True;
        }

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<RHI::FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<RHI::FVulkanDriver> Driver;

    public:
        FRHI() : IGlobalSingleton("RHI") {}
        ~FRHI() override;
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping RHI.");
        Driver = MakeUnique<RHI::FVulkanDriver>();

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");
        Driver.reset();

        Status = EStatus::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        { LOG_WARN("RHI must be terminated properly!"); }
    }

}
