module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Vulkan.Common;
       import Visera.Runtime.RHI.Vulkan.Loader;
       import Visera.Runtime.RHI.Vulkan.Allocator;
       import Visera.Runtime.RHI.Vulkan.Fence;
       import Visera.Runtime.RHI.Vulkan.ShaderModule;
       import Visera.Runtime.RHI.Vulkan.PipelineCache;
       import Visera.Runtime.RHI.Vulkan.RenderPass;
       import Visera.Runtime.RHI.Vulkan.CommandBuffer;
       import Visera.Runtime.RHI.Vulkan.RenderTarget;
       import Visera.Runtime.RHI.Vulkan.Image;
       import Visera.Runtime.AssetHub.Shader;
       import Visera.Runtime.Window;
       import Visera.Core.Log;
       import Visera.Core.Math.Arithmetic;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanDriver
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

        TUniquePtr<FVulkanLoader>    Loader;

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

        vk::raii::CommandPool GraphicsCommandPool {nullptr};

        TUniquePtr<FVulkanPipelineCache> PipelineCache;

    private:
        TArray<const char*> InstanceLayers;
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceLayers;
        TArray<const char*> DeviceExtensions;

    public:
        inline void
        BeginFrame() {};
        inline void
        EndFrame() {};
        inline void
        Present()  {};
        [[nodiscard]] TSharedPtr<FVulkanShaderModule>
        CreateShaderModule(TSharedPtr<FShader> I_Shader);
        [[nodiscard]] TSharedPtr<FVulkanRenderPass>
        CreateRenderPass(TSharedPtr<FVulkanShaderModule> I_VertexShader,
                         TSharedPtr<FVulkanShaderModule> I_FragmentShader);
        [[nodiscard]] TUniquePtr<FVulkanFence>
        CreateFence(Bool I_bSignaled);
        [[nodiscard]] TSharedPtr<FVulkanImage>
        CreateImage(EVulkanImageType       I_ImageType,
                    const FVulkanExtent3D& I_Extent,
                    EVulkanFormat          I_Format,
                    EVulkanImageUsage      I_Usages);
        [[nodiscard]] TSharedPtr<FVulkanRenderTarget>
        CreateRenderTarget(const FVulkanExtent2D& I_Extent);
        [[nodiscard]] TSharedPtr<FVulkanCommandBuffer>
        CreateCommandBuffer(EVulkanQueue I_Queue);
        [[nodiscard]] UInt32
        GetFrameCount() const { return SwapChain.Images.size(); }
        [[nodiscard]] inline const vk::raii::Instance&
        GetNativeInstance() const { return Instance;       };
        [[nodiscard]] inline const vk::raii::Device&
        GetNativeDevice() const { return Device.Context; };

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateDevice();
        void CreateSwapChain();
        void CreateCommandPools();
        void CreatePipelineCache();

        inline FVulkanDriver*
        AddInstanceLayer(const char* I_Layer)           { InstanceLayers.emplace_back(I_Layer);         return this; }
        inline FVulkanDriver*
        AddInstanceExtension(const char* I_Extension)   { InstanceExtensions.push_back(I_Extension);    return this; }
        inline FVulkanDriver*
        AddDeviceLayer(const char* I_Layer)             { DeviceLayers.push_back(I_Layer);              return this; }
        inline FVulkanDriver*
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
                LOG_TRACE("{}", I_CallbackData->pMessage);
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
                LOG_ERROR("Unknown Vulkan Debug Message Severity {}", static_cast<Int32>(I_Severity));
            }
            return vk::False; // Always return VK_FALSE
        }

    public:
        FVulkanDriver();
        ~FVulkanDriver();
    };

    FVulkanDriver::
    FVulkanDriver()
    {
        AppInfo = vk::ApplicationInfo{}
        .setPApplicationName    ("Visera")
        .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName         ("Visera")
        .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion          (vk::ApiVersion13);

#if defined(VISERA_ON_APPLE_SYSTEM)
        if (setenv("VK_ICD_FILENAMES",
                   VISERA_VULKAN_SDK_PATH "/share/vulkan/icd.d/MoltenVK_icd.json", True) != 0)
        { LOG_ERROR("Failed to set \"VK_ICD_FILENAMES\"!"); }
        if (setenv("VK_LAYER_PATH",
                   VISERA_VULKAN_SDK_PATH "/share/vulkan/explicit_layer.d", True) != 0)
        { LOG_ERROR("Failed to set \"VK_LAYER_PATH\"!"); }
#endif

        Loader = MakeUnique<FVulkanLoader>();

        // Instance
        {
            CollectInstanceLayersAndExtensions();

            CreateInstance();
            Loader->Load(Instance);

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
            Loader->Load(Device.Context);
        }

        // Allocator
        {
            GVulkanAllocator = MakeUnique<FVulkanAllocator>
            (
                AppInfo.apiVersion,
                Instance,
                GPU.Context,
                Device.Context
            );
        }

        // Swap Chain
        if (GWindow->IsBootstrapped())
        {
            CreateSwapChain();
        }

        // Command Pools
        {
            CreateCommandPools();
        }

        // Pipeline Cache
        {
            CreatePipelineCache();
        }
    };

    FVulkanDriver::
    ~FVulkanDriver()
    {
        Device.Context.waitIdle();

        // Pipeline Cache
        {
            PipelineCache.reset();
        }

        // Command Pools
        {
            GraphicsCommandPool.clear();
        }

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
            GVulkanAllocator.reset();
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

        Loader.reset();
    }

    void FVulkanDriver::
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
                LOG_FATAL("Required instance layer \"{}\" is not supported!", Layer);
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
                LOG_FATAL("Required instance extension \"{}\" is not supported!", Extension);
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

        auto Result = Context.createInstance(CreateInfo);
        if (!Result)
        { LOG_FATAL("Failed to create Vulkan Instance: {}", static_cast<Int32>(Result.error())); }
        else
        { Instance = std::move(*Result); }
    }

    void FVulkanDriver::
    CreateDebugMessenger()
    {
#if !defined(VISERA_RELEASE_MODE)
        auto CreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
            .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose    |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning  |
                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                                )
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral       |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                            )
            .setPfnUserCallback(DebugCallback);
        auto Result = Instance.createDebugUtilsMessengerEXT(CreateInfo);
        if (!Result)
        { LOG_FATAL("Failed to create Vulkan Debug Messenger: {}", static_cast<Int32>(Result.error())); }
        else
        { DebugMessenger = std::move(*Result); }
