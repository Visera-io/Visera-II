module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.Shader;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.Graphics.Shader.Slang;

export namespace Visera
{
    class VISERA_RUNTIME_API FShader
    {
    private:
        FSlangShader Shader;
    };
}
