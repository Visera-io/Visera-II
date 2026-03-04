module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.RenderPipeline.RenderGraph;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;
import Visera.Core.Containers.Array;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Pointer;
import Visera.Core.Log;

export namespace Visera
{
    // =========================================================================
    // FGraphicsID -- Unified resource handle for RenderGraph textures & buffers
    // =========================================================================
    struct VISERA_RUNTIME_API FGraphicsID
    {
        UInt32 Value;

        static constexpr UInt32 kBufferBit = 1U << 31;
        static constexpr UInt32 kIndexMask = ~kBufferBit;

        static constexpr FGraphicsID Invalid()              { return {kIndexMask}; }
        static FGraphicsID Texture(UInt32 I_Index)          { return {I_Index}; }
        static FGraphicsID Buffer(UInt32 I_Index)           { return {I_Index | kBufferBit}; }

        [[nodiscard]] Bool   IsTexture() const { return (Value & kBufferBit) == 0 && IsValid(); }
        [[nodiscard]] Bool   IsBuffer()  const { return (Value & kBufferBit) != 0; }
        [[nodiscard]] Bool   IsValid()   const { return (Value & kIndexMask) != kIndexMask; }
        [[nodiscard]] UInt32 GetIndex()  const { return (Value & kIndexMask); }

        Bool operator==(const FGraphicsID&) const = default;
    };

    // Forward declaration for IRGPass::Execute parameter
    class FRenderGraph;

    PROFILING_ONLY_FIELD(
    struct FRenderGraphCompileProfilingMetrics
    {
        UInt64 TotalCompiles      {0};
        UInt64 PeakNodes          {0};
        UInt64 TotalCulled        {0};
        UInt64 PeakArenaBytesUsed {0};
        UInt64 ArenaSpillCount    {0};
    };
    )

    // =========================================================================
    // IRGPass -- Base interface for all RenderGraph pass types
    // =========================================================================
    class VISERA_RUNTIME_API IRGPass
    {
    public:
        enum class EType : UInt8 { Render, Compute };

        virtual void
        Execute(const FRenderGraph* I_Graph, FRHICommandList* I_CommandList) = 0;

        [[nodiscard]] virtual const char*
        GetName() const = 0;

        [[nodiscard]] virtual EType
        GetType() const = 0;

        virtual ~IRGPass() = default;

    protected:
        IRGPass() = default;
        IRGPass(const IRGPass&) = delete;
        IRGPass& operator=(const IRGPass&) = delete;
        IRGPass(IRGPass&&) = default;
        IRGPass& operator=(IRGPass&&) = default;
    };

    class VISERA_RUNTIME_API IComputePass : public IRGPass
    {
    public:
        IRGPass::EType
        GetType() const final { return IRGPass::EType::Compute; }

        [[nodiscard]] const char*
        GetName() const override { return "IComputePass"; }

        ~IComputePass() override = default;

    protected:
        IComputePass() = default;
        IComputePass(const IComputePass&) = delete;
        IComputePass& operator=(const IComputePass&) = delete;
        IComputePass(IComputePass&&) = default;
        IComputePass& operator=(IComputePass&&) = default;
    };

    class VISERA_RUNTIME_API IRenderPass : public IRGPass
    {
    public:
        IRGPass::EType
        GetType() const final { return IRGPass::EType::Render; }

        [[nodiscard]] const char*
        GetName() const override { return "IRenderPass"; }

        ~IRenderPass() override = default;

    protected:
        IRenderPass() = default;
        IRenderPass(const IRenderPass&) = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;
        IRenderPass(IRenderPass&&) = default;
        IRenderPass& operator=(IRenderPass&&) = default;
    };

    namespace Concepts
    {
        template<typename T> concept
        RenderPass = std::derived_from<T, IRenderPass>;
    }

    namespace Concepts
    {
        template<typename T> concept
        ComputePass = std::derived_from<T, IComputePass>;
    }

    // =========================================================================
    // FRGNode -- A single node in the RenderGraph
    // =========================================================================
    struct FRGNode
    {
        TUniquePtr<IRGPass>     Pass;
        TArray<FGraphicsID>     Reads;
        TArray<FGraphicsID>     Writes;
    };

    // =========================================================================
    // FRGTexture -- Transient or external texture managed by the RenderGraph
    // =========================================================================
    using ERGFormat = ERHIFormat;

