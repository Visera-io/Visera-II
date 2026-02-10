module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Registry.Handle;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Types.Handle;
import Visera.Core.Math.Arithmetic.Interval;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIResourceHandle : public FHandle
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
        FRHIResourceHandle() = default;
        FRHIResourceHandle(UInt32 I_Generation, UInt32 I_Index,
            EType I_Type      = EType::Unknown,
            Bool             I_bWritable = False)
        {
            const UInt32 GenerationBits = (I_Generation & GENERATION_MASK);
            const UInt32 TypeBits       = (static_cast<UInt32>(I_Type) & 0b111U) << 28;
            const UInt32 WritableBit    = I_bWritable ? WRITABLE_MASK : 0U;

            Value = (static_cast<UInt64>(WritableBit | TypeBits | GenerationBits) << 32) | I_Index;
        }
    };
    static_assert(Concepts::Handle<FRHIResourceHandle>);

    struct VISERA_RUNTIME_API FRHITextureHandle : FRHIResourceHandle
    {
        FRHITextureHandle() = default;
        FRHITextureHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHITextureHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Texture, False) {}
        FRHITextureHandle(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Texture, I_bWritable) {}
    };

    struct VISERA_RUNTIME_API FRHIBufferHandle : FRHIResourceHandle
    {
        FRHIBufferHandle() = default;
        FRHIBufferHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHIBufferHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Buffer, False) {}
        FRHIBufferHandle(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Buffer, I_bWritable) {}
    };

    struct VISERA_RUNTIME_API FRHISamplerHandle : FRHIResourceHandle
    {
        FRHISamplerHandle() = default;
        FRHISamplerHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHISamplerHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Sampler, False) {}
        FRHISamplerHandle(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Sampler, I_bWritable) {}
    };

    struct VISERA_RUNTIME_API FRHIShaderHandle : FRHIResourceHandle
    {
        FRHIShaderHandle() = default;
        FRHIShaderHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHIShaderHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::Shader, False) {}
    };

    struct VISERA_RUNTIME_API FRHIRenderPassHandle : FRHIResourceHandle
    {
        FRHIRenderPassHandle() = default;
        FRHIRenderPassHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHIRenderPassHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::RenderPass, False) {}
    };

    /** Internal use only: index into Registry's DescriptorSetLayout cache. Users create layout + DSet via FRHIDescriptorSetCreateInfo only. */
    struct VISERA_RUNTIME_API FRHIDescriptorSetLayoutHandle : FRHIResourceHandle
    {
        FRHIDescriptorSetLayoutHandle() = default;
        FRHIDescriptorSetLayoutHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHIDescriptorSetLayoutHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::DescriptorSetLayout, False) {}
        FRHIDescriptorSetLayoutHandle(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceHandle(I_Generation, I_Index, EType::DescriptorSetLayout, I_bWritable) {}
    };

    struct VISERA_RUNTIME_API FRHIDescriptorSetHandle : FRHIResourceHandle
    {
        FRHIDescriptorSetHandle() = default;
        FRHIDescriptorSetHandle(const FRHIResourceHandle& I_Handle) : FRHIResourceHandle(I_Handle) {}
        FRHIDescriptorSetHandle(UInt32 I_Generation, UInt32 I_Index)
        : FRHIResourceHandle(I_Generation, I_Index, EType::DescriptorSet, False) {}
        FRHIDescriptorSetHandle(UInt32 I_Generation, UInt32 I_Index, Bool I_bWritable)
        : FRHIResourceHandle(I_Generation, I_Index, EType::DescriptorSet, I_bWritable) {}
    };

    namespace Concepts
    {
        template<typename T> concept
        RHIHandle = std::derived_from<std::remove_cvref_t<T>, FRHIResourceHandle>;
    }
}
VISERA_MAKE_FORMATTER(Visera::FRHIRenderPassHandle::EType,
    const char* Name = "None";
    switch (I_Formatee)
    {
    case Visera::FRHIRenderPassHandle::EType::Texture:             Name = "Texture";             break;
    case Visera::FRHIRenderPassHandle::EType::Sampler:             Name = "Sampler";             break;
    case Visera::FRHIRenderPassHandle::EType::Buffer:              Name = "Buffer";              break;
    case Visera::FRHIRenderPassHandle::EType::RenderPass:          Name = "RenderPass";          break;
    case Visera::FRHIRenderPassHandle::EType::DescriptorSet:       Name = "DescriptorSet";       break;
    case Visera::FRHIRenderPassHandle::EType::Shader:              Name = "Shader";             break;
    case Visera::FRHIRenderPassHandle::EType::DescriptorSetLayout: Name = "DescriptorSetLayout"; break;
    default: break;
    }, "{}", Name
);

VISERA_MAKE_HASH(Visera::FRHIResourceHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIResourceHandle, {},
    "Type:{}, Writable:{}, Gen:{}, Idx:{}",
    I_Formatee.GetType(),
    I_Formatee.IsWritable(),
    I_Formatee.GetGeneration(),
    I_Formatee.GetIndex());

VISERA_MAKE_HASH(Visera::FRHITextureHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHITextureHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIBufferHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIBufferHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHISamplerHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHISamplerHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIDescriptorSetLayoutHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIDescriptorSetLayoutHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIDescriptorSetHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIDescriptorSetHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIShaderHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIShaderHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));

VISERA_MAKE_HASH(Visera::FRHIRenderPassHandle, { return I_Object.GetValue(); })
VISERA_MAKE_FORMATTER(Visera::FRHIRenderPassHandle, {}, "{}", static_cast<Visera::FRHIResourceHandle>(I_Formatee));