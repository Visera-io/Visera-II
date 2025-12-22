module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.Graphics.RenderGraph.Node;
import Visera.RHI;
import Visera.Core.Types.Array;
import Visera.Core.Delegate.Unicast;
import Visera.Runtime.Name;
import Visera.Runtime.Log;

export namespace Visera
{
    //[TODO]: WIP
    class VISERA_GRAPHICS_API FRenderGraph
    {
    public:
        void inline
        AddNode(FName I_Name, TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>)> I_Execute);
        void inline
        Execute(TSharedRef<FRHICommandBuffer> I_CommandBuffer);
        void inline
        Clear();

    private:
        TArray<FRGNode> Nodes;
    };

    void FRenderGraph::
    AddNode(FName I_Name, TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>)> I_Execute)
    {
        Nodes.emplace_back(FRGNode{
            .Name    = I_Name,
            .Execute = std::move(I_Execute)
        });
    }

    void FRenderGraph::
    Execute(TSharedRef<FRHICommandBuffer> I_CommandBuffer)
    {
        for (auto& Node : Nodes)
        {
            LOG_TRACE("Executing render graph node: {}", Node.Name);
            Node.Execute.Invoke(I_CommandBuffer);
        }
    }

    void FRenderGraph::
    Clear()
    {
        Nodes.clear();
    }
}