    class VISERA_RUNTIME_API FRGTexture
    {
    public:
        struct FCreateInfo
        {
            UInt32    Width  {0};
            UInt32    Height {0};
            ERGFormat Format {ERGFormat::Undefined};
        };

        FRGTexture() = default;
        explicit FRGTexture(FRHITextureID I_Imported) : CreateInfo{}, RHIID(I_Imported), bIsExternal(True) {}
        explicit FRGTexture(const FCreateInfo& I_CreateInfo) : CreateInfo(I_CreateInfo), RHIID{}, bIsExternal(False) {}

        [[nodiscard]] const FRHITextureID& GetRHIID()      const { return RHIID; }
        [[nodiscard]] Bool              IsExternal()    const { return bIsExternal; }
        [[nodiscard]] Bool              IsTransient()   const { return !bIsExternal; }
        [[nodiscard]] const FCreateInfo& GetCreateInfo() const { return CreateInfo; }
        void SetRHIID(FRHITextureID I_ID) { RHIID = I_ID; }

    private:
        FCreateInfo    CreateInfo;
        FRHITextureID  RHIID;
        Bool           bIsExternal {False};
    };

    using FRGTextureCreateInfo = FRGTexture::FCreateInfo;

    // =========================================================================
    // FRGBuffer -- Transient or external buffer managed by the RenderGraph
    // =========================================================================
    class VISERA_RUNTIME_API FRGBuffer
    {
    public:
        struct FCreateInfo
        {
            UInt64          Size   {0};
            ERHIBufferUsage Usages {ERHIBufferUsage::StorageBuffer};
        };

        FRGBuffer() = default;
        explicit FRGBuffer(FRHIBufferID I_Imported) : CreateInfo{}, RHIID(I_Imported), bIsExternal(True) {}
        explicit FRGBuffer(const FCreateInfo& I_CreateInfo) : CreateInfo(I_CreateInfo), RHIID{}, bIsExternal(False) {}

        [[nodiscard]] const FRHIBufferID& GetRHIID()     const { return RHIID; }
        [[nodiscard]] Bool              IsExternal()   const { return bIsExternal; }
        [[nodiscard]] Bool              IsTransient()  const { return !bIsExternal; }
        [[nodiscard]] const FCreateInfo& GetCreateInfo() const { return CreateInfo; }
        void SetRHIID(FRHIBufferID I_ID) { RHIID = I_ID; }

    private:
        FCreateInfo   CreateInfo;
        FRHIBufferID  RHIID;
        Bool          bIsExternal {False};
    };

    using FRGBufferCreateInfo = FRGBuffer::FCreateInfo;

    // =========================================================================
    // FRenderGraph
    // =========================================================================
    using FSwapChainID = FRHISwapChainID;

    class VISERA_RUNTIME_API FRenderGraph final
    {
    public:
        [[nodiscard]] FSwapChainID
        GetSwapChainID() const;

        [[nodiscard]] Bool
        HasBackBuffer() const;

        /** BackBuffer as FGraphicsID (texture slot 0 when present). Invalid when headless. */
        [[nodiscard]] FGraphicsID
        GetBackBuffer() const;

        /** BackBuffer as const FRHITextureID& for recording clear/transition. Null when headless. */
        [[nodiscard]] const FRHITextureID&
        GetBackBufferRHI() const;

        /** Resolve FGraphicsID to const FRHITextureID&. Returns null ref on error. */
        [[nodiscard]] const FRHITextureID&
        GetTexture(FGraphicsID I_ID) const;

        /** Resolve FGraphicsID to const FRHIBufferID&. Returns null ref on error. */
        [[nodiscard]] const FRHIBufferID&
        GetBuffer(FGraphicsID I_ID) const;

        // -----------------------------------------------------------------
        // Node management
        // -----------------------------------------------------------------

        /** Add a render or compute pass node. Pass type is determined by IRGPass::GetType(). */
        FRGNode*
        AddNode(TUniquePtr<IRGPass>  I_Pass,
                TArray<FGraphicsID>  I_Reads  = {},
                TArray<FGraphicsID>  I_Writes = {});

        // -----------------------------------------------------------------
        // Resource management
        // -----------------------------------------------------------------

        /** Create transient texture (allocated at Compile). */
        [[nodiscard]] FGraphicsID
        CreateTexture(const FRGTextureCreateInfo& I_CreateInfo);

