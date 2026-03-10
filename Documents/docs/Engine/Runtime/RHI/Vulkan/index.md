# Runtime.RHI.Vulkan (Visera.Runtime.RHI.Vulkan)

**Runtime.RHI.Vulkan** is the RHI Vulkan backend: maps RHI handles, commands and sync to Vulkan VkImage, VkBuffer, VkCommandBuffer, VkPipeline. Handles Vulkan instance/device creation, queue selection, memory allocation (Allocator) and resource/pipeline create/destroy; sync via Fence and Semaphore.

## Responsibilities
- **Resources**: Allocator allocates device memory; Buffer, Image, Sampler, DescriptorSet/Layout, ShaderModule map to Vulkan; Pipeline (Compute/Render) with Cache and Layout manage pipeline state and descriptor layout.
- **Commands**: CommandPool allocates CommandBuffer; recording and submit via Vulkan queue submit.
- **Synchronization**: Sync module provides Fence (CPU waits GPU) and Semaphore (GPU–GPU) for frame sync and Present.
- **Loader**: Loader loads Vulkan entry points; Attachment defines color and depth attachment Vulkan descriptors.

## Submodules
| Module | Description |
|------|------|
| [Allocator](Allocator.md) | Device memory allocation. |
| [Buffer](Buffer.md), [Image](Image.md), [Sampler](Sampler.md) | Buffer, image, sampler. |
| [CommandPool](CommandPool.md), [CommandBuffer](CommandBuffer.md) | Command pool and command buffer. |
| [DescriptorPool](DescriptorPool.md), [DescriptorSet](DescriptorSet.md), [DescriptorSetLayout](DescriptorSetLayout.md) | Descriptor pool, set and layout. |
| [Pipeline](Pipeline/index.md) | Pipeline (Cache, Layout, Compute, Render). |
| [Sync](Sync/index.md) | [Fence](Sync/Fence.md), [Semaphore](Sync/Semaphore.md). |
| [Loader](Loader.md), [Common](Common.md), [Attachment](Attachment/index.md) | Loader, common types, attachments. |

## See also
- [RHI](../index.md) — Parent module
- [Resource](../Resource/index.md) — RHI resource abstraction
- [SwapChain](../SwapChain.md) — Swap chain and Present
