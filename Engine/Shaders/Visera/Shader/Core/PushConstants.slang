/// Engine-wide push constants. All pipelines share this layout.
/// Per-frame / per-material data belongs in UBOs (set 2+), not here.

struct ViseraPushConstants
{
    uint InstanceOffset;   // starting index into the instance StructuredBuffer
};

[[vk::push_constant]] ViseraPushConstants _PushConstants;
