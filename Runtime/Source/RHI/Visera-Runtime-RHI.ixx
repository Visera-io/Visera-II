module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Driver.Interface;
import Visera.Runtime.RHI.Driver.Vulkan;
import Visera.Runtime.RHI.Frame;
import Visera.Core.Log;

namespace Visera
{
    export namespace RHI
    {

    }

    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        void
        BeginFrame()  const { GetDriver()->BeginFrame(); };
        void
        EndFrame()    const { GetDriver()->EndFrame(); };

        [[nodiscard]] inline const TUniquePtr<RHI::IDriver>&
        GetDriver()   const { return Driver; };

    private:
        TUniquePtr<RHI::IDriver> Driver;
        TArray<RHI::FFrame>      Frames;

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
            Driver = MakeUnique<RHI::FVulkan>();
            Frames.resize(Driver->GetFrameCount());
            LOG_TRACE("Created {} frames.", Frames.size());
        }
        catch (const SRuntimeError& Error)
        {
            LOG_FATAL("{}", Error.what());
        }

        Status = EStatues::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");
        Driver.reset();

        Status = EStatues::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        { LOG_FATAL("RHI must be terminated properly!"); }
    }

}
