# Runtime.RHI.CommandList (Visera.Runtime.RHI.CommandList)

**Runtime.RHI.CommandList** provides command list type (e.g. FRHICommandList) to record all render and compute commands for a frame: draw calls, compute dispatch, resource copy and barriers. After recording, Submit at end of frame; after submit the list can be reset or discarded. Corresponds to Vulkan VkCommandBuffer; RHI abstraction allows backend switch or extended semantics.

## Responsibilities
- BeginRecording/EndRecording and draw, dispatch, copy, bind descriptor set, set pipeline APIs.
- **PushConstants**: Record push-constant data for the current pipeline. Use `PushConstants(const void* data, UInt32 offset, UInt32 size)` for raw bytes, or the template overload `PushConstants(const T& data, UInt32 offset = 0)` to push a struct by value (size and address are derived from `T`).
- Ensure command order and barriers: insert barriers before resource access; complete all submits before Present.
- Typically one or more command lists per frame, one submit; multi-threaded recording may use multiple lists then merge submit (implementation-dependent).

## See also
- [RHI](index.md) — Parent module
- [Barrier](Barrier.md) — Resource barriers
- [Vulkan.CommandBuffer](Vulkan/CommandBuffer.md) — Vulkan implementation
- [Graphics.RenderGraph](../Graphics/RenderGraph.md) — Render graph drives command recording
