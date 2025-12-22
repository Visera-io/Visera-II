module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderPass.Interface;
#define VISERA_MODULE_NAME "Graphics.RenderPass"
import Visera.RHI;
import Visera.Shader;
import Visera.Core.Delegate.Unicast;

export namespace Visera
{
    struct FRGNode
    {
        FStringView         Name;
        //TArray<FRGTexture>  Textures;

        TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>)>
        Execute;
    };
}