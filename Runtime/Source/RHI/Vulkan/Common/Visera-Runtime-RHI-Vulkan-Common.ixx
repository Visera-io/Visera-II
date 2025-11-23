module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera
{
    constexpr auto IdentitySwizzle = vk::ComponentMapping{}
        .setR(vk::ComponentSwizzle::eIdentity)
        .setG(vk::ComponentSwizzle::eIdentity)
        .setB(vk::ComponentSwizzle::eIdentity)
        .setA(vk::ComponentSwizzle::eIdentity)
    ;
}

template <>
struct fmt::formatter<vk::ImageLayout>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(vk::ImageLayout I_ImageLayout, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* Name = "(WIP)";
        switch (I_ImageLayout)
        {
        case vk::ImageLayout::eUndefined:                         Name = "Undefined"; break;
        case vk::ImageLayout::eColorAttachmentOptimal:            Name = "Color"; break;
        case vk::ImageLayout::eDepthAttachmentOptimal:            Name = "Depth"; break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:     Name = "DepthStencil"; break;
        case vk::ImageLayout::eTransferSrcOptimal:                Name = "TransferSource"; break;
        case vk::ImageLayout::eTransferDstOptimal:                Name = "TransferDestination"; break;
        case vk::ImageLayout::ePresentSrcKHR:                     Name = "Present"; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", Name);
    }
};


template <>
struct fmt::formatter<vk::PresentModeKHR>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(vk::PresentModeKHR I_ImageLayout, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* Name = "Unknown";
        switch (I_ImageLayout)
        {
        case vk::PresentModeKHR::eImmediate:               Name = "Immediate"; break;
        case vk::PresentModeKHR::eMailbox:                 Name = "Mailbox"; break;
        case vk::PresentModeKHR::eFifo:                    Name = "FIFO"; break;
        case vk::PresentModeKHR::eFifoRelaxed:             Name = "FIFO Relaxed"; break;
        case vk::PresentModeKHR::eSharedDemandRefresh:     Name = "Shared Demand Refresh"; break;
        case vk::PresentModeKHR::eSharedContinuousRefresh: Name = "Shared Continuous Refresh"; break;
        case vk::PresentModeKHR::eFifoLatestReady:         Name = "FIFO Latest Ready"; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", Name);
    }
};

template <>
struct fmt::formatter<vk::Result>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(vk::Result I_Result, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* Message = "(WIP)";
        switch (I_Result)
        {
        case vk::Result::eSuccess: Message = "Success"; break;
        case vk::Result::eNotReady: Message = "A fence or query has not yet completed."; break;
        case vk::Result::eTimeout: Message = "A wait operation has not completed in the specified time."; break;
        case vk::Result::eEventSet: Message = "An event is signaled."; break;
        case vk::Result::eEventReset: Message = "An event is unsignaled."; break;
        case vk::Result::eIncomplete: Message = "A Message = array was too small for the result."; break;
        case vk::Result::eErrorOutOfHostMemory: Message = "Host memory allocation failed."; break;
        case vk::Result::eErrorOutOfDeviceMemory: Message = "Device memory allocation failed."; break;
        case vk::Result::eErrorInitializationFailed: Message = "Initialization failed for implementation-specific reasons."; break;
        case vk::Result::eErrorDeviceLost: Message = "The logical or physical device has been lost."; break;
        case vk::Result::eErrorMemoryMapFailed: Message = "Mapping of a memory object has failed."; break;
        case vk::Result::eErrorLayerNotPresent: Message = "A requested layer is not present or cannot be loaded."; break;
        case vk::Result::eErrorExtensionNotPresent: Message = "A requested extension is not supported."; break;
        case vk::Result::eErrorFeatureNotPresent: Message = "A requested feature is not supported."; break;
        case vk::Result::eErrorIncompatibleDriver: Message = "The requested version of Vulkan is not supported by the driver."; break;
        case vk::Result::eErrorTooManyObjects: Message = "Too many objects of the type have already been created."; break;
        case vk::Result::eErrorFormatNotSupported: Message = "A requested format is not supported on this device."; break;
        case vk::Result::eErrorFragmentedPool: Message = "A pool allocation has failed due to fragmentation."; break;
        case vk::Result::eErrorOutOfPoolMemory: Message = "A pool memory allocation has failed."; break;
        case vk::Result::eErrorInvalidExternalHandle: Message = "An external handle is not a valid handle."; break;
        case vk::Result::eErrorSurfaceLostKHR: Message = "A surface is no longer available."; break;
        case vk::Result::eErrorNativeWindowInUseKHR: Message = "The native window is already in use."; break;
        case vk::Result::eSuboptimalKHR: Message = "Swapchain no longer matches the surface exactly."; break;
        case vk::Result::eErrorOutOfDateKHR: Message = "Swapchain is out of date, must be recreated."; break;
        case vk::Result::eErrorValidationFailedEXT: Message = "Validation layer detected a validation failure."; break;
        case vk::Result::eErrorInvalidShaderNV: Message = "Invalid shader was provided."; break;
        case vk::Result::eErrorInvalidDeviceAddressEXT: Message = "Invalid device address."; break;
        case vk::Result::eErrorPipelineCompileRequiredEXT: Message = "Pipeline requires compilation."; break;
        case vk::Result::eErrorNotPermittedEXT: Message = "Operation not permitted."; break;
        default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", Message);
    }
};