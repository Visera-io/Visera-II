module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.Graphics.RenderGraph.Node;
import Visera.RHI.CommandList;
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
        AddNode(FName I_Name, TUnicastDelegate<void(FRHICommandList*)> I_Execute);
        void inline
        Execute(FRHICommandList* I_DrawCommandList);
        void inline
        Clear() { VISERA_UNIMPLEMENTED_API; }
        [[nodiscard]] Bool inline
        Compile() { VISERA_UNIMPLEMENTED_API; }
        [[nodiscard]] Bool  inline
        CreateFromJSON(const FJSON& I_JSON) { VISERA_UNIMPLEMENTED_API; }

    private:
        //TPMRArray<FRGNode>     Nodes;
        Memory::TMonotonicArena<1_MB>  LocalArena;
    };

    void FRenderGraph::
    AddNode(FName I_Name, TUnicastDelegate<void(FRHICommandList*)> I_Execute)
    {
        // Nodes.emplace_back(FRGNode{
        //     .Name    = I_Name,
        //     .Execute = std::move(I_Execute)
        // });
    }

    void FRenderGraph::
    Execute(FRHICommandList* I_CommandBuffer)
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