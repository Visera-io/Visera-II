module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.GFX;
#define VISERA_MODULE_NAME "Runtime.GFX"
export import Visera.Runtime.GFX.Shaders;
       import Visera.Runtime.RHI;

namespace Visera
{
    class FGFX : public IGlobalSingleton
    {
    public:
        FGFX() : IGlobalSingleton{"GFX"}{}
        ~FGFX() override;

        void Bootstrap() override;
        void Terminate() override;
    };

    // Legacy global for backward compatibility
    export inline VISERA_RUNTIME_API TUniquePtr<FGFX>
    GGFX = MakeUnique<FGFX>();

    void FGFX::Bootstrap()
    {
        Statue = EStatues::Bootstrapped;
    }

    void FGFX::Terminate()
    {
        Statue = EStatues::Terminated;
    }

    FGFX::~FGFX()
    {
        if (IsBootstrapped())
        {
            //LOG_WARN("GFX destroyed without proper termination!");
        }
    }

}
