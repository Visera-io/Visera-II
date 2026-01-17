module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.Graphics.RenderGraph.Node;
import Visera.RHI.Types.CommandBuffer;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Array;
import Visera.Core.Types.JSON;
import Visera.Core.Delegate.Unicast;
import Visera.Global.Name;
import Visera.Global.Log;

export namespace Visera
{
    class VISERA_GRAPHICS_API FRenderGraph
    {
    public:
        void inline
        AddNode(FName I_Name, TUnicastDelegate<void(FRHIDrawCommandList*)> I_Execute);
        void inline
        Execute(FRHIDrawCommandList* I_DrawCommandList);
        void inline
        Clear();
        [[nodiscard]] Bool inline
        Compile();
        [[nodiscard]] Bool  inline
        CreateFromJSON(const FJSON& I_JSON);

    private:
        //TPMRArray<FRGNode>     Nodes;
        TMonotonicArena<1_MB>  LocalArena;
    };

    void FRenderGraph::
    AddNode(FName I_Name, TUnicastDelegate<void(FRHIDrawCommandList*)> I_Execute)
    {
        // Nodes.emplace_back(FRGNode{
        //     .Name    = I_Name,
        //     .Execute = std::move(I_Execute)
        // });
    }

    void FRenderGraph::
    Execute(FRHIDrawCommandList* I_CommandBuffer)
    {
        if (!I_CommandBuffer) { return; }
        // for (auto& Node : Nodes)
        // {
        //     LOG_TRACE("Executing render graph node: {}", Node.Name);
        //     Node.Execute.Invoke(I_CommandBuffer);
        // }
    }

    // void FRenderGraph::
    // Clear()
    // {
    //     Nodes.clear();
    // }

    // Bool FRenderGraph::
    // CreateFromJSON(const FJSON& I_JSON)
    // {
    //     VISERA_UNIMPLEMENTED_API;
    //     return True;
    // }

    // Bool FRenderGraph::
    // Compile()
    // {

    //     VISERA_UNIMPLEMENTED_API;
    //     return True;
    // }
}