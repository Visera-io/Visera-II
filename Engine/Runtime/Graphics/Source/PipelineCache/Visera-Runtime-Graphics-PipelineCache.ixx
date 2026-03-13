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
        /** Get or create a PSO for the given material shaders compiled against the specified render target formats. */
        [[nodiscard]] FRHIRenderPassID
        GetOrCreate(FRHI*                     I_RHI,
                    const FMaterial*          I_Material,
                    const TArray<ERHIFormat>& I_ColorFormats,
                    ERHIFormat                I_DepthFormat = ERHIFormat::Undefined)
        {
            const auto VertHandle = I_Material->GetVertexShader().GetHandle();
            const auto FragHandle = I_Material->GetFragmentShader().GetHandle();
            // Hash-based O(1) lookup; falls through to creation on miss or collision.
            const UInt64 Key = ComputeKey(VertHandle, FragHandle, I_ColorFormats, I_DepthFormat);

            auto It = HashIndex.Find(Key);
            if (It != HashIndex.end())
            {
                // Full key comparison guards against hash collisions.
                const UInt32 Idx = It->second;
                const auto& Entry = Entries[Idx];
                if (Entry.VertexShader == VertHandle &&
                    Entry.FragmentShader == FragHandle &&
                    Entry.DepthFormat == I_DepthFormat &&
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
                .PSO            = { .ColorFormats = ColorFormatsInline, .DepthStencilFormat = I_DepthFormat },
            };
            FRHIRenderPassID Pipeline = I_RHI->CreateRenderPass(std::move(RPInfo));

            const UInt32 NewIdx = static_cast<UInt32>(Entries.GetSize());
            Entries.PushBack(FCacheEntry{
                .VertexShader   = VertHandle,
                .FragmentShader = FragHandle,
                .ColorFormats   = ColorFormatsInline,
                .DepthFormat    = I_DepthFormat,
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
            ERHIFormat          DepthFormat {ERHIFormat::Undefined};
            FRHIRenderPassID    Pipeline;
        };

        TArray<FCacheEntry>     Entries;
        TMap<UInt64, UInt32>    HashIndex;   // Hash(key) -> Entries index

        /** Deterministic hash from shader handles + attachment formats via GoldenRatio. */
        [[nodiscard]] static UInt64
        ComputeKey(const FRHIShaderHandle& I_Vert,
                   const FRHIShaderHandle& I_Frag,
                   const TArray<ERHIFormat>& I_ColorFormats,
                   ERHIFormat I_DepthFormat)
        {
            UInt64 Seed = Math::GoldenRatioHashCombine(0ULL,
                I_Vert.GetValue(), I_Frag.GetValue(), I_DepthFormat);
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
