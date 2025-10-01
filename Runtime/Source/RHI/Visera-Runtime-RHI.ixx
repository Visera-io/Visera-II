module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Runtime.RHI.Frame;
import Visera.Core.Log;

namespace Visera
{
    export namespace RHI
    {
        using EQueue        = EVulkanQueue;
        using EImageType    = EVulkanImageType;
        using EFormat       = EVulkanFormat;
        using EImageUsage   = EVulkanImageUsage;
        using EImageLayout  = EVulkanImageLayout;
        using EAccess       = EVulkanAccess;
    }

    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        void
        BeginFrame()  const { GetDriver()->BeginFrame(); };
        void
        EndFrame()    const { GetDriver()->EndFrame(); };

        [[nodiscard]] inline const TUniquePtr<RHI::FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" accessed the RHI driver.",
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<RHI::FVulkanDriver> Driver;
        TArray<RHI::FFrame>            Frames;

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

        try
        {
            Driver = MakeUnique<RHI::FVulkanDriver>();
            Frames.resize(Driver->GetFrameCount());
            LOG_TRACE("Created {} frames.", Frames.size());
        }
        catch (const SRuntimeError& Error)
        {
            LOG_FATAL("{}", Error.what());
        }

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
