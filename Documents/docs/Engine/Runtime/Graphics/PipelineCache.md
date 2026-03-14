# Runtime.Graphics.PipelineCache (Visera.Runtime.Graphics.PipelineCache)

Pipeline cache for graphics pipelines: get-or-create PSO by material shaders, render target formats, and material render state. Used when building the draw list so that renderables sharing the same configuration reuse one pipeline.

## FPipelineCache

### GetOrCreate

**`GetOrCreate(I_RHI, I_Material, I_ColorFormats, I_DepthFormat)`** — Returns an existing or newly created render pass ID.

**Cache key** includes:

- Vertex and fragment shader handles
- Color attachment formats
- Depth attachment format
- Cull mode (`ERHICullMode`)
- Depth test enable (`Bool`)
- Depth write enable (`Bool`)
- Depth compare op (`ERHICompareOp`)

Lookup is **hash-based O(1)** (key from all components via `Math::GoldenRatioHashCombine`). On hash collision, a full key comparison ensures correctness; on miss, a new PSO is created with the material's depth/stencil and cull state, and the entry is stored with its hash index.

Materials with different depth configurations (e.g. opaque with depth write vs. transparent with depth test only) produce distinct PSOs even if they share the same shaders and formats.

## See also

- [Graphics](index.md) — parent module
- [Material](Material.md) — material render state
- [RHI.Vulkan.Pipeline.Cache](../RHI/Vulkan/Pipeline/Cache.md) — Vulkan pipeline cache
- [RenderGraph](RenderGraph.md) — render graph
