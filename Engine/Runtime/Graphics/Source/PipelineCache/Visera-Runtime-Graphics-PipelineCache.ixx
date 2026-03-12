module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.PipelineCache;
#define VISERA_MODULE_NAME "Runtime.Graphics"
       import Visera.Runtime.RHI;
       import Visera.Runtime.Graphics.Material;
       import Visera.Core.Containers.Array;
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

			for (const auto& Entry : Entries)
			{
				if (Entry.VertexShader   == VertHandle &&
				    Entry.FragmentShader == FragHandle &&
				    Entry.DepthFormat    == I_DepthFormat &&
				    Entry.ColorFormats.GetSize() == I_ColorFormats.GetSize())
				{
					Bool bMatch = True;
					for (UInt32 i = 0; i < I_ColorFormats.GetSize(); ++i)
					{
						if (Entry.ColorFormats[i] != I_ColorFormats[i])
						{ bMatch = False; break; }
					}
					if (bMatch) { return Entry.Pipeline; }
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

			Entries.PushBack(FCacheEntry{
				.VertexShader   = VertHandle,
				.FragmentShader = FragHandle,
				.ColorFormats   = ColorFormatsInline,
				.DepthFormat    = I_DepthFormat,
				.Pipeline       = Pipeline,
			});

			LOG_DEBUG("PipelineCache: compiled new PSO (vert={}, frag={}, colorFmts={}, depthFmt={}).",
			          VertHandle, FragHandle, I_ColorFormats.GetSize(), static_cast<UInt32>(I_DepthFormat));
			return Pipeline;
		}

	private:
		struct FCacheEntry
		{
			FRHIShaderHandle    VertexShader;
			FRHIShaderHandle    FragmentShader;
			
			TInlineArray<ERHIFormat, kMaxColorAttachments>
			ColorFormats;
			ERHIFormat
			DepthFormat {ERHIFormat::Undefined};
			FRHIRenderPassID
			Pipeline;
		};
		TArray<FCacheEntry> Entries;
	};
}
