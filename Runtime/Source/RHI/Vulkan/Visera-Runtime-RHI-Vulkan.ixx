module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Driver;
import Visera.Runtime.RHI.Vulkan.Loader;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Runtime.Window;
import Visera.Core.Log;
import Visera.Core.Math.Arithmetic;

namespace Visera::RHI
{
    class VISERA_RUNTIME_API FVulkan : public IDriver
    {
    public:
        vk::ApplicationInfo              AppInfo;
        vk::raii::Context                Context;

        vk::raii::Instance               Instance        {nullptr};
        vk::raii::DebugUtilsMessengerEXT DebugMessenger  {nullptr};

        vk::raii::SurfaceKHR             Surface         {nullptr};

        struct
        {
            vk::raii::PhysicalDevice Context {nullptr};
            TSet<UInt32> GraphicsQueueFamilies{};
            TSet<UInt32> PresentQueueFamilies {};
            TSet<UInt32> ComputeQueueFamilies {};
            TSet<UInt32> TransferQueueFamilies{};
        }GPU;

        struct
        {
            vk::raii::Device Context         {nullptr};
            vk::raii::Queue  GraphicsQueue   {nullptr};
        }Device;

        TUniquePtr<FVulkanAllocator>Allocator;

        struct
        {
            vk::raii::SwapchainKHR  Context     {nullptr};
            vk::raii::SwapchainKHR  OldContext  {nullptr};
            vk::Extent2D            Extent      {0U, 0U};
            TArray<vk::Image>           Images      {}; // SwapChain manage Images so do NOT use RAII here.
            TArray<vk::raii::ImageView> ImageViews  {};
            vk::ImageUsageFlags     ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eTransferDst};
            vk::Format              ImageFormat {vk::Format::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR       ColorSpace  {vk::ColorSpaceKHR::eSrgbNonlinear};
            UInt32                  MinimalImageCount{3};
            vk::PresentModeKHR      PresentMode {vk::PresentModeKHR::eFifo};
            vk::SharingMode         SharingMode {vk::SharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                          bClipped       {True};
        }SwapChain;

    private:
        TArray<const char*> InstanceLayers;
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceLayers;
        TArray<const char*> DeviceExtensions;

    public:
        void
        BeginFrame() override {};
        void
        EndFrame()   override {};
        void
        Present()    override {};
        inline void*
        GetNativeInstance() override { return *Instance;       };
        inline void*
        GetNativeDevice()   override { return *Device.Context; };

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateDevice();
        void CreateSwapChain();

        inline FVulkan*
        AddInstanceLayer(const char* I_Layer)           { InstanceLayers.emplace_back(I_Layer);         return this; }
        inline FVulkan*
        AddInstanceExtension(const char* I_Extension)   { InstanceExtensions.push_back(I_Extension);    return this; }
        inline FVulkan*
        AddDeviceLayer(const char* I_Layer)             { DeviceLayers.push_back(I_Layer);              return this; }
        inline FVulkan*
        AddDeviceExtension(const char* I_Extension)     { DeviceExtensions.push_back(I_Extension);      return this; }

        void inline
        CollectInstanceLayersAndExtensions();
        void inline
        CollectDeviceLayersAndExtensions();

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL
        DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      I_Severity,
                      vk::DebugUtilsMessageTypeFlagsEXT             I_Type,
                      const vk::DebugUtilsMessengerCallbackDataEXT* I_CallbackData,
                      void*)
        {
            if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
            {
                //LOG_INFO("{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            {
                LOG_DEBUG("{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            {
                LOG_WARN("{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            {
                LOG_ERROR("{}", I_CallbackData->pMessage);
            }
            else
            {
                LOG_FATAL("Unknown Vulkan Debug Message Severity {}", static_cast<Int32>(I_Severity));
            }
            return vk::False; // Always return VK_FALSE
        }

    public:
        FVulkan() : IDriver{EDriverType::Vulkan} {};
        ~FVulkan() override;
        void Bootstrap() override;
        void Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<IDriver>
    GVulkan = MakeUnique<FVulkan>();

    void FVulkan::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping Vulkan.");

        AppInfo = vk::ApplicationInfo{}
        .setPApplicationName    ("Visera")
        .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName         ("Visera")
        .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion          (vk::ApiVersion13);

        GVulkanLoader->Bootstrap();

        // Instance
        {
            CollectInstanceLayersAndExtensions();

            CreateInstance();
            GVulkanLoader->Load(Instance);

            CreateDebugMessenger();
        }

        // Surface
        if (GWindow->IsBootstrapped())
        {
            CreateSurface();
        }

        // Device
        {
            CollectDeviceLayersAndExtensions();

            PickPhysicalDevice();
            CreateDevice();
            GVulkanLoader->Load(Device.Context);
        }

        // Allocator
        {
            Allocator = MakeUnique<FVulkanAllocator>
            (
                AppInfo.apiVersion,
                *Instance,
                *GPU.Context,
                *Device.Context
            );
        }

        // Swap Chain
        if (GWindow->IsBootstrapped())
        {
            CreateSwapChain();
        }

        Statue = EStatues::Bootstrapped;
    }
    void FVulkan::
    Terminate()
    {
        LOG_DEBUG("Terminating Vulkan.");
        Device.Context.waitIdle();

        if (GWindow->IsBootstrapped())
        {
            SwapChain.ImageViews.clear();
            SwapChain.Context.clear();
        }

        // Device
        {
            Device.Context.clear();
        }

        // Allocator
        {
            Allocator.reset();
        }

        // Surface
        if (GWindow->IsBootstrapped())
        {
            Surface.clear();
        }

        // Instance
        {
            DebugMessenger.clear();
            Instance.clear();
        }

        GVulkanLoader->Terminate();

        Statue = EStatues::Terminated;
    }

    FVulkan::
    ~FVulkan()
    {
        if (IsBootstrapped())
        { std::cerr << "GVulkan did NOT be terminated properly!"; }
    }

    void FVulkan::
    CreateInstance()
    {
        // Check Layers
        const auto LayerProperties = Context.enumerateInstanceLayerProperties();
        for (const auto& Layer : InstanceLayers)
        {
            if (std::ranges::none_of(LayerProperties,
                [&Layer](auto const& LayerProperty)
                { return strcmp(LayerProperty.layerName, Layer) == 0; }))
            {
                throw SRuntimeError(fmt::format("Required instance layer \"{}\" is not supported!", Layer));
            }
        }

        // Check Extensions
        auto ExtensionProperties = Context.enumerateInstanceExtensionProperties();
        for (const auto& Extension : InstanceExtensions)
        {
            if (std::ranges::none_of(ExtensionProperties,
                [&Extension](auto const& ExtensionProperty)
                { return strcmp(ExtensionProperty.extensionName, Extension) == 0; }))
            {
                throw SRuntimeError(fmt::format("Required instance extension \"{}\" is not supported!", Extension));
            }
        }

        auto Flags = vk::InstanceCreateFlags{};
#if defined(VISERA_ON_APPLE_SYSTEM)
        Flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

        const auto CreateInfo = vk::InstanceCreateInfo{}
            .setPApplicationInfo        (&AppInfo)
            .setEnabledLayerCount       (InstanceLayers.size())
            .setPpEnabledLayerNames     (InstanceLayers.data())
            .setEnabledExtensionCount   (InstanceExtensions.size())
            .setPpEnabledExtensionNames (InstanceExtensions.data())
            .setFlags                   (Flags)
        ;

        Instance = vk::raii::Instance(Context, CreateInfo);
    }

    void FVulkan::
    CreateDebugMessenger()
    {
#if defined(VISERA_DEBUG_MODE)
        auto CreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
            .setMessageSeverity(//vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose    |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning  |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                                )
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral       |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                            )
            .setPfnUserCallback(DebugCallback);
        DebugMessenger = Instance.createDebugUtilsMessengerEXT(CreateInfo);
#endif
    }

    void FVulkan::
    CreateSurface()
    {
        VkSurfaceKHR SurfaceHandle;

        if(glfwCreateWindowSurface(
            *Instance,
            GWindow->GetHandle(),
            nullptr,
            &SurfaceHandle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create Vulkan Surface!"); }

        Surface = vk::raii::SurfaceKHR(Instance, SurfaceHandle);
    }

    void FVulkan::
    PickPhysicalDevice()
    {
        VISERA_ASSERT(Instance != nullptr);

        auto PhysicalDeviceCandidates = Instance.enumeratePhysicalDevices();
        if (PhysicalDeviceCandidates.empty())
        { throw SRuntimeError("Failed to find PhysicalDevices with Vulkan support!"); }

        const auto SelectedPhysicalDevice = std::ranges::
        find_if(PhysicalDeviceCandidates, [&](auto const & PhysicalDeviceCandidate)
        {
            auto PhysicalDeviceInfo = PhysicalDeviceCandidate.getProperties();
            LOG_DEBUG("Current PhysicalDevice candidate: {}.", PhysicalDeviceInfo.deviceName.data());

            LOG_DEBUG("Checking Vulkan API Version...");
            Bool bSuitable = PhysicalDeviceInfo.apiVersion >= AppInfo.apiVersion;
            if (!bSuitable) { return False; }

            LOG_DEBUG("Checking Queue Families...");
            auto QueueFamilies = PhysicalDeviceCandidate.getQueueFamilyProperties();

            for (UInt32 Idx = 0; Idx < QueueFamilies.size(); ++Idx)
            {
                auto& QueueFamily = QueueFamilies[Idx];
                if (vk::QueueFlagBits::eGraphics & QueueFamily.queueFlags)
                { GPU.GraphicsQueueFamilies.insert(Idx); }
                if (vk::QueueFlagBits::eCompute & QueueFamily.queueFlags)
                { GPU.ComputeQueueFamilies.insert(Idx); }
                if (vk::QueueFlagBits::eTransfer & QueueFamily.queueFlags)
                { GPU.TransferQueueFamilies.insert(Idx); }
                if (GWindow->IsBootstrapped())
                {
                    VISERA_ASSERT(Surface != nullptr);
                    if (PhysicalDeviceCandidate.getSurfaceSupportKHR(Idx, *Surface))
                    { GPU.PresentQueueFamilies.insert(Idx); }
                }
            }
            bSuitable = !GPU.GraphicsQueueFamilies.empty() &&
                        !GPU.ComputeQueueFamilies.empty()  &&
                        !GPU.TransferQueueFamilies.empty();
            if (GWindow->IsBootstrapped()) { bSuitable &= !GPU.PresentQueueFamilies.empty(); }

            if (!bSuitable) { return False; }

            LOG_DEBUG("Checking Extension supporting...");
            auto Extensions = PhysicalDeviceCandidate.enumerateDeviceExtensionProperties( );
            for (auto const & RequiredExtension : DeviceExtensions)
            {
                auto ExtensionIter =
                    std::ranges::find_if(Extensions,
                [RequiredExtension](auto const & Extension)
                    {return strcmp(Extension.extensionName, RequiredExtension) == 0;});
                bSuitable &= (ExtensionIter != Extensions.end());
            }

            if (bSuitable)
            { GPU.Context = PhysicalDeviceCandidate; }
            return bSuitable;
        });

        if (SelectedPhysicalDevice == PhysicalDeviceCandidates.end())
        { throw SRuntimeError("Failed to find a suitable PhysicalDevice!"); }

        LOG_INFO("Selected PhysicalDevice: {}", GPU.Context.getProperties().deviceName.data());
    }

    void FVulkan::
    CreateDevice()
    {
        VISERA_ASSERT(GPU.Context != nullptr);

        constexpr Float Priority = 0.0f;
        auto DeviceQueueCreateInfo = vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(*GPU.GraphicsQueueFamilies.begin())
            .setQueueCount(1)
            .setQueuePriorities({Priority});

        // First build each feature struct explicitly
        auto Feature1 = vk::PhysicalDeviceFeatures2{};

        auto Feature2 =  vk::PhysicalDeviceVulkan13Features{};
        Feature2.dynamicRendering     = VK_TRUE;

        auto Feature3 = vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{};
        Feature3.extendedDynamicState = VK_TRUE;

        vk::StructureChain FeatureChain(Feature1, Feature2, Feature3);

        const auto CreateInfo = vk::DeviceCreateInfo{}
            .setPNext                   (&FeatureChain.get<vk::PhysicalDeviceFeatures2>())
            .setQueueCreateInfoCount    (1)
            .setPQueueCreateInfos       (&DeviceQueueCreateInfo)
            .setEnabledExtensionCount   (DeviceExtensions.size())
            .setPpEnabledExtensionNames (DeviceExtensions.data())
        ;
        Device.Context       = vk::raii::Device{GPU.Context, CreateInfo};

        Device.GraphicsQueue = vk::raii::Queue{Device.Context, *GPU.GraphicsQueueFamilies.begin(), 0};
    }


    void FVulkan::
    CollectInstanceLayersAndExtensions()
    {
        // Layers
        this
#if defined(VISERA_DEBUG_MODE)
            ->AddInstanceLayer("VK_LAYER_KHRONOS_validation")
            ->AddInstanceLayer("VK_LAYER_KHRONOS_synchronization2")
        ;
#else

#endif
        ;

        // Extensions
        this
#if defined(VISERA_DEBUG_MODE)
            ->AddInstanceExtension(vk::EXTDebugUtilsExtensionName)
#else

#endif
#if defined(VISERA_ON_APPLE_SYSTEM)
            ->AddInstanceExtension(vk::KHRPortabilityEnumerationExtensionName)
#endif
        ;

        if (GWindow->IsBootstrapped())
        {
            if (!glfwVulkanSupported())
            { throw SRuntimeError("The Window does NOT support Vulkan!"); }

            UInt32 WindowExtensionCount = 0;
            auto WindowExtensions = glfwGetRequiredInstanceExtensions(&WindowExtensionCount);

            for (UInt32 Idx = 0; Idx < WindowExtensionCount; ++Idx)
            {
                this->AddInstanceExtension(WindowExtensions[Idx]);
            }
        }
    }

    void FVulkan::
    CreateSwapChain()
    {
        VISERA_ASSERT(Surface != nullptr);

        // Check Present Mode
        {
            Bool bFoundRequiredPresentMode {False};
            for (const auto PresentModes= GPU.Context.getSurfacePresentModesKHR(Surface);
                 const auto& PresentMode : PresentModes)
            {
                if (PresentMode == SwapChain.PresentMode)
                { bFoundRequiredPresentMode = True; break; }
            }
            if (!bFoundRequiredPresentMode)
            { throw SRuntimeError("Failed to find required present mode for swapchain!"); }
        }

        // Check Image Format and Color Space
        {
            Bool bFoundRequiredFormatAndColorSpace {False};
            for (const auto AvailableFormats = GPU.Context.getSurfaceFormatsKHR(Surface);
                 const auto& AvailableFormat : AvailableFormats)
            {
                if (AvailableFormat.format     == SwapChain.ImageFormat &&
                    AvailableFormat.colorSpace == SwapChain.ColorSpace)
                { bFoundRequiredFormatAndColorSpace = True; break; }
            }
            if (!bFoundRequiredFormatAndColorSpace)
            { throw SRuntimeError("Failed to find required format and color space for swapchain images!"); }

        }

        const auto SurfaceCapabilities = GPU.Context.getSurfaceCapabilitiesKHR(Surface);
        // Swap Chain Image Extent
        {
            if (SurfaceCapabilities.currentExtent.width != Math::UpperBound<UInt32>())
            {
                SwapChain.Extent = SurfaceCapabilities.currentExtent;
            }
            else
            {
                SwapChain.Extent = vk::Extent2D{ GWindow->GetWidth(), GWindow->GetHeight() };

                Math::Clamp(&SwapChain.Extent.width,
                            SurfaceCapabilities.minImageExtent.width,
                            SurfaceCapabilities.maxImageExtent.width);
                Math::Clamp(&SwapChain.Extent.height,
                            SurfaceCapabilities.minImageExtent.height,
                            SurfaceCapabilities.maxImageExtent.height);
            }
        }

        SwapChain.MinimalImageCount = Math::Max(SurfaceCapabilities.minImageCount,
                                                SwapChain.MinimalImageCount);

        const auto CreateInfo =  vk::SwapchainCreateInfoKHR{}
            .setSurface         (Surface)
            .setMinImageCount   (SwapChain.MinimalImageCount)
            .setImageFormat     (SwapChain.ImageFormat)
            .setImageColorSpace (SwapChain.ColorSpace)
            .setImageExtent     (SwapChain.Extent)
            .setImageUsage      (SwapChain.ImageUsage)
            .setImageSharingMode(SwapChain.SharingMode)
            .setImageArrayLayers(1U)
            .setPreTransform    (SurfaceCapabilities.currentTransform)
            .setCompositeAlpha  (SwapChain.CompositeAlpha)
            .setPresentMode     (SwapChain.PresentMode)
            .setClipped         (SwapChain.bClipped);

        SwapChain.Context = vk::raii::SwapchainKHR(Device.Context, CreateInfo);

        LOG_DEBUG("Created a SwapChain (extent:[{},{}]).",
                   SwapChain.Extent.width, SwapChain.Extent.height);

        SwapChain.Images = SwapChain.Context.getImages();
        constexpr auto ImageSubresourceRage = vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        constexpr auto ComponentSwizzle = vk::ComponentMapping{}
            .setR(vk::ComponentSwizzle::eIdentity)
            .setG(vk::ComponentSwizzle::eIdentity)
            .setB(vk::ComponentSwizzle::eIdentity)
            .setA(vk::ComponentSwizzle::eIdentity);

        auto ImageViewCreateInfo = vk::ImageViewCreateInfo{}
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(SwapChain.ImageFormat)
            .setComponents(ComponentSwizzle)
            .setSubresourceRange(ImageSubresourceRage);

        for (const auto& Image : SwapChain.Images)
        {
            ImageViewCreateInfo.image = Image;
            SwapChain.ImageViews.emplace_back(Device.Context, ImageViewCreateInfo);
        }
    }

    void FVulkan::
    CollectDeviceLayersAndExtensions()
    {
        this->AddDeviceExtension(vk::EXTDescriptorIndexingExtensionName)
            ->AddDeviceExtension(vk::KHRSynchronization2ExtensionName)
            ->AddDeviceExtension(vk::KHRDynamicRenderingExtensionName)
            ->AddDeviceExtension(vk::KHRMaintenance1ExtensionName)
#if defined(VISERA_ON_APPLE_SYSTEM)
            ->AddDeviceExtension("VK_KHR_portability_subset")
#endif
        ;
        if (GWindow->IsBootstrapped())
        { this->AddDeviceExtension(vk::KHRSwapchainExtensionName); }
    }
}
