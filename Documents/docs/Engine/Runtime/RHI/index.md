# Runtime.RHI (Visera.Runtime.RHI)

**Runtime.RHI**(Render Hardware Interface)is the rendering hardware abstraction layer: wraps the graphics API (currently Vulkan); provides unified resource creation and lifecycle, command recording and submission, swap chain and present interface. the engine's graphics pipeline, materials and render graph work on top of RHI handles and command lists; no direct Vulkan API dependency. 

## Responsibilities
- **Resources**:textures, buffers, samplers, descriptor sets represented by handles (e.g. `FRHITextureHandle`, `FRHIBufferHandle`); create/destroy via RHI interface, descriptor set layout derived and cached by RHI from binding description. 
- **Commands**:record draw, dispatch, copy and barriers via `FRHICommandList`; submit at end of frame (Submit), then end frame and present (Present). 
- **Synchronization**:barriers manage resource layout and access; Vulkan backend uses Fence/Semaphore for CPU-GPU and GPU-GPU sync. 
- **Swap chain**:present target associated with window; RHI manages images and present queue. 

## Key concepts
- **Handle**:opaque ID for RHI-managed resources; does not expose underlying Vk objects. 
- **Descriptor set**:declare bindings (binding, type, count, stage) via `FRHIDescriptorSetCreateDesc`; RHI creates and caches layout; update via `UpdateDescriptorSet(handle, binding, ...)` texture/sampler/buffer overloads; destroy when done (transient or deferred). 
- **Frame lifecycle**:BeginFrame → record commands → Submit → EndFrame → Present (optional). 

## Submodules
| Module | Description |
|------|------|
| [Barrier](Barrier.md) | resource barriers (layout and access).  |
| [CommandList](CommandList.md) | command list recording and submission.  |
| [Resource](Resource/index.md) | resource types: Buffer, Texture, Sampler, DescriptorSet, Shader, RenderPass, ComputePass.  |
| [Registry](Registry/index.md) | handle registry and Item.  |
| [SwapChain](SwapChain.md) | swap chain and present.  |
| [Vulkan](Vulkan/index.md) | Vulkan backend: allocator, buffer/image, pipeline, descriptor, sync, etc..  |

## See also
- [Runtime](../index.md) — Parent module
- [Graphics](../Graphics/index.md) — graphics layer that uses RHI
- [Window](../Window/index.md) — window and swap chain association
