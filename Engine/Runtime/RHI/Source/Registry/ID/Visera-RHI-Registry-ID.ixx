module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Registry.ID;
#define VISERA_MODULE_NAME "RHI.Registry"
import Visera.Core.Types.Handle;
import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    class VISERA_RHI_API FRHIResourceID : public FHandle
    {
    public:
        enum class EType : UInt8
        {
            Unknown = 0,
            Texture,
            Sampler,
            Buffer,
            RenderPass,
            DescriptorSet,
            Shader,
            DescriptorSetLayout,
        };

        enum : UInt32 // Embedded in the Generation(High32)
        {
            GENERATION_MASK    = (1U << 28) - 1U,  //[0~27]    : 28Bits
            TYPE_MASK          = (0b111) << 28,    //[28~30]   :  3Bits
            WRITABLE_MASK      = (1U << 31),       //[31]      :  1Bit
        };
        static constexpr TClosedInterval<UInt32>
        GetGenerationRange() { return {1U, GENERATION_MASK}; }

        [[nodiscard]] constexpr UInt32
        GetIndex()      const { return static_cast<UInt32>(Value & 0xFFFFFFFFULL); }
        [[nodiscard]] constexpr UInt32
        GetGeneration() const { return static_cast<UInt32>(Value >> 32) & GENERATION_MASK; }
        [[nodiscard]] constexpr EType
        GetType() const { return static_cast<EType>((FHandle::GetGeneration() & TYPE_MASK) >> 28); }
        [[nodiscard]] constexpr Bool
        IsWritable() const { return ((Value >> 32) & WRITABLE_MASK) != 0; }

    public:
        FRHIResourceID() = default;
        FRHIResourceID(UInt32 I_Generation, UInt32 I_Index,
            EType I_Type      = EType::Unknown,
            Bool             I_bWritable = False)
        {
            const UInt32 GenerationBits = (I_Generation & GENERATION_MASK);
            const UInt32 TypeBits       = (static_cast<UInt32>(I_Type) & 0b111U) << 28;
            const UInt32 WritableBit    = I_bWritable ? WRITABLE_MASK : 0U;

            Value = (static_cast<UInt64>(WritableBit | TypeBits | GenerationBits) << 32) | I_Index;
        }
    };
    static_assert(Concepts::Handle<FRHIResourceID>);

    struct VISERA_RHI_API FRHITextureID : FRHIResourceID
    {
        FRHITextureID() = default;
        FRHITextureID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHITextureID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::Texture, False) {}
        FRHITextureID(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceID(I_Generation, I_Index, EType::Texture, I_bWritable) {}
    };

    struct VISERA_RHI_API FRHIBufferID : FRHIResourceID
    {
        FRHIBufferID() = default;
        FRHIBufferID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHIBufferID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::Buffer, False) {}
        FRHIBufferID(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceID(I_Generation, I_Index, EType::Buffer, I_bWritable) {}
    };

    struct VISERA_RHI_API FRHISamplerID : FRHIResourceID
    {
        FRHISamplerID() = default;
        FRHISamplerID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHISamplerID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::Sampler, False) {}
        FRHISamplerID(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceID(I_Generation, I_Index, EType::Sampler, I_bWritable) {}
    };

    struct VISERA_RHI_API FRHIShaderID : FRHIResourceID
    {
        FRHIShaderID() = default;
        FRHIShaderID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHIShaderID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::Shader, False) {}
    };

    struct VISERA_RHI_API FRHIRenderPassID : FRHIResourceID
    {
        FRHIRenderPassID() = default;
        FRHIRenderPassID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHIRenderPassID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::RenderPass, False) {}
    };

    /** Internal use only: index into Registry's DescriptorSetLayout cache. Users create layout + DSet via FRHIDescriptorSetCreateDesc only. */
    struct VISERA_RHI_API FRHIDescriptorSetLayoutID : FRHIResourceID
    {
        FRHIDescriptorSetLayoutID() = default;
        FRHIDescriptorSetLayoutID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHIDescriptorSetLayoutID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::DescriptorSetLayout, False) {}
        FRHIDescriptorSetLayoutID(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceID(I_Generation, I_Index, EType::DescriptorSetLayout, I_bWritable) {}
    };

    struct VISERA_RHI_API FRHIDescriptorSetID : FRHIResourceID
    {
        FRHIDescriptorSetID() = default;
        FRHIDescriptorSetID(const FRHIResourceID& I_Handle) : FRHIResourceID(I_Handle) {}
        FRHIDescriptorSetID(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceID(I_Generation, I_Index, EType::DescriptorSet, False) {}
        FRHIDescriptorSetID(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceID(I_Generation, I_Index, EType::DescriptorSet, I_bWritable) {}
    };

    namespace Concepts
    {
        template<typename T> concept
        RHIID = std::derived_from<std::remove_cvref_t<T>, FRHIResourceID>;
    }
}
VISERA_MAKE_FORMATTER(Visera::FRHIRenderPassID::EType,
    const char* Name = "None";
    switch (I_Formatee)
    {
    case Visera::FRHIRenderPassID::EType::Texture:             Name = "Texture";             break;
    case Visera::FRHIRenderPassID::EType::Sampler:             Name = "Sampler";             break;
    case Visera::FRHIRenderPassID::EType::Buffer:              Name = "Buffer";              break;
    case Visera::FRHIRenderPassID::EType::RenderPass:          Name = "RenderPass";          break;
    case Visera::FRHIRenderPassID::EType::DescriptorSet:       Name = "DescriptorSet";       break;
    case Visera::FRHIRenderPassID::EType::Shader:              Name = "Shader";             break;
    case Visera::FRHIRenderPassID::EType::DescriptorSetLayout: Name = "DescriptorSetLayout"; break;
    default: break;
    }, "{}", Name
);

VISERA_MAKE_HASH(Visera::FRHIResourceID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIResourceID, {},
    "Type:{}, Writable:{}, Gen:{}, Idx:{}",
    I_Formatee.GetType(),
    I_Formatee.IsWritable(),
    I_Formatee.GetGeneration(),
    I_Formatee.GetIndex());

VISERA_MAKE_HASH(Visera::FRHITextureID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHITextureID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIBufferID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIBufferID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHISamplerID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHISamplerID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIDescriptorSetLayoutID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIDescriptorSetLayoutID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIDescriptorSetID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIDescriptorSetID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIShaderID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIShaderID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIRenderPassID, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIRenderPassID, {}, "{}", static_cast<Visera::FRHIResourceID>(I_Formatee));