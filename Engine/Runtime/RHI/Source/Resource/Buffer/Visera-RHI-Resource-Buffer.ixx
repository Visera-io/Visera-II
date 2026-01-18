module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Buffer;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.Buffer;

export namespace Visera
{
    struct VISERA_RHI_API FRHIBufferCreateDesc
    {
        UInt64           Size       {0};
        ERHIBufferUsage  Usages     {ERHIBufferUsage::None};

        Bool IsCompatibleWith(const FRHIBufferCreateDesc& I_Other) const
        { return (Size >= I_Other.Size) &&
                 ((Usages & I_Other.Usages) == I_Other.Usages); }
    };

    class VISERA_RHI_API FRHIBuffer
    {
    public:
        [[nodiscard]] const auto&
        GetInfo() const { return Info; }
        [[nodiscard]] void
        Write(const FByte* I_Data, UInt64 I_Size) { Buffer.Write(I_Data, I_Size); }
        [[nodiscard]] auto*
        GetVulkanBuffer() { return &Buffer; }

    private:
        const FRHIBufferCreateDesc Info;
        FVulkanBuffer              Buffer;

    public:
        FRHIBuffer() = delete;
        FRHIBuffer(FRHIBufferCreateDesc&& I_BufferDesc, FVulkanBuffer&& I_Buffer)
        : Info  {std::move(I_BufferDesc)},
          Buffer{std::move(I_Buffer)}
        {

        }
    };
}