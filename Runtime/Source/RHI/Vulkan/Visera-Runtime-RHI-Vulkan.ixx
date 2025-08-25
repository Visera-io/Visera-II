module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkan
    {
    public:
        using FContext        = vk::raii::Context;
        using FInstance       = vk::raii::Instance;
        using FPhysicalDevice = vk::raii::PhysicalDevice;
        using FDevice         = vk::raii::Device;

        [[nodiscard]] inline const FInstance&
        GetInstance() const { return Instance; }
        [[nodiscard]] inline const FDevice&
        GetDevice()   const { return Device; }

        FVulkan() = delete;
        FVulkan(TArrayView<const char*> I_InstanceExtensions,
                TArrayView<const char*> I_DeviceExtensions);
        ~FVulkan();

    private:
        FContext        Context;
        FInstance       Instance  {nullptr};
        FPhysicalDevice GPU       {nullptr};
        FDevice         Device    {nullptr};

        TArrayView<const char*> InstanceExtensions;
        TArrayView<const char*> DeviceExtensions;

    private:
        void inline
        CreateInstance();
        void inline
        CreateDevice();
    };

    FVulkan::
    FVulkan(TArrayView<const char*> I_InstanceExtensions,
            TArrayView<const char*> I_DeviceExtensions)
                :
            InstanceExtensions{I_InstanceExtensions},
            DeviceExtensions  {I_DeviceExtensions}
    {
        CreateInstance();
        CreateDevice();
    }

    FVulkan::
    ~FVulkan()
    {

    }

    void FVulkan::
    CreateInstance()
    {
        auto AppInfo = vk::ApplicationInfo{}
            .setPApplicationName    ("Visera")
            .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName         ("Visera")
            .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion          (vk::ApiVersion14);

        // Check if the required GLFW extensions are supported by the Vulkan implementation.
        auto ExtensionProperties = Context.enumerateInstanceExtensionProperties();

        for (UInt32 Idx = 0; Idx < InstanceExtensions.size(); ++Idx)
        {
            if (std::ranges::none_of(ExtensionProperties,
                                     [Extension = InstanceExtensions[Idx]](auto const& ExtensionProperty)
                                     { return strcmp(ExtensionProperty.extensionName, Extension) == 0; }))
            {
                throw SRuntimeError(fmt::format("Required extension not supported: {}", InstanceExtensions[Idx]));
            }
        }

        auto CreateInfo = vk::InstanceCreateInfo{}
            .setPApplicationInfo(&AppInfo)
            .setEnabledExtensionCount(InstanceExtensions.size())
            .setPpEnabledExtensionNames(InstanceExtensions.data())
#if defined(VISERA_ON_APPLE_SYSTEM)
            .setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR)
#endif()
        ;

        Instance = FInstance(Context, CreateInfo);
    }

    void FVulkan::
    CreateDevice()
    {

    }
}