#endif
    }

    void FVulkanDriver::
    CreateSurface()
    {
        VkSurfaceKHR SurfaceHandle {nullptr};

        VISERA_ASSERT(GWindow->GetType() == EWindowType::GLFW);

        if(glfwCreateWindowSurface(
            *Instance,
            static_cast<GLFWwindow*>(GWindow->GetHandle()),
            nullptr,
            &SurfaceHandle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create Vulkan Surface!"); }

        Surface = vk::raii::SurfaceKHR(Instance, SurfaceHandle);
    }

    void FVulkanDriver::
    PickPhysicalDevice()
    {
        VISERA_ASSERT(Instance != nullptr);

        auto Result = Instance.enumeratePhysicalDevices();
        if (!Result)
        {
            LOG_FATAL("Failed to find PhysicalDevices with Vulkan support (error:{})!",
                      static_cast<Int32>(Result.error()));
        }

        auto PhysicalDeviceCandidates = std::move(*Result);

        // Sort: Discrete GPU first, then Integrated, then others
        std::ranges::sort(PhysicalDeviceCandidates,
            [](auto const& A, auto const& B)
            {
                auto AType = A.getProperties().deviceType;
                auto BType = B.getProperties().deviceType;

                auto Rank = [](vk::PhysicalDeviceType Type)
                {
                    switch (Type)
                    {
                    case vk::PhysicalDeviceType::eDiscreteGpu:    return 0;
                    case vk::PhysicalDeviceType::eIntegratedGpu:  return 1;
                    default:                                      return 2;
                    }
                };

                return Rank(AType) < Rank(BType);
            });

        const auto SelectedPhysicalDevice = std::ranges::find_if(
            PhysicalDeviceCandidates, [&](auto const& PhysicalDeviceCandidate)
        {
            auto PhysicalDeviceInfo = PhysicalDeviceCandidate.getProperties();
            LOG_TRACE("Current GPU candidate: {}.", PhysicalDeviceInfo.deviceName.data());

            LOG_TRACE("Checking Vulkan API Version...");
            Bool bSuitable = PhysicalDeviceInfo.apiVersion >= AppInfo.apiVersion;
            if (!bSuitable) { return False; }

            LOG_TRACE("Checking Queue Families...");
            auto QueueFamilies = PhysicalDeviceCandidate.getQueueFamilyProperties();

            GPU.GraphicsQueueFamilies.clear();
            GPU.ComputeQueueFamilies.clear();
            GPU.TransferQueueFamilies.clear();
            GPU.PresentQueueFamilies.clear();

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
            if (GWindow->IsBootstrapped())
            {
                bSuitable &= !GPU.PresentQueueFamilies.empty();
            }

            if (!bSuitable) { return False; }

            LOG_TRACE("Checking Extension supporting...");
            auto Extensions = PhysicalDeviceCandidate.enumerateDeviceExtensionProperties();
            for (auto const& RequiredExtension : DeviceExtensions)
            {
                auto ExtensionIter = std::ranges::find_if(
                    Extensions,
                    [RequiredExtension](auto const& Extension)
                    {
                        return strcmp(Extension.extensionName, RequiredExtension) == 0;
                    });
                bSuitable &= (ExtensionIter != Extensions.end());
            }

            if (bSuitable)
            {
                GPU.Context = PhysicalDeviceCandidate;
            }
            return bSuitable;
        });

        if (SelectedPhysicalDevice == PhysicalDeviceCandidates.end())
        {
            LOG_FATAL("Failed to find a suitable PhysicalDevice!");
        }

        LOG_INFO("Selected GPU: {}", GPU.Context.getProperties().deviceName.data());
    }

    void FVulkanDriver::
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
        //Create Device
        {
            auto Result = GPU.Context.createDevice(CreateInfo);
            if (!Result)
            { LOG_FATAL("Failed to create Vulkan Device -- error:{}!", static_cast<Int32>(Result.error())); }
            else
            { Device.Context = std::move(*Result); }
        }
        // Get Queues
        {
            auto Result = Device.Context.getQueue(*GPU.GraphicsQueueFamilies.begin(), 0);
            if (!Result)
            {
                LOG_FATAL("Failed to get graphics queue (index:{}) -- error:{}!",
                        0,
                        static_cast<Int32>(Result.error()));
            }
            else
            { Device.GraphicsQueue = std::move(*Result); }
        }
    }


    void FVulkanDriver::
    CollectInstanceLayersAndExtensions()
    {
        // Layers
        this
#if !defined(VISERA_RELEASE_MODE)
            ->AddInstanceLayer("VK_LAYER_KHRONOS_validation")
            ->AddInstanceLayer("VK_LAYER_KHRONOS_synchronization2")
        ;
#else

#endif
        ;

        // Extensions
        this
#if !defined(VISERA_RELEASE_MODE)
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
            { LOG_FATAL("The Window does NOT support Vulkan!"); }

            UInt32 WindowExtensionCount = 0;
            auto WindowExtensions = glfwGetRequiredInstanceExtensions(&WindowExtensionCount);

            for (UInt32 Idx = 0; Idx < WindowExtensionCount; ++Idx)
            {
                this->AddInstanceExtension(WindowExtensions[Idx]);
            }
        }
    }

    void FVulkanDriver::
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
            { LOG_FATAL("Failed to find required present mode for SwapChain!"); }
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
            { LOG_FATAL("Failed to find required format and color space for swapchain images!"); }

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

        auto Result = Device.Context.createSwapchainKHR(CreateInfo);
        if (!Result)
        {
            LOG_FATAL("Failed to create Vulkan SwapChain -- error:{}!",
                      static_cast<Int32>(Result.error()));
        }
        else
        { SwapChain.Context = std::move(*Result); }

        LOG_TRACE("Created a SwapChain (extent:[{},{}]).",
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
            auto Result = Device.Context.createImageView(ImageViewCreateInfo);
            if (!Result)
            {
                LOG_FATAL("Failed to create Vulkan ImageView -- error:{}!",
                          static_cast<Int32>(Result.error()));
            }
            SwapChain.ImageViews.push_back(std::move(*Result));
        }
    }

    void FVulkanDriver::
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

    TSharedPtr<FVulkanShaderModule> FVulkanDriver::
    CreateShaderModule(TSharedPtr<FShader> I_Shader)
    {
        VISERA_ASSERT(!I_Shader->IsEmpty());
        LOG_TRACE("Creating a Vulkan Shader Module (shader:{})",
                  I_Shader->GetName());
        return MakeShared<FVulkanShaderModule>(Device.Context, I_Shader);
    }

    TSharedPtr<FVulkanRenderPass> FVulkanDriver::
    CreateRenderPass(TSharedPtr<FVulkanShaderModule> I_VertexShader,
                     TSharedPtr<FVulkanShaderModule> I_FragmentShader)
    {
        LOG_DEBUG("Creating a Vulkan Render Pass (vertex:{}, fragment:{})",
                  I_VertexShader->GetName(), I_FragmentShader->GetName());
        return MakeUnique<FVulkanRenderPass>(Device.Context,
               I_VertexShader,
               I_FragmentShader,
               PipelineCache);
    }

    TUniquePtr<FVulkanFence> FVulkanDriver::
    CreateFence(Bool I_bSignaled)
    {
        LOG_TRACE("Creating a Vulkan Fence (signaled:{})", I_bSignaled);
        return MakeUnique<FVulkanFence>(Device.Context, I_bSignaled);
    }

    TSharedPtr<FVulkanImage> FVulkanDriver::
    CreateImage(EVulkanImageType       I_ImageType,
                const FVulkanExtent3D& I_Extent,
                EVulkanFormat          I_Format,
                EVulkanImageUsage      I_Usages)
    {
        VISERA_ASSERT(I_Extent.width && I_Extent.height && I_Extent.depth);
        LOG_TRACE("Creating a Vulkan Image (extent:[{},{},{}]).",
                  I_Extent.width, I_Extent.height, I_Extent.depth);
        return MakeShared<FVulkanImage>(I_ImageType, I_Extent, I_Format, I_Usages);
    }

    TSharedPtr<FVulkanRenderTarget> FVulkanDriver::
    CreateRenderTarget(const vk::Extent2D& I_Extent)
    {
        LOG_TRACE("Creating a Vulkan Render Target");
        VISERA_WIP;
        return {};
    }

    void FVulkanDriver::
    CreateCommandPools()
    {
        LOG_TRACE("Creating a Vulkan Graphics Command Pool.");
        {
            auto CreateInfo = vk::CommandPoolCreateInfo{}
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                .setQueueFamilyIndex(*GPU.GraphicsQueueFamilies.begin())
            ;
            auto Result = Device.Context.createCommandPool(CreateInfo);
            if (!Result)
            { LOG_FATAL("Failed to create the Vulkan Graphics Command Pool!"); }
            else
            { GraphicsCommandPool = std::move(*Result); }
        }
    }

    void FVulkanDriver::
    CreatePipelineCache()
    {
        LOG_TRACE("Creating a Vulkan Pipeline Cache.");
        {
            PipelineCache = MakeUnique<FVulkanPipelineCache>(PATH("VulkanPipelines.cache"));
        }
    }

    TSharedPtr<FVulkanCommandBuffer> FVulkanDriver::
    CreateCommandBuffer(EVulkanQueue I_Queue)
    {
        switch (I_Queue)
        {
        case EVulkanQueue::eGraphics:
            LOG_TRACE("Creating a new Vulkan Graphics Command Buffer.");
            return MakeShared<FVulkanCommandBuffer>(
                Device.Context,
                GraphicsCommandPool);
        default:
            LOG_FATAL("Invalid Command Buffer Type!");
            return {};
        }

    }
}
