# RHI (Render Hardware Interface)

The RHI layer abstracts the graphics API (e.g. Vulkan) and provides a stable interface for the rest of the engine: resource creation and lifecycle, command recording and submission, and swap chain / presentation.

## Scope

- **Public API**: Texture, buffer, sampler handles and create/destroy; command list recording; frame begin/end and present.
- **Backends**: Vulkan (primary); additional backends may be added later.
- **Ownership**: Engine module `Visera.Runtime.Global`; implementation under `Engine/Runtime/RHI/`.

## Key concepts

- **Handles**: `FRHITextureID`, `FRHIBufferID`, `FRHISamplerID`, `FRHIDescriptorSetID` — opaque IDs for resources managed by the RHI. Descriptor set **layout** is not exposed at the RHI API: the user provides `FRHIDescriptorSetCreateDesc` (bindings); RHI derives and caches the layout internally.
- **Descriptor sets**: Create a set with `CreateDescriptorSet(FRHIDescriptorSetCreateDesc)` — the create desc carries `Bindings` (binding, type, count, stages). RHI builds/caches the layout from bindings and returns `FRHIDescriptorSetID`. Update bindings via `UpdateDescriptorSet(handle, binding, ...)` overloads (texture, sampler, buffer). Destroy with `DestroyDescriptorSet(handle)` (transient or deferred).
- **Command list**: Record draw and transfer commands; submit once per frame (or as needed).
- **Frame lifecycle**: BeginFrame → record commands → Submit → EndFrame → Present.

## Usage (high level)

1. Obtain the global `FRHI` service (e.g. via `IGlobalService` / `EName::RHI`).
2. Create resources via `CreateTexture`, `CreateBuffer`, `CreateSampler`; destroy with the corresponding `Destroy*` when no longer needed.
3. For shader resources: fill `FRHIDescriptorSetCreateDesc` with `FRHIDescriptorSetBinding` entries, then `auto setHandle = CreateDescriptorSet(createDesc)`. Call `UpdateDescriptorSet(setHandle, binding, ...)` (texture/sampler/buffer overloads). Destroy with `DestroyDescriptorSet(setHandle)`. Layout is managed internally by RHI (cached by bindings hash).
4. Record commands into an `FRHICommandList` and call `Submit(I_CommandList)` before `EndFrame()`.
5. Call `Present()` after `EndFrame()` to present the swap chain (or omit in offscreen mode).

*(This document is maintained by the RHI Engineer agent; expand with API details, backend notes, and examples as the RHI evolves.)*
