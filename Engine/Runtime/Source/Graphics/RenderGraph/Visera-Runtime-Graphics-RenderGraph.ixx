module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.RenderGraph;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;
import Visera.Runtime.Graphics.Framework;
import Visera.Runtime.Graphics.PipelineCache;
import Visera.Runtime.Graphics.Scene;
import Visera.Core.Containers.Array;
import Visera.Core.Containers.Map;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Pointer;
import Visera.Core.Types.Function;
import Visera.Core.Types.Name;
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

    /** Well-known name for the back-buffer texture when present (headless has no BackBuffer). */
    inline const FName kBackBufferTextureName {"BackBuffer"};

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
    // ERGResourceUsage -- Describes how a pass accesses a resource
    // =========================================================================
    enum class ERGResourceUsage : UInt8
    {
        TransferSrc,
        TransferDst,
        ColorAttachment,
        DepthStencilAttachment,
        ShaderReadOnly,
        ShaderStorage,
        Present,
    };

    // =========================================================================
    // FRDGResourceAccess -- A resource + its intended usage within a pass
    // =========================================================================
    struct VISERA_RUNTIME_API FRDGResourceAccess
    {
        FGraphicsID       Resource;
        ERGResourceUsage  Usage;
    };

    // =========================================================================
    // FRDGPassContext -- Read-only context passed to pass execute lambdas
    // =========================================================================
    struct VISERA_RUNTIME_API FRDGPassContext
    {
        FRHICommandList&    CommandList;
        const FRenderList&  RenderList;
        const FRenderView&  RenderView;

        [[nodiscard]] const FRHITextureID&
        GetTexture(FGraphicsID I_ID) const;

        [[nodiscard]] const FRHIBufferID&
        GetBuffer(FGraphicsID I_ID) const;

        [[nodiscard]] FGraphicsID
        FindTexture(FName I_Name) const;

        /** Write user data to the engine-managed FrameData UBO (Set 0 Binding 0). */
        void
        WritePerFrameData(const void* I_Data, UInt32 I_Size);

        /** Returns the color attachment formats for this pass (declared at Setup, resolved at Compile). */
        [[nodiscard]] const TArray<ERHIFormat>&
        GetColorAttachmentFormats() const { return ActiveColorFormats; }

        /** Draw a standard render batch. Pipeline is resolved lazily from Material + active formats. */
        Bool
        DrawBatch(const FRenderBatch&               I_Batch,
                  const FRHIRenderPassAttachments&  I_Attachments);

        /** End the active render pass only when this pass context has begun one. */
        void
        EndRenderingIfActive();

    private:
        friend class FRenderGraph;
        const FRenderGraph*  Graph          {nullptr};
        FRHI*                RHI            {nullptr};
        FPipelineCache*      PipelineCache  {nullptr};
        FRHIBufferID         FrameDataUBO;
        FRHIDescriptorSetID  FrameDataDescriptorSet;
        FRHIDescriptorSetID  LightDescriptorSet;
        TArray<ERHIFormat>   ActiveColorFormats;
        ERHIFormat           ActiveDepthFormat {ERHIFormat::Undefined};
        Bool                 bRenderingActive  {False};
        void
        SetDeclaredColorFormats(const TArray<ERHIFormat>& I_ColorFormats, ERHIFormat I_DepthFormat);

        FRDGPassContext(FRHICommandList& I_CmdList, const FRenderGraph& I_Graph,
                        const FRenderList& I_RenderList, const FRenderView& I_RenderView,
                        FRHI* I_RHI, FPipelineCache* I_PipelineCache,
                        FRHIBufferID I_FrameDataUBO, FRHIDescriptorSetID I_FrameDataDescriptorSet,
                        FRHIDescriptorSetID I_LightDescriptorSet)
        : CommandList(I_CmdList), RenderList(I_RenderList), RenderView(I_RenderView),
          Graph(&I_Graph), RHI(I_RHI), PipelineCache(I_PipelineCache),
          FrameDataUBO(I_FrameDataUBO), FrameDataDescriptorSet(I_FrameDataDescriptorSet),
          LightDescriptorSet(I_LightDescriptorSet) {}
    };

    // =========================================================================
    // FRDGPassBuilder -- Scoped builder for declaring resource access in setup
    // =========================================================================
    class VISERA_RUNTIME_API FRDGPassBuilder
    {
    public:
        FRDGPassBuilder& Read(FGraphicsID I_Resource, ERGResourceUsage I_Usage)
        {
            Reads.EmplaceBack(FRDGResourceAccess{I_Resource, I_Usage});
            return *this;
        }

        FRDGPassBuilder& Write(FGraphicsID I_Resource, ERGResourceUsage I_Usage)
        {
            Writes.EmplaceBack(FRDGResourceAccess{I_Resource, I_Usage});
            return *this;
        }

        [[nodiscard]] FGraphicsID
        FindTexture(FName I_Name) const;

    private:
        friend class FRenderGraph;
        const FRenderGraph* OwnerGraph {nullptr};
        TArray<FRDGResourceAccess> Reads;
        TArray<FRDGResourceAccess> Writes;
    };

    // =========================================================================
    // FAutoBarrier -- A barrier computed during Compile, emitted during Execute
    // =========================================================================
    struct FAutoBarrier
    {
        FGraphicsID       Resource;
        ERHIImageLayout   OldLayout;
        ERHIImageLayout   NewLayout;
        FRHIMemoryBarrier MemoryBarrier;
    };

    // =========================================================================
    // FRDGNode -- Internal graph node (lambda-based, not class-based)
    // =========================================================================
    struct FRDGNode
    {
        FName                             Name;
        TArray<FRDGResourceAccess>        Reads;
        TArray<FRDGResourceAccess>        Writes;
        TFunction<void(FRDGPassContext&)> ExecuteFn;
    };

    // =========================================================================
    // FRDGTexture -- Transient or external texture managed by the RenderGraph
    // =========================================================================
    using ERGFormat = ERHIFormat;

    class VISERA_RUNTIME_API FRDGTexture
    {
    public:
        struct FCreateInfo
        {
            UInt32    Width  {0};
            UInt32    Height {0};
            ERGFormat Format {ERGFormat::Undefined};
        };

        FRDGTexture() = default;
        explicit FRDGTexture(FRHITextureID     I_Imported,
                            const FCreateInfo& I_CreateInfo,
                            ERHIImageLayout    I_InitialLayout)
            : CreateInfo(I_CreateInfo), RHIID(I_Imported), KnownLayout(I_InitialLayout), bIsExternal(True) {}
        explicit FRDGTexture(const FCreateInfo& I_CreateInfo)
            : CreateInfo(I_CreateInfo), RHIID{}, KnownLayout(ERHIImageLayout::Undefined), bIsExternal(False) {}

        [[nodiscard]] const FRHITextureID& GetRHIID()      const { return RHIID; }
        [[nodiscard]] Bool              IsExternal()    const { return bIsExternal; }
        [[nodiscard]] Bool              IsTransient()   const { return !bIsExternal; }
        [[nodiscard]] const FCreateInfo& GetCreateInfo() const { return CreateInfo; }
        [[nodiscard]] ERHIImageLayout   GetKnownLayout() const { return KnownLayout; }
        void SetRHIID(FRHITextureID I_ID) { RHIID = I_ID; }
        void SetKnownLayout(ERHIImageLayout I_Layout) { KnownLayout = I_Layout; }

    private:
        FCreateInfo    CreateInfo;
        FRHITextureID  RHIID;
        ERHIImageLayout KnownLayout {ERHIImageLayout::Undefined};
        Bool           bIsExternal {False};
    };

    using FRDGTextureCreateInfo = FRDGTexture::FCreateInfo;

    // =========================================================================
    // FRDGBuffer -- Transient or external buffer managed by the RenderGraph
    // =========================================================================
    class VISERA_RUNTIME_API FRDGBuffer
    {
    public:
        struct FCreateInfo
        {
            UInt64          Size   {0};
            ERHIBufferUsage Usages {ERHIBufferUsage::StorageBuffer};
        };

        FRDGBuffer() = default;
        explicit FRDGBuffer(FRHIBufferID I_Imported) : CreateInfo{}, RHIID(I_Imported), bIsExternal(True) {}
        explicit FRDGBuffer(const FCreateInfo& I_CreateInfo) : CreateInfo(I_CreateInfo), RHIID{}, bIsExternal(False) {}

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

    using FRDGBufferCreateInfo = FRDGBuffer::FCreateInfo;

    // =========================================================================
    // Barrier helpers (free functions, internal)
    // =========================================================================

    [[nodiscard]] constexpr ERHIImageLayout
    UsageToLayout(ERGResourceUsage I_Usage)
    {
        switch (I_Usage)
        {
        case ERGResourceUsage::TransferSrc:            return ERHIImageLayout::TransferSrc;
        case ERGResourceUsage::TransferDst:            return ERHIImageLayout::TransferDst;
        case ERGResourceUsage::ColorAttachment:        return ERHIImageLayout::ColorAttachment;
        case ERGResourceUsage::DepthStencilAttachment: return ERHIImageLayout::DepthStencilAttachment;
        case ERGResourceUsage::ShaderReadOnly:         return ERHIImageLayout::ShaderReadOnly;
        case ERGResourceUsage::ShaderStorage:          return ERHIImageLayout::General;
        case ERGResourceUsage::Present:                return ERHIImageLayout::Present;
        }
        return ERHIImageLayout::Undefined;
    }

    struct FBarrierStageAccess { ERHIPipelineStage Stage; ERHIAccessFlag Access; };

    [[nodiscard]] constexpr FBarrierStageAccess
    LayoutToSourceBarrier(ERHIImageLayout I_Layout)
    {
        switch (I_Layout)
        {
        case ERHIImageLayout::Undefined:              return { ERHIPipelineStage::TopOfPipe,              ERHIAccessFlag::None };
        case ERHIImageLayout::TransferSrc:            return { ERHIPipelineStage::Transfer,               ERHIAccessFlag::TransferRead };
        case ERHIImageLayout::TransferDst:            return { ERHIPipelineStage::Transfer,               ERHIAccessFlag::TransferWrite };
        case ERHIImageLayout::ColorAttachment:        return { ERHIPipelineStage::ColorAttachmentOutput,  ERHIAccessFlag::ColorAttachmentWrite };
        case ERHIImageLayout::DepthStencilAttachment: return { ERHIPipelineStage::EarlyFragmentTests | ERHIPipelineStage::LateFragmentTests,
                                                               ERHIAccessFlag::DepthStencilRead | ERHIAccessFlag::DepthStencilWrite };
        case ERHIImageLayout::ShaderReadOnly:         return { ERHIPipelineStage::FragmentShader,         ERHIAccessFlag::ShaderSampledRead };
        case ERHIImageLayout::General:                return { ERHIPipelineStage::ComputeShader,          ERHIAccessFlag::ShaderStorageRead | ERHIAccessFlag::ShaderStorageWrite };
        case ERHIImageLayout::Present:                return { ERHIPipelineStage::BottomOfPipe,           ERHIAccessFlag::None };
        default:                                      return { ERHIPipelineStage::AllCommands,             ERHIAccessFlag::MemoryRead | ERHIAccessFlag::MemoryWrite };
        }
    }

    [[nodiscard]] constexpr FBarrierStageAccess
    UsageToDestBarrier(ERGResourceUsage I_Usage)
    {
        switch (I_Usage)
        {
        case ERGResourceUsage::TransferSrc:            return { ERHIPipelineStage::Transfer,               ERHIAccessFlag::TransferRead };
        case ERGResourceUsage::TransferDst:            return { ERHIPipelineStage::Transfer,               ERHIAccessFlag::TransferWrite };
        case ERGResourceUsage::ColorAttachment:        return { ERHIPipelineStage::ColorAttachmentOutput,  ERHIAccessFlag::ColorAttachmentRead | ERHIAccessFlag::ColorAttachmentWrite };
        case ERGResourceUsage::DepthStencilAttachment: return { ERHIPipelineStage::EarlyFragmentTests | ERHIPipelineStage::LateFragmentTests,
                                                                ERHIAccessFlag::DepthStencilRead | ERHIAccessFlag::DepthStencilWrite };
        case ERGResourceUsage::ShaderReadOnly:         return { ERHIPipelineStage::FragmentShader,         ERHIAccessFlag::ShaderSampledRead };
        case ERGResourceUsage::ShaderStorage:          return { ERHIPipelineStage::ComputeShader,          ERHIAccessFlag::ShaderStorageRead | ERHIAccessFlag::ShaderStorageWrite };
        case ERGResourceUsage::Present:                return { ERHIPipelineStage::BottomOfPipe,           ERHIAccessFlag::None };
        }
        return { ERHIPipelineStage::AllCommands, ERHIAccessFlag::MemoryRead | ERHIAccessFlag::MemoryWrite };
    }

    // =========================================================================
    // FRenderGraph
    // =========================================================================

    class VISERA_RUNTIME_API FRenderGraph final
    {
    public:
        [[nodiscard]] FRHISwapChainID
        GetSwapChainID() const;

        [[nodiscard]] const FRHITextureID&
        GetTexture(FGraphicsID I_ID) const;

        [[nodiscard]] const FRHIBufferID&
        GetBuffer(FGraphicsID I_ID) const;

        // -----------------------------------------------------------------
        // Pass management (RDG-style: setup lambda + execute lambda)
        // -----------------------------------------------------------------

        void
        AddPass(FName                             I_Name,
                TFunction<void(FRDGPassBuilder&)> I_Setup,
                TFunction<void(FRDGPassContext&)> I_Execute);

        // -----------------------------------------------------------------
        // Resource management
        // -----------------------------------------------------------------

        [[nodiscard]] FGraphicsID
        CreateTexture(const FRDGTextureCreateInfo& I_CreateInfo);

        [[nodiscard]] FGraphicsID
        CreateTexture(FName I_Name, const FRDGTextureCreateInfo& I_CreateInfo);

        [[nodiscard]] FGraphicsID
        FindTexture(FName I_Name) const;

        [[nodiscard]] ERGFormat
        GetTextureFormat(FGraphicsID I_ID) const;

        [[nodiscard]] FGraphicsID
        RegisterExternalTexture(FName I_Name,
                                FRHITextureID I_Imported,
                                const FRDGTextureCreateInfo& I_CreateInfo = {},
                                ERHIImageLayout I_InitialLayout = ERHIImageLayout::Undefined);

        [[nodiscard]] const TMap<FName, FGraphicsID>&
        GetNamedTextures() const { return NamedTextures; }

        /** Valid after Compile(). True if the texture is referenced by at least one surviving pass. */
        [[nodiscard]] Bool
        IsTextureLive(FGraphicsID I_ID) const;

        /** Returns pointer to internal texture entry for I_ID, or nullptr if invalid. Used by FGraphics for cache export. */
        [[nodiscard]] const FRDGTexture*
        GetTextureEntry(FGraphicsID I_ID) const
        {
            if (!I_ID.IsTexture()) { return nullptr; }
            const UInt32 Index = I_ID.GetIndex();
            if (Index >= Textures.GetSize()) { return nullptr; }
            return &Textures[Index];
        }

        [[nodiscard]] FGraphicsID
        CreateBuffer(const FRDGBufferCreateInfo& I_CreateInfo);

        [[nodiscard]] FGraphicsID
        RegisterExternalBuffer(FRHIBufferID I_Imported);

        // -----------------------------------------------------------------
        // Compile / Execute
        // -----------------------------------------------------------------

        FRenderGraph*
        Compile(FRHI* I_RHI);

        void
        Execute(const FRenderContext* I_RenderContext);

        explicit FRenderGraph(FRHISwapChainID I_SwapChainID);
        FRenderGraph(FRHISwapChainID I_SwapChainID, FRHITextureID I_BackBuffer);

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
        FRHISwapChainID          SwapChainID;
        Memory::TMonotonicArena<kRenderGraphCompilingInlineMemory> CompileArena;
        TPMRArray<TArray<FAutoBarrier>>   PerNodeBarriers;
        TPMRArray<TArray<ERHIFormat>>     PerNodeColorFormats;
        TPMRArray<ERHIFormat>             PerNodeDepthFormat;
        TArray<FRDGNode>       Nodes;
        TArray<FRDGTexture>    Textures;
        TArray<FRDGBuffer>     Buffers;
        TArray<UInt8>         LiveTextures;
        TArray<UInt8>         LiveBuffers;
        TMap<FName, FGraphicsID> NamedTextures;

        [[nodiscard]] ERHIImageLayout
        GetTextureKnownLayout(FGraphicsID I_ID) const;

        void
        SetTextureKnownLayout(FGraphicsID I_ID, ERHIImageLayout I_Layout);

        PROFILING_ONLY_FIELD(
        static inline FRenderGraphCompileProfilingMetrics ProfilingMetrics;
        );

        void CullDeadPasses(std::pmr::memory_resource* I_Scratch);
        void TopologicalSort(std::pmr::memory_resource* I_Scratch);
        void ComputeLayoutTransitions();
    };

    void LogRenderGraphCompileProfilingSummary();

    // =================================================================
    // FRDGPassContext — out-of-line
    // =================================================================

    const FRHITextureID& FRDGPassContext::
    GetTexture(FGraphicsID I_ID) const { return Graph->GetTexture(I_ID); }

    const FRHIBufferID& FRDGPassContext::
    GetBuffer(FGraphicsID I_ID) const { return Graph->GetBuffer(I_ID); }

    void FRDGPassContext::
    WritePerFrameData(const void* I_Data, UInt32 I_Size)
    {
        if (!RHI || FrameDataUBO.IsNull() || !I_Data || I_Size == 0) { return; }
        RHI->WriteBufferDirect(FrameDataUBO, reinterpret_cast<const FByte*>(I_Data), I_Size);
    }

    void FRDGPassContext::
    SetDeclaredColorFormats(const TArray<ERHIFormat>& I_ColorFormats, ERHIFormat I_DepthFormat)
    {
        ActiveColorFormats = I_ColorFormats;
        ActiveDepthFormat  = I_DepthFormat;
    }

    Bool FRDGPassContext::
    DrawBatch(const FRenderBatch&               I_Batch,
              const FRHIRenderPassAttachments&  I_Attachments)
    {
        if (I_Batch.Instances.IsEmpty() || !I_Batch.Material || !PipelineCache) { return False; }

        FRHIRenderPassID Pipeline = PipelineCache->GetOrCreate(
            RHI, I_Batch.Material, ActiveColorFormats, ActiveDepthFormat);

        if (!bRenderingActive)
        {
            CommandList.BeginRendering(Pipeline, I_Attachments);
            bRenderingActive = True;
        }
        else
        {
            CommandList.BindPipeline(Pipeline);
        }

        if (!FrameDataDescriptorSet.IsNull())
        { CommandList.BindDescriptorSet(FrameDataDescriptorSet, kFrameDataDescriptorSet); }
        CommandList.BindDescriptorSet(I_Batch.MaterialDescriptorSet,  kMaterialDescriptorSet);
        if (!LightDescriptorSet.IsNull())
        { CommandList.BindDescriptorSet(LightDescriptorSet,           kLightDataDescriptorSet); }
        CommandList.BindDescriptorSet(I_Batch.InstanceDescriptorSet,  kInstanceDataDescriptorSet);

        FViseraPushConstants PushConstants { .InstanceOffset = I_Batch.InstanceStartIndex };
        CommandList.PushConstants(PushConstants);

        const UInt32 InstanceCount = static_cast<UInt32>(I_Batch.Instances.GetSize());
        if (I_Batch.Mesh)
        {
            CommandList.BindIndexBuffer(I_Batch.Mesh->IndexBuffer, I_Batch.Mesh->IndexType);
            CommandList.DrawIndexed(I_Batch.Mesh->IndexCount, InstanceCount);
        }
        else
        {
            CommandList.Draw(6, InstanceCount);
        }

        return True;
    }

    void FRDGPassContext::
    EndRenderingIfActive()
    {
        if (!bRenderingActive) { return; }
        CommandList.EndRendering();
        bRenderingActive = False;
    }

    FGraphicsID FRDGPassContext::
    FindTexture(FName I_Name) const { return Graph->FindTexture(I_Name); }

    // =================================================================
    // FRDGPassBuilder — out-of-line
    // =================================================================

    FGraphicsID FRDGPassBuilder::
    FindTexture(FName I_Name) const
    { return OwnerGraph ? OwnerGraph->FindTexture(I_Name) : FGraphicsID::Invalid(); }

    // =================================================================
    // FRenderGraph::AddPass
    // =================================================================

    void FRenderGraph::
    AddPass(FName                             I_Name,
            TFunction<void(FRDGPassBuilder&)> I_Setup,
            TFunction<void(FRDGPassContext&)> I_Execute)
    {
        FRDGPassBuilder Builder;
        Builder.OwnerGraph = this;
        I_Setup(Builder);
        Nodes.EmplaceBack(FRDGNode{
            .Name      = std::move(I_Name),
            .Reads     = std::move(Builder.Reads),
            .Writes    = std::move(Builder.Writes),
            .ExecuteFn = std::move(I_Execute),
        });
        LOG_TRACE("RenderGraph: added pass '{}'.", Nodes.Back().Name.GetNameString());
    }

    // =================================================================
    // FRenderGraph — out-of-line definitions
    // =================================================================

    FRHISwapChainID FRenderGraph::
    GetSwapChainID() const { return SwapChainID; }

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

    Bool FRenderGraph::
    IsTextureLive(FGraphicsID I_ID) const
    {
        if (!I_ID.IsTexture()) { return False; }
        const UInt32 Index = I_ID.GetIndex();
        return Index < LiveTextures.GetSize() && LiveTextures[Index] != 0;
    }

    FGraphicsID FRenderGraph::
    CreateTexture(const FRDGTextureCreateInfo& I_CreateInfo)
    {
        Textures.EmplaceBack(FRDGTexture(I_CreateInfo));
        return FGraphicsID::Texture(static_cast<UInt32>(Textures.GetSize() - 1));
    }

    FGraphicsID FRenderGraph::
    CreateTexture(FName I_Name, const FRDGTextureCreateInfo& I_CreateInfo)
    {
        auto It = NamedTextures.Find(I_Name);
        if (It != NamedTextures.end())
        {
            const auto* Existing = GetTextureEntry(It->second);
            if (!Existing)
            {
                LOG_ERROR("RenderGraph::CreateTexture('{}'): existing named texture handle is invalid.",
                          I_Name.GetNameString());
                return FGraphicsID::Invalid();
            }
            const auto& ExistingInfo = Existing->GetCreateInfo();
            if (ExistingInfo.Width  != I_CreateInfo.Width  ||
                ExistingInfo.Height != I_CreateInfo.Height ||
                ExistingInfo.Format != I_CreateInfo.Format)
            {
                LOG_ERROR("RenderGraph::CreateTexture('{}'): descriptor mismatch (existing={}x{} fmt={}, requested={}x{} fmt={}).",
                          I_Name.GetNameString(),
                          ExistingInfo.Width, ExistingInfo.Height, static_cast<UInt32>(ExistingInfo.Format),
                          I_CreateInfo.Width, I_CreateInfo.Height, static_cast<UInt32>(I_CreateInfo.Format));
                return FGraphicsID::Invalid();
            }
            return It->second;
        }
        auto ID = CreateTexture(I_CreateInfo);
        NamedTextures.Insert(std::move(I_Name), ID);
        return ID;
    }

    FGraphicsID FRenderGraph::
    FindTexture(FName I_Name) const
    {
        auto It = NamedTextures.Find(I_Name);
        if (It != NamedTextures.end()) { return It->second; }
        return FGraphicsID::Invalid();
    }

    ERGFormat FRenderGraph::
    GetTextureFormat(FGraphicsID I_ID) const
    {
        if (!I_ID.IsTexture()) { return ERGFormat::Undefined; }
        const UInt32 Index = I_ID.GetIndex();
        if (Index >= Textures.GetSize()) { return ERGFormat::Undefined; }
        return Textures[Index].GetCreateInfo().Format;
    }

    ERHIImageLayout FRenderGraph::
    GetTextureKnownLayout(FGraphicsID I_ID) const
    {
        if (!I_ID.IsTexture()) { return ERHIImageLayout::Undefined; }
        const UInt32 Index = I_ID.GetIndex();
        if (Index >= Textures.GetSize()) { return ERHIImageLayout::Undefined; }
        return Textures[Index].GetKnownLayout();
    }

    void FRenderGraph::
    SetTextureKnownLayout(FGraphicsID I_ID, ERHIImageLayout I_Layout)
    {
        if (!I_ID.IsTexture()) { return; }
        const UInt32 Index = I_ID.GetIndex();
        if (Index >= Textures.GetSize()) { return; }
        Textures[Index].SetKnownLayout(I_Layout);
    }

    FGraphicsID FRenderGraph::
    RegisterExternalTexture(FName I_Name,
                            FRHITextureID I_Imported,
                            const FRDGTextureCreateInfo& I_CreateInfo,
                            ERHIImageLayout I_InitialLayout)
    {
        auto It = NamedTextures.Find(I_Name);
        if (It != NamedTextures.end())
        {
            const auto* Existing = GetTextureEntry(It->second);
            if (!Existing)
            {
                LOG_ERROR("RenderGraph::RegisterExternalTexture('{}'): existing named texture handle is invalid.",
                          I_Name.GetNameString());
                return FGraphicsID::Invalid();
            }
            if (Existing->GetRHIID().GetHandle() == I_Imported.GetHandle()) { return It->second; }
            const auto& ExistingInfo = Existing->GetCreateInfo();
            if (ExistingInfo.Width  != I_CreateInfo.Width  ||
                ExistingInfo.Height != I_CreateInfo.Height ||
                ExistingInfo.Format != I_CreateInfo.Format)
            {
                LOG_ERROR("RenderGraph::RegisterExternalTexture('{}'): descriptor mismatch with existing named texture.",
                          I_Name.GetNameString());
            }
            else
            {
                LOG_ERROR("RenderGraph::RegisterExternalTexture('{}'): name already bound to a different external texture.",
                          I_Name.GetNameString());
            }
            return FGraphicsID::Invalid();
        }
        if (I_Imported.IsNull()) { return FGraphicsID::Invalid(); }
        Textures.EmplaceBack(FRDGTexture(I_Imported, I_CreateInfo, I_InitialLayout));
        const FGraphicsID ID = FGraphicsID::Texture(static_cast<UInt32>(Textures.GetSize() - 1));
        NamedTextures.Insert(std::move(I_Name), ID);
        return ID;
    }

    FGraphicsID FRenderGraph::
    CreateBuffer(const FRDGBufferCreateInfo& I_CreateInfo)
    {
        Buffers.EmplaceBack(FRDGBuffer(I_CreateInfo));
        return FGraphicsID::Buffer(static_cast<UInt32>(Buffers.GetSize() - 1));
    }

    FGraphicsID FRenderGraph::
    RegisterExternalBuffer(FRHIBufferID I_Imported)
    {
        if (I_Imported.IsNull()) { return FGraphicsID::Invalid(); }
        Buffers.EmplaceBack(FRDGBuffer(I_Imported));
        return FGraphicsID::Buffer(static_cast<UInt32>(Buffers.GetSize() - 1));
    }

    // =================================================================
    // Compile
    // =================================================================

    FRenderGraph* FRenderGraph::
    Compile(FRHI* I_RHI)
    {
        if (!I_RHI) { return this; }

        std::pmr::memory_resource* const Scratch = &CompileArena.Get();
        const UInt32 NodeCountBefore = static_cast<UInt32>(Nodes.GetSize());
        CullDeadPasses(Scratch);
        const UInt32 SurvivingCount = static_cast<UInt32>(Nodes.GetSize());
        TopologicalSort(Scratch);

        LiveTextures.Resize(Textures.GetSize(), 0);
        LiveBuffers.Resize(Buffers.GetSize(), 0);
        for (const auto& Node : Nodes)
        {
            for (const auto& A : Node.Reads)
            {
                if (A.Resource.IsTexture() && A.Resource.GetIndex() < LiveTextures.GetSize()) { LiveTextures[A.Resource.GetIndex()] = 1; }
                if (A.Resource.IsBuffer()  && A.Resource.GetIndex() < LiveBuffers.GetSize())  { LiveBuffers[A.Resource.GetIndex()]  = 1; }
            }
            for (const auto& A : Node.Writes)
            {
                if (A.Resource.IsTexture() && A.Resource.GetIndex() < LiveTextures.GetSize()) { LiveTextures[A.Resource.GetIndex()] = 1; }
                if (A.Resource.IsBuffer()  && A.Resource.GetIndex() < LiveBuffers.GetSize())  { LiveBuffers[A.Resource.GetIndex()]  = 1; }
            }
        }

        for (UInt32 Index = 0; Index < Textures.GetSize(); ++Index)
        {
            if (Index >= LiveTextures.GetSize() || LiveTextures[Index] == 0) { continue; }
            auto& Texture = Textures[Index];
            if (!Texture.IsTransient()) { continue; }
            const auto& Info = Texture.GetCreateInfo();
            if (Info.Width == 0 || Info.Height == 0 || Info.Format == ERGFormat::Undefined) { continue; }
            FRHITextureCreateInfo RHIInfo{
                .Width    = Info.Width,
                .Height   = Info.Height,
                .Depth    = 1,
                .Format   = Info.Format,
                .Type     = ERHIImageType::Image2D,
                .Usages   = ERHIImageUsage::RenderTarget | ERHIImageUsage::ShaderResource | ERHIImageUsage::TransferSrc | ERHIImageUsage::TransferDst,
                .ViewType = ERHIImageViewType::Image2D,
            };
            Texture.SetRHIID(I_RHI->CreateTexture(std::move(RHIInfo)));
            Texture.SetKnownLayout(ERHIImageLayout::Undefined);
            LOG_TRACE("RenderGraph::Compile: allocated transient texture ({}x{}).",
                      Info.Width, Info.Height);
        }

        for (UInt32 Index = 0; Index < Buffers.GetSize(); ++Index)
        {
            if (Index >= LiveBuffers.GetSize() || LiveBuffers[Index] == 0) { continue; }
            auto& Buffer = Buffers[Index];
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

        ComputeLayoutTransitions();

        PerNodeColorFormats.Resize(Nodes.GetSize());
        PerNodeDepthFormat.Resize(Nodes.GetSize(), ERHIFormat::Undefined);
        for (UInt32 NodeIndex = 0; NodeIndex < Nodes.GetSize(); ++NodeIndex)
        {
            auto& ColorFormats = PerNodeColorFormats[NodeIndex];
            ColorFormats.Clear();
            for (const auto& Write : Nodes[NodeIndex].Writes)
            {
                if (Write.Usage == ERGResourceUsage::ColorAttachment && Write.Resource.IsTexture())
                {
                    const ERHIFormat Format = GetTextureFormat(Write.Resource);
                    if (Format != ERHIFormat::Undefined)
                    { ColorFormats.EmplaceBack(Format); }
                }
                if (Write.Usage == ERGResourceUsage::DepthStencilAttachment && Write.Resource.IsTexture())
                {
                    PerNodeDepthFormat[NodeIndex] = GetTextureFormat(Write.Resource);
                    break;
                }
            }
        }

        PROFILING_ONLY_FIELD(
        ++ProfilingMetrics.TotalCompiles;
        ProfilingMetrics.TotalCulled += (NodeCountBefore - SurvivingCount);
        if (NodeCountBefore > ProfilingMetrics.PeakNodes) { ProfilingMetrics.PeakNodes = NodeCountBefore; }
        {
            const std::size_t ArenaUsed = CompileArena.GetTotalBytesUsed();
            if (static_cast<UInt64>(ArenaUsed) > ProfilingMetrics.PeakArenaBytesUsed)
            { ProfilingMetrics.PeakArenaBytesUsed = static_cast<UInt64>(ArenaUsed); }
            ProfilingMetrics.ArenaSpillCount += static_cast<UInt64>(CompileArena.GetOverflowBlockCount());
        }
        )

        LOG_TRACE("RenderGraph::Compile: {} nodes, {} textures, {} buffers.",
                  Nodes.GetSize(), Textures.GetSize(), Buffers.GetSize());
        return this;
    }

    // =================================================================
    // Execute
    // =================================================================

    void FRenderGraph::
    Execute(const FRenderContext* I_RenderContext)
    {
        if (!I_RenderContext || !I_RenderContext->RHI) { return; }
        VISERA_ASSERT(I_RenderContext->RenderList != nullptr && "Execute requires a valid RenderList.");
        VISERA_ASSERT(I_RenderContext->RenderView != nullptr && "Execute requires a valid RenderView.");
        auto CmdList = I_RenderContext->RHI->CreateCommandList();
        FRDGPassContext Ctx(CmdList, *this, *I_RenderContext->RenderList, *I_RenderContext->RenderView,
                            I_RenderContext->RHI, I_RenderContext->PipelineCache,
                            I_RenderContext->FrameDataUBO, I_RenderContext->FrameDataDescriptorSet,
                            I_RenderContext->LightDescriptorSet);
        for (UInt32 i = 0; i < Nodes.GetSize(); ++i)
        {
            if (i < PerNodeBarriers.GetSize())
            {
                for (const auto& B : PerNodeBarriers[i])
                {
                    CmdList.TransitionTexture(FRHIImageBarrier{
                        .Image         = GetTexture(B.Resource),
                        .OldLayout     = B.OldLayout,
                        .NewLayout     = B.NewLayout,
                        .MemoryBarrier = B.MemoryBarrier,
                    });
                }
            }

            const auto& Node = Nodes[i];
            if (Node.ExecuteFn)
            {
                if (i < PerNodeColorFormats.GetSize())
                {
                    Ctx.SetDeclaredColorFormats(PerNodeColorFormats[i],
                        i < PerNodeDepthFormat.GetSize() ? PerNodeDepthFormat[i] : ERHIFormat::Undefined);
                }
                LOG_TRACE("RenderGraph::Execute: running pass '{}'.", Node.Name.GetNameString());
                Node.ExecuteFn(Ctx);
            }
        }
        I_RenderContext->RHI->Submit(std::move(CmdList));
    }

    // =================================================================
    // Constructors
    // =================================================================

    FRenderGraph::
    FRenderGraph(FRHISwapChainID I_SwapChainID)
        : SwapChainID(I_SwapChainID)
        , CompileArena()
        , PerNodeBarriers(&CompileArena.Get())
        , PerNodeColorFormats(&CompileArena.Get())
        , PerNodeDepthFormat(&CompileArena.Get())
    {}

    FRenderGraph::
    FRenderGraph(FRHISwapChainID I_SwapChainID, FRHITextureID I_BackBuffer)
        : SwapChainID(I_SwapChainID)
        , CompileArena()
        , PerNodeBarriers(&CompileArena.Get())
        , PerNodeColorFormats(&CompileArena.Get())
        , PerNodeDepthFormat(&CompileArena.Get())
    {
        if (!I_BackBuffer.IsNull())
        { (void)RegisterExternalTexture(kBackBufferTextureName, I_BackBuffer, {}, ERHIImageLayout::Undefined); }
    }

    // =================================================================
    // CullDeadPasses
    // =================================================================

    /** Removes passes whose outputs are never consumed by any live pass.
     *  Algorithm: O(N+E) reverse BFS instead of the previous O(N^3) fixed-point iteration.
     *    1. Build a ResourceWriters map (resource -> list of node indices that write it).
     *    2. Seed the alive set with nodes writing to external resources (backbuffer, cached textures).
     *    3. BFS backwards: for each alive node's reads, mark all writers of that resource alive.
     *  Passes not reached by the BFS are dead and removed from the graph. */
    void FRenderGraph::
    CullDeadPasses(std::pmr::memory_resource* I_Scratch)
    {
        const UInt32 NodeCount = static_cast<UInt32>(Nodes.GetSize());
        if (NodeCount == 0) { return; }

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

        // Step 1: resource ID -> writer node indices (uses FGraphicsID::Value as slot index).
        TPMRArray<TArray<UInt32>> ResourceWriters(I_Scratch);
        auto EnsureSlot = [&ResourceWriters](UInt32 I_ResourceIndex)
        {
            while (ResourceWriters.GetSize() <= I_ResourceIndex)
            { ResourceWriters.EmplaceBack(); }
        };
        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            for (const auto& W : Nodes[i].Writes)
            {
                if (!W.Resource.IsValid()) { continue; }
                const UInt32 Slot = W.Resource.Value;
                EnsureSlot(Slot);
                ResourceWriters[Slot].EmplaceBack(i);
            }
        }

        // Step 2: seed — any node that writes to an external resource is alive.
        TPMRArray<UInt8> Alive(I_Scratch);
        Alive.Resize(NodeCount, 0);
        TPMRArray<UInt32> Queue(I_Scratch);
        Queue.Reserve(NodeCount);

        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            for (const auto& W : Nodes[i].Writes)
            {
                if (IsExternalResource(W.Resource))
                { Alive[i] = True; Queue.EmplaceBack(i); break; }
            }
        }

        // Step 3: propagate liveness backwards through read dependencies.
        UInt32 Head = 0;
        while (Head < Queue.GetSize())
        {
            const UInt32 AliveNode = Queue[Head++];
            for (const auto& R : Nodes[AliveNode].Reads)
            {
                if (!R.Resource.IsValid()) { continue; }
                const UInt32 Slot = R.Resource.Value;
                if (Slot >= ResourceWriters.GetSize()) { continue; }
                for (UInt32 Writer : ResourceWriters[Slot])
                {
                    if (!Alive[Writer]) { Alive[Writer] = 1; Queue.EmplaceBack(Writer); }
                }
            }
        }

        TPMRArray<FRDGNode> SurvivingNodes(I_Scratch);
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
                LOG_DEBUG("RenderGraph::Compile: culled dead pass '{}'.", Nodes[i].Name.GetNameString());
                ++Culled;
            }
        }
        Nodes.Clear();
        Nodes.Reserve(static_cast<UInt32>(SurvivingNodes.GetSize()));
        for (UInt64 i = 0; i < SurvivingNodes.GetSize(); ++i)
        { Nodes.PushBack(std::move(SurvivingNodes[i])); }
        if (Culled > 0) { LOG_DEBUG("RenderGraph::Compile: culled {} dead pass(es).", Culled); }
    }

    // =================================================================
    // TopologicalSort
    // TODO: Deduplicate edges in adjacency list to avoid inflated InDegree
    //       when multiple producers write the same resource.
    // =================================================================

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
                Bool bInPlace = False;
                for (const auto& W : Nodes[Consumer].Writes)
                {
                    if (W.Resource == R.Resource) { bInPlace = True; break; }
                }

                if (bInPlace)
                {
                    for (Int32 Producer = static_cast<Int32>(Consumer) - 1; Producer >= 0; --Producer)
                    {
                        Bool bFound = False;
                        for (const auto& W : Nodes[static_cast<UInt32>(Producer)].Writes)
                        {
                            if (W.Resource == R.Resource)
                            {
                                Adjacency[static_cast<UInt32>(Producer)].EmplaceBack(Consumer);
                                ++InDegree[Consumer];
                                bFound = True;
                                break;
                            }
                        }
                        if (bFound) { break; }
                    }
                }
                else
                {
                    for (UInt32 Producer = 0; Producer < NodeCount; ++Producer)
                    {
                        if (Producer == Consumer) { continue; }
                        for (const auto& W : Nodes[Producer].Writes)
                        {
                            if (W.Resource == R.Resource)
                            {
                                Adjacency[Producer].EmplaceBack(Consumer);
                                ++InDegree[Consumer];
                            }
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

        TPMRArray<FRDGNode> SortedNodes(I_Scratch);
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

    // =================================================================
    // ComputeLayoutTransitions
    // =================================================================

    void FRenderGraph::
    ComputeLayoutTransitions()
    {
        const UInt32 NodeCount = static_cast<UInt32>(Nodes.GetSize());
        PerNodeBarriers.Resize(NodeCount);

        for (UInt32 i = 0; i < NodeCount; ++i)
        {
            auto& Barriers = PerNodeBarriers[i];
            Barriers.Clear();

            auto ProcessAccess = [&](const FRDGResourceAccess& Access)
            {
                if (!Access.Resource.IsTexture()) { return; }

                const ERHIImageLayout Required = UsageToLayout(Access.Usage);
                const ERHIImageLayout Current  = GetTextureKnownLayout(Access.Resource);

                if (Current != Required)
                {
                    const auto Src = LayoutToSourceBarrier(Current);
                    const auto Dst = UsageToDestBarrier(Access.Usage);

                    Barriers.EmplaceBack(FAutoBarrier{
                        .Resource      = Access.Resource,
                        .OldLayout     = Current,
                        .NewLayout     = Required,
                        .MemoryBarrier = FRHIMemoryBarrier
                        {
                            .SourceStage  = Src.Stage,
                            .DestStage    = Dst.Stage,
                            .SourceAccess = Src.Access,
                            .DestAccess   = Dst.Access,
                        },
                    });

                    SetTextureKnownLayout(Access.Resource, Required);
                }
            };

            for (const auto& R : Nodes[i].Reads)  { ProcessAccess(R); }
            for (const auto& W : Nodes[i].Writes) { ProcessAccess(W); }
        }
    }
}
