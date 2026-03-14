# Runtime.RHI.Common (Visera.Runtime.RHI.Common)

RHI common types and enums shared across the RHI layer.

## Enums

### ERHIIndexType

Index buffer element type. Used by `BindIndexBuffer`.

| Value    | Vulkan mapping       |
|----------|---------------------|
| `UInt16` | `VK_INDEX_TYPE_UINT16` |
| `UInt32` | `VK_INDEX_TYPE_UINT32` |

### ERHICompareOp

Comparison operation for depth/stencil testing. Used by `FRHIRenderPassState::DepthStencil`.

| Value          | Vulkan mapping |
|----------------|---------------|
| `Never`        | `VK_COMPARE_OP_NEVER` |
| `Less`         | `VK_COMPARE_OP_LESS` |
| `Equal`        | `VK_COMPARE_OP_EQUAL` |
| `LessOrEqual`  | `VK_COMPARE_OP_LESS_OR_EQUAL` |
| `Greater`      | `VK_COMPARE_OP_GREATER` |
| `NotEqual`     | `VK_COMPARE_OP_NOT_EQUAL` |
| `GreaterOrEqual`| `VK_COMPARE_OP_GREATER_OR_EQUAL` |
| `Always`       | `VK_COMPARE_OP_ALWAYS` |

Both enums provide a `TypeCast` function for conversion to the corresponding `vk::` type.

## See also

- [RHI](index.md) — parent module
- [Resource](Resource/index.md) — resource types
- [CommandList](CommandList.md) — uses ERHIIndexType
- [Resource.RenderPass](Resource/RenderPass.md) — uses ERHICompareOp
