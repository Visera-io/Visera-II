# Runtime.RHI.Vulkan.DescriptorPool (Visera.Runtime.RHI.Vulkan.DescriptorPool)

Vulkan descriptor pool. Allocates descriptor sets.

## FVulkanDescriptorPool

Created by `FVulkanDriver::CreateDescriptorPool` with a fixed capacity of **4096 max sets** and per-type pool sizes (SampledImage: 1024, UniformBuffer: 512, StorageBuffer: 128, Sampler: 256).

The pool is created with `VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT`, allowing individual descriptor sets to be freed back to the pool.

### MaxSets capacity

!!! warning "Not `maxBoundDescriptorSets`"
    The pool's `maxSets` must be the total number of descriptor sets that can be **allocated** from the pool, not `VkPhysicalDeviceLimits::maxBoundDescriptorSets` (which is the maximum number of sets that can be **bound to a pipeline simultaneously** — typically 8 on Apple Silicon). Using `maxBoundDescriptorSets` as pool capacity will exhaust the pool almost immediately when per-frame instance descriptor sets are created.

### Contract

For every `FVulkanDescriptorSetLayout` that allocates from this pool:

- For each descriptor type, the sum of `descriptorCount` across bindings of that type must not exceed the pool's corresponding `VkDescriptorPoolSize::descriptorCount`.
- The total number of allocated sets must not exceed `maxSets`.

### CreateDescriptorSet

**`CreateDescriptorSet(I_DescriptorSetLayout)`** — Allocates one descriptor set from the pool. Decrements `AvailableSets`; asserts if the pool is exhausted.

## See also

- [Vulkan](index.md) — parent module
- [DescriptorSet](DescriptorSet.md) — descriptor set
- [DescriptorSetLayout](DescriptorSetLayout.md) — layout
