module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.Buffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.Buffer;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIBuffer : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            UInt64           Size           {0};
            ERHIBufferUsage  Usages         {ERHIBufferUsage::None};
            /** When true the buffer is allocated in host-visible, persistently
             *  mapped memory so the CPU can write directly without staging.
             *  Ideal for per-frame data such as instance SSBOs. */
            Bool             bHostWritable  {False};

            Bool operator==(const FCreateInfo&) const = default;
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            { return (Size >= I_Other.Size) &&
                     ((Usages & I_Other.Usages) == I_Other.Usages) &&
                     (bHostWritable == I_Other.bHostWritable); }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        void
        Write(const FByte* I_Data, UInt64 I_Size) { Buffer.Write(I_Data, I_Size); }
        [[nodiscard]] auto*
        GetVulkanBuffer() { return &Buffer; }

    private:
        const FCreateInfo Info;
        FVulkanBuffer     Buffer;

    public:
        FRHIBuffer() = delete;
        FRHIBuffer(FCreateInfo&& I_CreateInfo, FVulkanBuffer&& I_Buffer)
        : Info  {std::move(I_CreateInfo)},
          Buffer{std::move(I_Buffer)}
        {

        }
    };

    using FRHIBufferCreateInfo = FRHIBuffer::FCreateInfo;
}
