module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource;
#define VISERA_MODULE_NAME "RHI.Resource"
export import Visera.RHI.Resource.Texture;
export import Visera.RHI.Resource.Sampler;
export import Visera.RHI.Resource.Buffer;
       import Visera.Core.Types.Handle;
       import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    enum class ERHIResourceType : UInt8
    {
        Unknown = 0,
        Texture,
        Sampler,
        Buffer,
    };

    class FRHIResourceHandle : public FHandle
    {
        enum : UInt32 // Embedded in the Generation(High32)
        {
            GENERATION_MASK    = (1U << 28) - 1U,  //[0~27]    : 28Bits
            TYPE_MASK          = (0b111) << 28,    //[28~30]   :  3Bits
            WRITABLE_MASK      = (1U << 31),       //[31]      :  1Bit
        };
    public:
        static constexpr TClosedInterval<UInt32>
        GetGenerationRange() { return {1U, GENERATION_MASK}; }

        [[nodiscard]] constexpr UInt32
        GetIndex()      const { return static_cast<UInt32>(Value & 0xFFFFFFFFULL); }
        [[nodiscard]] constexpr UInt32
        GetGeneration() const { return static_cast<UInt32>(Value >> 32) & GENERATION_MASK; }
        [[nodiscard]] constexpr ERHIResourceType
        GetType() const { return static_cast<ERHIResourceType>((FHandle::GetGeneration() & TYPE_MASK) >> 28); }
        [[nodiscard]] constexpr Bool
        IsWritable() const { return (GetGeneration() & WRITABLE_MASK) != 0; }

    public:
        FRHIResourceHandle() = default;
        FRHIResourceHandle(UInt32 I_Generation, UInt32 I_Index) {}
    };
    static_assert(Concepts::Handle<FRHIResourceHandle>);
}
VISERA_MAKE_FORMATTER(Visera::ERHIResourceType,
    const char* Name = "Unknown";
    switch (I_Formatee)
    {
    case Visera::ERHIResourceType::Texture:     Name = "Texture";   break;
    case Visera::ERHIResourceType::Sampler:     Name = "Sampler";   break;
    case Visera::ERHIResourceType::Buffer:      Name = "Buffer";    break;
    }, "{}", Name);
VISERA_MAKE_HASH(Visera::FRHIResourceHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIResourceHandle, {},
    "Type:{}, Gen:{}, Idx:{}",
    I_Formatee.GetType(),
    I_Formatee.GetGeneration(),
    I_Formatee.GetIndex());