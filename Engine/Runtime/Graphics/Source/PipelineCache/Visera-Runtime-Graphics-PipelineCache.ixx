module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.PipelineCache;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.Graphics.Material;
import Visera.Runtime.RHI;
import Visera.Core.Containers.Array;
import Visera.Core.Containers.Map;
import Visera.Core.Math.Hash.GoldenRatio;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_RUNTIME_API FPipelineCache
    {
    public:
        /** Get or create a PSO for the given material shaders + render state compiled against the specified render target formats. */
        [[nodiscard]] FRHIRenderPassID
        GetOrCreate(FRHI*                     I_RHI,
                    const FMaterial*          I_Material,
                    const TArray<ERHIFormat>& I_ColorFormats,
                    ERHIFormat                I_DepthFormat = ERHIFormat::Undefined)
        {
            const auto VertHandle = I_Material->GetVertexShader().GetHandle();
            const auto FragHandle = I_Material->GetFragmentShader().GetHandle();
            const auto MatCullMode    = I_Material->GetCullMode();
            const auto MatDepthTest   = I_Material->GetDepthTest();
            const auto MatZWrite      = I_Material->GetZWrite();
            const auto MatDepthCmpOp  = I_Material->GetDepthCompareOp();
            const UInt64 Key = ComputeKey(VertHandle, FragHandle, I_ColorFormats, I_DepthFormat,
                                          MatCullMode, MatDepthTest, MatZWrite, MatDepthCmpOp);

            auto It = HashIndex.Find(Key);
            if (It != HashIndex.end())
            {
                const UInt32 Idx = It->second;
                const auto& Entry = Entries[Idx];
                if (Entry.VertexShader == VertHandle &&
                    Entry.FragmentShader == FragHandle &&
                    Entry.DepthFormat == I_DepthFormat &&
                    Entry.CullMode == MatCullMode &&
                    Entry.bDepthTest == MatDepthTest &&
                    Entry.bZWrite == MatZWrite &&
                    Entry.DepthCompareOp == MatDepthCmpOp &&
                    MatchColorFormats(Entry.ColorFormats, I_ColorFormats))
                {
                    return Entry.Pipeline;
                }
            }

            TInlineArray<ERHIFormat, kMaxColorAttachments> ColorFormatsInline;
            for (const auto& F : I_ColorFormats) { ColorFormatsInline.PushBack(F); }
            VISERA_ASSERT(I_ColorFormats.GetSize() <= kMaxColorAttachments);

            FRHIRenderPassCreateInfo RPInfo
            {
                .VertexShader   = VertHandle,
                .FragmentShader = FragHandle,
                .PSO            = {
                    .Rasterization = { .CullMode = I_Material->GetCullMode() },
                    .DepthStencil  = {
                        .bEnableDepthTest  = I_Material->GetDepthTest(),
                        .bEnableDepthWrite = I_Material->GetZWrite(),
                        .DepthCompareOp    = I_Material->GetDepthCompareOp(),
                    },
                    .ColorFormats = ColorFormatsInline,
                    .DepthStencilFormat = I_DepthFormat,
                },
            };
            FRHIRenderPassID Pipeline = I_RHI->CreateRenderPass(std::move(RPInfo));

            const UInt32 NewIdx = static_cast<UInt32>(Entries.GetSize());
            Entries.PushBack(FCacheEntry{
                .VertexShader   = VertHandle,
                .FragmentShader = FragHandle,
                .ColorFormats   = ColorFormatsInline,
                .DepthFormat    = I_DepthFormat,
                .CullMode       = MatCullMode,
                .bDepthTest     = MatDepthTest,
                .bZWrite        = MatZWrite,
                .DepthCompareOp = MatDepthCmpOp,
                .Pipeline       = Pipeline,
            });
            HashIndex.InsertOrAssign(Key, NewIdx);

            LOG_DEBUG("PipelineCache: compiled new PSO (vert={}, frag={}, colorFmts={}, depthFmt={}).",
                      VertHandle, FragHandle, I_ColorFormats.GetSize(), static_cast<UInt32>(I_DepthFormat));
            return Pipeline;
        }

    private:
        struct FCacheEntry
        {
            FRHIShaderHandle    VertexShader;
            FRHIShaderHandle    FragmentShader;
            TInlineArray<ERHIFormat, kMaxColorAttachments> ColorFormats;
            ERHIFormat          DepthFormat    {ERHIFormat::Undefined};
            ERHICullMode        CullMode       {ERHICullMode::Back};
            Bool                bDepthTest     {False};
            Bool                bZWrite        {True};
            ERHICompareOp       DepthCompareOp {ERHICompareOp::LessOrEqual};
            FRHIRenderPassID    Pipeline;
        };

        TArray<FCacheEntry>     Entries;
        TMap<UInt64, UInt32>    HashIndex;   // Hash(key) -> Entries index

        /** Deterministic hash from shader handles + attachment formats + render state via GoldenRatio. */
        [[nodiscard]] static UInt64
        ComputeKey(const FRHIShaderHandle& I_Vert,
                   const FRHIShaderHandle& I_Frag,
                   const TArray<ERHIFormat>& I_ColorFormats,
                   ERHIFormat I_DepthFormat,
                   ERHICullMode  I_CullMode,
                   Bool          I_DepthTest,
                   Bool          I_ZWrite,
                   ERHICompareOp I_DepthCmpOp)
        {
            UInt64 Seed = Math::GoldenRatioHashCombine(0ULL,
                I_Vert.GetValue(), I_Frag.GetValue(), I_DepthFormat,
                I_CullMode, I_DepthTest, I_ZWrite, I_DepthCmpOp);
            for (UInt32 i = 0; i < I_ColorFormats.GetSize(); ++i)
            { Seed = Math::GoldenRatioHash(Seed, I_ColorFormats[i]); }
            return Seed;
        }

        [[nodiscard]] static Bool
        MatchColorFormats(const TInlineArray<ERHIFormat, kMaxColorAttachments>& I_Cached,
                          const TArray<ERHIFormat>& I_Requested)
        {
            if (I_Cached.GetSize() != I_Requested.GetSize()) { return False; }
            for (UInt32 i = 0; i < I_Requested.GetSize(); ++i)
            {
                if (I_Cached[i] != I_Requested[i]) { return False; }
            }
            return True;
        }
    };
}
