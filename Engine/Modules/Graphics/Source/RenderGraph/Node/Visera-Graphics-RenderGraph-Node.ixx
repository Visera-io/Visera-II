module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Node;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.RHI;
import Visera.Shader;
import Visera.Runtime.Name;
import Visera.Core.Delegate.Unicast;
import Visera.Graphics.RenderGraph.Common;

export namespace Visera
{
    struct FRGTextureHandle
    {
        UInt32 Index = 0xFFFFFFFF;
        [[nodiscard]] inline Bool
        IsValid() const { return Index != 0xFFFFFFFF; }
        Bool operator==(const FRGTextureHandle& I_Other) const { return Index == I_Other.Index; }
    };

    // struct FRGTextureDesc
    // {
    //     FName           Name;
    //     FRHITextureDesc Desc;
    //     TSharedPtr<FRHITexture> PhysicalTexture = nullptr;
    //     EResourceState          CurrentState = EResourceState::Undefined;
    //
    //     UInt32 ReferenceCount = 0;
    // };

    struct FRGNode
    {
        FName         Name;
        //TArray<FRGTexture>  Textures;

        TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>)>
        Execute;
    };
}