# Runtime.RHI.Resource.Texture (Visera.Runtime.RHI.Resource.Texture)

**Runtime.RHI.Resource.Texture** represents RHI texture resource: 1D/2D/3D or cubemap for texture, RTT or depth/stencil attachment. Created via RHI CreateTexture (or equivalent) and returns handle; Destroy on teardown. Shaders bind texture and sampler via descriptor set for sampling or write.

## Responsibilities
- Texture lifecycle managed by RHI; creation specifies size, format, usage (sample, render target, depth, etc.) and mip levels.
- With [DescriptorSet](DescriptorSet.md): bind texture handle and sampler to descriptor set binding for pipeline sampling or storage write.
- With [AssetHub.Image](../../AssetHub/Image/index.md): upload pixel data from image asset to texture (e.g. via StagingRing or temp buffer).

## See also
- [Resource](index.md) — Parent module
- [Vulkan.Image](../Vulkan/Image.md) — Vulkan image implementation
- [DescriptorSet](DescriptorSet.md) — Descriptor binding
- [AssetHub.Image](../../AssetHub/Image/index.md) — Image loading
