module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.GFX.Shaders;
#define VISERA_MODULE_NAME "Runtime.GFX"
import Visera.Runtime.GFX.Shaders.Slang;

export namespace Visera
{
    class VISERA_RUNTIME_API FShader
    {
    private:
        FSlangShader Shader;
    };
}