        /** Register external texture (e.g. backbuffer). */
        [[nodiscard]] FGraphicsID
        RegisterExternalTexture(FRHITextureID I_Imported);

        /** Create transient buffer (allocated at Compile). */
        [[nodiscard]] FGraphicsID
        CreateBuffer(const FRGBufferCreateInfo& I_CreateInfo);

        /** Register external buffer. */
        [[nodiscard]] FGraphicsID
        RegisterExternalBuffer(FRHIBufferID I_Imported);

        // -----------------------------------------------------------------
        // Compile / Execute
        // -----------------------------------------------------------------

        /** Allocate RHI resources, cull dead passes, topological sort, compute barriers. */
        FRenderGraph*
        Compile(FRHI* I_RHI);

        void
        Execute(FRHICommandList* I_CommandList) const;

        /** Construct for headless (no backbuffer). */
        explicit FRenderGraph(FSwapChainID I_SwapChainID);

        /** Construct with optional backbuffer. If valid, registers it as external texture at slot 0. */
        FRenderGraph(FSwapChainID I_SwapChainID, FRHITextureID I_BackBuffer);

        PROFILING_ONLY_FIELD(
        friend void LogRenderGraphCompileProfilingSummary()
        {
            LOG_INFO("[Profiling] RenderGraph.Compile summary: total_compiles={} peak_nodes={} total_culled={} arena_peak_bytes={} arena_spill_count={}.",
              FRenderGraph::ProfilingMetrics.TotalCompiles,
              FRenderGraph::ProfilingMetrics.PeakNodes,
              FRenderGraph::ProfilingMetrics.TotalCulled,
              FRenderGraph::ProfilingMetrics.PeakArenaBytesUsed,
              FRenderGraph::ProfilingMetrics.ArenaSpillCount);
            if (FRenderGraph::ProfilingMetrics.PeakArenaBytesUsed >  kRenderGraphCompilingInlineMemory &&
                FRenderGraph::ProfilingMetrics.ArenaSpillCount    != 0)
            { LOG_WARN("Memory arena for compiling the render graph is insufficient!"); }
        })

    private:
        FSwapChainID        SwapChainID;
        Bool                bHasBackBuffer {False};
        TArray<FRGNode>       Nodes;
        TArray<FRGTexture>  Textures;
        TArray<FRGBuffer>   Buffers;
        TArray<UInt8>       NeedsBarrierBefore;

        PROFILING_ONLY_FIELD(
        static inline FRenderGraphCompileProfilingMetrics ProfilingMetrics;
        );

        void CullDeadPasses(std::pmr::memory_resource* I_Scratch);
        void TopologicalSort(std::pmr::memory_resource* I_Scratch);
        void ComputeBarrierFlags();
    };

    /** Call at app end (e.g. from FGraphics destructor) to log RenderGraph compile profiling summary. */
    void LogRenderGraphCompileProfilingSummary();

    // =================================================================
    // FRenderGraph — out-of-line definitions
    // =================================================================

    FSwapChainID FRenderGraph::
    GetSwapChainID() const { return SwapChainID; }

    Bool FRenderGraph::
    HasBackBuffer() const { return bHasBackBuffer; }

    FGraphicsID FRenderGraph::
    GetBackBuffer() const { return bHasBackBuffer ? FGraphicsID::Texture(0) : FGraphicsID::Invalid(); }

    const FRHITextureID& FRenderGraph::
    GetBackBufferRHI() const
    {
        static const FRHITextureID NullTexture{};
        if (!bHasBackBuffer || Textures.IsEmpty())
        {
            LOG_ERROR("RenderGraph::GetBackBufferRHI: no backbuffer available (headless context).");
            return NullTexture;
        }
        return Textures[0].GetRHIID();
    }

    const FRHITextureID& FRenderGraph::
    GetTexture(FGraphicsID I_ID) const
    {
        static const FRHITextureID NullTexture{};
        if (!I_ID.IsTexture())
        {
            LOG_ERROR("RenderGraph::GetTexture: ID is not a texture (value={:#x}).", I_ID.Value);
            return NullTexture;
        }
        const UInt32 Index = I_ID.GetIndex();
        if (Index >= Textures.GetSize())
        {
            LOG_ERROR("RenderGraph::GetTexture: index {} out of range (size={}).", Index, Textures.GetSize());
            return NullTexture;
        }
        return Textures[Index].GetRHIID();
    }

