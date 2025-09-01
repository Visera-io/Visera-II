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


    public:
        FGFX() : IGlobalSingleton{"GFX"}{}
        ~FGFX() override;

        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FGFX>
    GGFX = MakeUnique<FGFX>();

    void FGFX::
    Bootstrap()
    {
        if (!GRHI->IsBootstrapped())
        { throw SRuntimeError("Failed to bootstrap GFX because RHI is not bootstrapped but GFX depends on it!"); }


    }

    void FGFX::
    Terminate()
    {

    }

    FGFX::
    ~FGFX()
    {

    }

}
