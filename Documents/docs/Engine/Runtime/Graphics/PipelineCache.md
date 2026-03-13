# Runtime.Graphics.PipelineCache (Visera.Runtime.Graphics.PipelineCache)

Pipeline cache for graphics pipelines: get-or-create PSO by material shaders and render target formats. Used when building the draw list so that renderables sharing the same material and formats reuse one pipeline.

## FPipelineCache

### GetOrCreate

**`GetOrCreate(I_RHI, I_Material, I_ColorFormats, I_DepthFormat)`** — Returns an existing or newly created render pass ID. Lookup is **hash-based O(1)** (key from vertex/fragment shader handles and color/depth formats via `Math::GoldenRatio`). On hash collision, a full key comparison ensures correctness; on miss, a new PSO is created and the entry is stored with its hash index.

## See also

- [Graphics](index.md) — parent module
- [RHI.Vulkan.Pipeline.Cache](../RHI/Vulkan/Pipeline/Cache.md) — Vulkan pipeline cache
- [RenderGraph](RenderGraph.md) — render graph