    const FRHIBufferID& FRenderGraph::
    GetBuffer(FGraphicsID I_ID) const
    {
        static const FRHIBufferID NullBuffer{};
        if (!I_ID.IsBuffer())
        {
            LOG_ERROR("RenderGraph::GetBuffer: ID is not a buffer (value={:#x}).", I_ID.Value);
            return NullBuffer;
        }
        const UInt32 Index = I_ID.GetIndex();
        if (Index >= Buffers.GetSize())
        {
            LOG_ERROR("RenderGraph::GetBuffer: index {} out of range (size={}).", Index, Buffers.GetSize());
            return NullBuffer;
        }
        return Buffers[Index].GetRHIID();
    }

    FRGNode* FRenderGraph::
    AddNode(TUniquePtr<IRGPass>  I_Pass,
            TArray<FGraphicsID>  I_Reads,
            TArray<FGraphicsID>  I_Writes)
    {
        auto& NewNode = Nodes.EmplaceBack(FRGNode{
            .Pass   = std::move(I_Pass),
            .Reads  = std::move(I_Reads),
            .Writes = std::move(I_Writes),
        });
        LOG_TRACE("RenderGraph: added {} node '{}'.",
                  NewNode.Pass->GetType() == IRGPass::EType::Render ? "render" : "compute",
                  NewNode.Pass->GetName());
        return &NewNode;
    }

    FGraphicsID FRenderGraph::
    CreateTexture(const FRGTextureCreateInfo& I_CreateInfo)
    {
        Textures.EmplaceBack(FRGTexture(I_CreateInfo));
        return FGraphicsID::Texture(static_cast<UInt32>(Textures.GetSize() - 1));
    }

    FGraphicsID FRenderGraph::
    RegisterExternalTexture(FRHITextureID I_Imported)
    {
        if (I_Imported.IsNull()) { return FGraphicsID::Invalid(); }
        Textures.EmplaceBack(FRGTexture(I_Imported));
        return FGraphicsID::Texture(static_cast<UInt32>(Textures.GetSize() - 1));
    }

    FGraphicsID FRenderGraph::
    CreateBuffer(const FRGBufferCreateInfo& I_CreateInfo)
    {
        Buffers.EmplaceBack(FRGBuffer(I_CreateInfo));
        return FGraphicsID::Buffer(static_cast<UInt32>(Buffers.GetSize() - 1));
    }

    FGraphicsID FRenderGraph::
    RegisterExternalBuffer(FRHIBufferID I_Imported)
    {
        if (I_Imported.IsNull()) { return FGraphicsID::Invalid(); }
        Buffers.EmplaceBack(FRGBuffer(I_Imported));
        return FGraphicsID::Buffer(static_cast<UInt32>(Buffers.GetSize() - 1));
    }

    FRenderGraph* FRenderGraph::
    Compile(FRHI* I_RHI)
    {
        if (!I_RHI) { return this; }

        for (auto& Texture : Textures)
        {
            if (!Texture.IsTransient()) { continue; }
            const auto& Info = Texture.GetCreateInfo();
            if (Info.Width == 0 || Info.Height == 0 || Info.Format == ERGFormat::Undefined) { continue; }
            FRHITextureCreateInfo RHIInfo{
                .Width    = Info.Width,
                .Height   = Info.Height,
                .Depth    = 1,
                .Format   = Info.Format,
                .Type     = ERHIImageType::Image2D,
                .Usages   = ERHIImageUsage::RenderTarget | ERHIImageUsage::ShaderResource,
                .ViewType = ERHIImageViewType::Image2D,
            };
            Texture.SetRHIID(I_RHI->CreateTexture(std::move(RHIInfo)));
            LOG_TRACE("RenderGraph::Compile: allocated transient texture ({}x{}).",
                      Info.Width, Info.Height);
        }

        for (auto& Buffer : Buffers)
        {
            if (!Buffer.IsTransient()) { continue; }
            const auto& Info = Buffer.GetCreateInfo();
            if (Info.Size == 0) { continue; }
            FRHIBufferCreateInfo RHIInfo{
                .Size   = Info.Size,
                .Usages = Info.Usages,
            };
            Buffer.SetRHIID(I_RHI->CreateBuffer(std::move(RHIInfo)));
            LOG_TRACE("RenderGraph::Compile: allocated transient buffer ({} bytes).", Info.Size);
        }

        Memory::TMonotonicArena<kRenderGraphCompilingInlineMemory> Arena;
        std::pmr::memory_resource* const Scratch = &Arena.Get();
        const UInt32 NodeCountBefore = static_cast<UInt32>(Nodes.GetSize());
        CullDeadPasses(Scratch);
        const UInt32 SurvivingCount = static_cast<UInt32>(Nodes.GetSize());
        TopologicalSort(Scratch);
        ComputeBarrierFlags();

        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.TotalCompiles;
        ProfilingMetrics.TotalCulled += (NodeCountBefore - SurvivingCount);
        if (NodeCountBefore > ProfilingMetrics.PeakNodes) { ProfilingMetrics.PeakNodes = NodeCountBefore; }
        {
            const std::size_t ArenaUsed = Arena.GetTotalBytesUsed();
            if (static_cast<UInt64>(ArenaUsed) > ProfilingMetrics.PeakArenaBytesUsed)
            { ProfilingMetrics.PeakArenaBytesUsed = static_cast<UInt64>(ArenaUsed); }
            ProfilingMetrics.ArenaSpillCount += static_cast<UInt64>(Arena.GetOverflowBlockCount());
        }
        )

