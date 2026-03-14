# Runtime.RHI.Resource.RenderPass (Visera.Runtime.RHI.Resource.RenderPass)

Render pass resource: pipeline state object (PSO) for dynamic rendering. Uses Vulkan's `VK_KHR_dynamic_rendering` — no legacy `VkRenderPass` or `VkFramebuffer` objects.

## FRHIRenderPassState

Configuration baked into the PSO at creation time.

### DepthStencil

| Field       | Type            | Description |
|-------------|-----------------|-------------|
| `bEnable`   | `Bool`          | Enable depth testing. |
| `bWrite`    | `Bool`          | Enable depth writes. |
| `CompareOp` | `ERHICompareOp` | Depth comparison operation (Less, LessOrEqual, etc.). |

The depth/stencil state is part of the PSO key in `FPipelineCache`, ensuring materials with different depth configurations produce separate PSOs.

## See also

- [Resource](index.md) — parent module
- [Common](../Common.md) — ERHICompareOp
- [Vulkan.Pipeline.Render](../Vulkan/Pipeline/Render.md) — render pipeline
- [Graphics.PipelineCache](../../Graphics/PipelineCache.md) — consumes render pass state
- [Graphics.RenderGraph](../../Graphics/RenderGraph.md) — render graph
- [Barrier](../Barrier.md) — barriers