        LOG_TRACE("RenderGraph::Compile: {} nodes, {} textures, {} buffers.",
                  Nodes.GetSize(), Textures.GetSize(), Buffers.GetSize());
        return this;
    }

    void FRenderGraph::
    Execute(FRHICommandList* I_CommandList) const
    {
        for (UInt32 i = 0; i < Nodes.GetSize(); ++i)
        {
            if (NeedsBarrierBefore.GetSize() > i && NeedsBarrierBefore[i])
            {
                I_CommandList->MemoryBarrier(FRHIMemoryBarrier{
                    .SourceStage  = ERHIPipelineStage::AllCommands,
                    .DestStage    = ERHIPipelineStage::AllCommands,
                    .SourceAccess = ERHIAccessFlag::MemoryRead | ERHIAccessFlag::MemoryWrite,
                    .DestAccess   = ERHIAccessFlag::MemoryRead | ERHIAccessFlag::MemoryWrite,
                });
            }

            const auto& Node = Nodes[i];
            if (Node.Pass)
            {
                LOG_TRACE("RenderGraph::Execute: running pass '{}'.", Node.Pass->GetName());
                Node.Pass->Execute(this, I_CommandList);
            }
        }
    }

    FRenderGraph::
    FRenderGraph(FSwapChainID I_SwapChainID) : SwapChainID(I_SwapChainID) {}

    FRenderGraph::
    FRenderGraph(FSwapChainID I_SwapChainID, FRHITextureID I_BackBuffer)
        : SwapChainID(I_SwapChainID)
        , bHasBackBuffer(!I_BackBuffer.IsNull())
    {
        if (bHasBackBuffer)
        { (void)RegisterExternalTexture(I_BackBuffer); }
    }

    void FRenderGraph::
    CullDeadPasses(std::pmr::memory_resource* I_Scratch)
    {
        const UInt32 NodeCount = static_cast<UInt32>(Nodes.GetSize());
        if (NodeCount == 0) { return; }

        TPMRArray<UInt8> Alive(I_Scratch);
        Alive.Resize(NodeCount, 0);

        auto IsExternalResource = [this](FGraphicsID I_ID) -> Bool
        {
            if (I_ID.IsTexture())
            {
                const UInt32 Index = I_ID.GetIndex();
                return Index < Textures.GetSize() && Textures[Index].IsExternal();
            }
            if (I_ID.IsBuffer())
            {
                const UInt32 Index = I_ID.GetIndex();
                return Index < Buffers.GetSize() && Buffers[Index].IsExternal();
            }
            return False;
        };

        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            for (const auto& W : Nodes[i].Writes)
            {
                if (IsExternalResource(W)) { Alive[i] = 1; break; }
            }
        }

        Bool Changed = True;
        while (Changed)
        {
            Changed = False;
            for (UInt32 i = 0; i < NodeCount; ++i)
            {
                if (Alive[i]) { continue; }
                for (const auto& W : Nodes[i].Writes)
                {
                    for (UInt32 j = 0; j < NodeCount; ++j)
                    {
                        if (!Alive[j]) { continue; }
                        for (const auto& R : Nodes[j].Reads)
                        {
                            if (R == W) { Alive[i] = 1; Changed = True; break; }
                        }
                        if (Alive[i]) { break; }
                    }
                    if (Alive[i]) { break; }
                }
            }
        }

        TPMRArray<FRGNode> SurvivingNodes(I_Scratch);
        SurvivingNodes.Reserve(NodeCount);
        UInt32 Culled = 0;
        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            if (Alive[i])
            {
                SurvivingNodes.EmplaceBack(std::move(Nodes[i]));
            }
            else
            {
                LOG_DEBUG("RenderGraph::Compile: culled dead pass '{}'.",
                          Nodes[i].Pass ? Nodes[i].Pass->GetName() : "null");
                ++Culled;
            }
        }
        Nodes.Clear();
        Nodes.Reserve(static_cast<UInt32>(SurvivingNodes.GetSize()));
        for (UInt64 i = 0; i < SurvivingNodes.GetSize(); ++i)
        { Nodes.PushBack(std::move(SurvivingNodes[i])); }
        if (Culled > 0) { LOG_DEBUG("RenderGraph::Compile: culled {} dead pass(es).", Culled); }
    }

    void FRenderGraph::
    TopologicalSort(std::pmr::memory_resource* I_Scratch)
    {
        const UInt32 NodeCount = static_cast<UInt32>(Nodes.GetSize());
        if (NodeCount <= 1) { return; }

        TPMRArray<TArray<UInt32>> Adjacency(I_Scratch);
        Adjacency.Resize(NodeCount);
        TPMRArray<UInt32> InDegree(I_Scratch);
        InDegree.Resize(NodeCount, 0);

        for (UInt32 Consumer = 0; Consumer < NodeCount; ++Consumer)
        {
            for (const auto& R : Nodes[Consumer].Reads)
            {
                for (UInt32 Producer = 0; Producer < NodeCount; ++Producer)
                {
                    if (Producer == Consumer) { continue; }
                    for (const auto& W : Nodes[Producer].Writes)
                    {
                        if (W == R)
                        {
                            Adjacency[Producer].EmplaceBack(Consumer);
                            ++InDegree[Consumer];
                        }
                    }
                }
            }
        }

        TPMRArray<UInt32> Queue(I_Scratch);
        Queue.Reserve(NodeCount);
        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            if (InDegree[i] == 0) { Queue.EmplaceBack(i); }
        }

        TPMRArray<UInt32> Sorted(I_Scratch);
        Sorted.Reserve(NodeCount);
        UInt32 Head = 0;
        while (Head < Queue.GetSize())
        {
            UInt32 Current = Queue[Head++];
            Sorted.EmplaceBack(Current);
            for (UInt32 Next : Adjacency[Current])
            {
                if (--InDegree[Next] == 0) { Queue.EmplaceBack(Next); }
            }
        }

        if (Sorted.GetSize() != NodeCount)
        { LOG_FATAL("RenderGraph::Compile: cycle detected in dependency graph!"); }
        VISERA_ASSERT(Sorted.GetSize() == NodeCount);

        TPMRArray<FRGNode> SortedNodes(I_Scratch);
        SortedNodes.Reserve(NodeCount);
        for (UInt32 Idx : Sorted)
        {
            SortedNodes.EmplaceBack(std::move(Nodes[Idx]));
        }
        Nodes.Clear();
        Nodes.Reserve(static_cast<UInt32>(SortedNodes.GetSize()));
        for (UInt64 i = 0; i < SortedNodes.GetSize(); ++i)
        { Nodes.PushBack(std::move(SortedNodes[i])); }
    }

    void FRenderGraph::
    ComputeBarrierFlags()
    {
        const UInt32 NodeCount = static_cast<UInt32>(Nodes.GetSize());
        NeedsBarrierBefore.Resize(NodeCount, 0);
        if (NodeCount <= 1) { return; }

        for (UInt32 i = 1; i < NodeCount; ++i)
        {
            for (const auto& R : Nodes[i].Reads)
            {
                for (UInt32 j = 0; j < i; ++j)
                {
                    for (const auto& W : Nodes[j].Writes)
                    {
                        if (W == R) { NeedsBarrierBefore[i] = 1; break; }
                    }
                    if (NeedsBarrierBefore[i]) { break; }
                }
                if (NeedsBarrierBefore[i]) { break; }
            }
        }
    }
}
