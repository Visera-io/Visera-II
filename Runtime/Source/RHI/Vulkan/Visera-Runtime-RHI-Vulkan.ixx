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
       import Visera.Runtime.Platform;
       import Visera.Runtime.Window;
       import Visera.Core.Log;
       import Visera.Core.Math.Arithmetic;
       import Visera.Core.Types.Path;

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
            TArray<TSharedPtr<FVulkanImageWrapper>> Images     {}; // SwapChain manages Images so do NOT use RAII here.
            TArray<TSharedPtr<FVulkanImageView>>    ImageViews {};
            UInt32                                  Cursor     {0};
            TArray<vk::raii::Semaphore>             PresentSemaphores;
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

        const FVulkanExtent2D                   ColorRTRes { 1920, 1080 };
        TArray<TSharedPtr<FVulkanFence>>        InFlightFences     {};
        TArray<TSharedPtr<FVulkanRenderTarget>> ColorRTs;
        vk::raii::CommandPool        GraphicsCommandPool {nullptr};

        TUniquePtr<FVulkanPipelineCache> PipelineCache;

    private:
        TArray<const char*> InstanceLayers;
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceLayers;
        TArray<const char*> DeviceExtensions;

    public:
        [[nodiscard]] inline Bool
        BeginFrame();
        [[nodiscard]] inline Bool
        EndFrame();
        [[nodiscard]] inline Bool
        Present();
        inline void
        Submit(TSharedPtr<FVulkanCommandBuffer> I_CommandBuffer,
               TSharedPtr<FVulkanFence>         I_Fence = {});
        [[nodiscard]] TSharedPtr<FVulkanShaderModule>
        CreateShaderModule(TSharedPtr<FShader> I_Shader);
        [[nodiscard]] TSharedPtr<FVulkanRenderPass>
        CreateRenderPass(const FText&                    I_Name,
                         TSharedRef<FVulkanShaderModule> I_VertexShader,
                         TSharedRef<FVulkanShaderModule> I_FragmentShader);
        [[nodiscard]] TSharedPtr<FVulkanFence>
        CreateFence(Bool I_bSignaled);
        [[nodiscard]] TSharedPtr<FVulkanImage>
        CreateImage(EVulkanImageType       I_ImageType,
                    const FVulkanExtent3D& I_Extent,
                    EVulkanFormat          I_Format,
                    EVulkanImageUsage      I_Usages);
        [[nodiscard]] TSharedPtr<FVulkanCommandBuffer>
        CreateCommandBuffer(EVulkanQueue I_Queue);
        [[nodiscard]] UInt32
        GetFrameCount() const { return SwapChain.Images.size(); }
        [[nodiscard]] inline const vk::raii::Instance&
        GetNativeInstance() const { return Instance;       };
        [[nodiscard]] inline const vk::raii::Device&
        GetNativeDevice() const { return Device.Context; };

    private:
        void inline CreateInstance();       void inline DestroyInstance();
        void inline CreateDebugMessenger(); void inline DestroyDebugMessenger();
        void inline CreateSurface();        void inline DestroySurface();
        void inline PickPhysicalDevice();
        void inline CreateDevice();         void inline DestroyDevice();
        void inline CreateSwapChain();      void inline DestroySwapChain();
        void inline CreateCommandPools();   void inline DestroyCommandPools();
        void inline CreatePipelineCache();  void inline DestroyPipelineCache();
        void inline CreateRenderTargets();  void inline DestroyRenderTargets();

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

        CollectInstanceLayersAndExtensions();

        CreateInstance();
        Loader->Load(Instance);

        CreateDebugMessenger();

        if (GWindow->IsBootstrapped())
        {
            CreateSurface();
        }

        CollectDeviceLayersAndExtensions();

        PickPhysicalDevice();
        CreateDevice();
        Loader->Load(Device.Context);

        CreateCommandPools();

        GVulkanAllocator = MakeUnique<FVulkanAllocator>
        (
            AppInfo.apiVersion,
            Instance,
            GPU.Context,
            Device.Context
        );

        if (GWindow->IsBootstrapped())
        {
            CreateSwapChain();
        }

        CreateRenderTargets();

        CreatePipelineCache();
    };

    FVulkanDriver::
    ~FVulkanDriver()
    {
        Device.Context.waitIdle();

        DestroyPipelineCache();

        DestroyRenderTargets();

        DestroyCommandPools();

        if (GWindow->IsBootstrapped())
        {
            DestroySwapChain();
        }

        DestroyDevice();

        GVulkanAllocator.reset();

        if (GWindow->IsBootstrapped())
        {
            DestroySurface();
        }

        DestroyDebugMessenger();
        DestroyInstance();

        Loader.reset();
    }

    void FVulkanDriver::
    DestroyInstance()
    {
        Instance.clear();
    }

    void FVulkanDriver::
    DestroyDebugMessenger()
    {
        DebugMessenger.clear();
    }

    void FVulkanDriver::
    DestroySurface()
    {
        Surface.clear();
    }

    void FVulkanDriver::
    DestroyDevice()
    {
        Device.Context.clear();
    }

    void FVulkanDriver::
    DestroySwapChain()
    {
        SwapChain.PresentSemaphores.clear();
        SwapChain.ImageViews.clear();
        SwapChain.Images.clear();
        SwapChain.Context.clear();
    }

    void FVulkanDriver::
    DestroyCommandPools()
    {
        GraphicsCommandPool.clear();
    }

    void FVulkanDriver::
    DestroyPipelineCache()
    {
        PipelineCache.reset();
    }

    void FVulkanDriver::
    DestroyRenderTargets()
    {
        ColorRTs.clear();
        InFlightFences.clear();
    }

    void FVulkanDriver::
    CreateRenderTargets()
    {
        UInt8 ColorRTCount = Math::Max(SwapChain.Images.size(), static_cast<size_t>(1));
        LOG_DEBUG("Creating Color Render Targets (count: {}, extent:[{},{}]).",
                  ColorRTCount, ColorRTRes.width, ColorRTRes.height);

        auto Cmd = CreateCommandBuffer(EVulkanQueue::eGraphics);
        auto Fence = CreateFence(False);
        Cmd->Begin();
        {
            for (UInt8 Idx = 0; Idx < ColorRTCount; ++Idx)
            {
                auto RTImage = CreateImage(EVulkanImageType::e2D,
                    {ColorRTRes.width, ColorRTRes.height, 1},
                    FVulkanRenderPass::ColorRTFormat,
                    vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferSrc);
                Cmd->ConvertImageLayout(RTImage, EVulkanImageLayout::eColorAttachmentOptimal,
                    EVulkanPipelineStage::eTopOfPipe,
                    EVulkanAccess::eNone,
                    EVulkanPipelineStage::eColorAttachmentOutput,
                    EVulkanAccess::eColorAttachmentWrite
                );
                ColorRTs.push_back(MakeShared<FVulkanRenderTarget>(RTImage));
            }
        }
        Cmd->End();
        Submit(Cmd, Fence);

        if (!Fence->Wait())
        { LOG_FATAL("Failed to convert the layout of color render targets!"); }

        InFlightFences.resize(ColorRTCount);
        for (auto& InFlightFence : InFlightFences)
        {
            InFlightFence = std::move(CreateFence(True)); // Init as Signaled
        }
    }

    void FVulkanDriver::
    CreateInstance()
    {
        // Check Layers
        {
            auto Result = Context.enumerateInstanceLayerProperties();
            if (!Result.has_value())
            { LOG_FATAL("Failed to enumerate instance layer properties!"); }

            const auto LayerProperties = std::move(*Result);
            for (const auto& Layer : InstanceLayers)
            {
                if (std::ranges::none_of(LayerProperties,
                    [&Layer](auto const& LayerProperty)
                    { return strcmp(LayerProperty.layerName, Layer) == 0; }))
                {
                    LOG_FATAL("Required instance layer \"{}\" is not supported!", Layer);
                }
            }
        }

        // Check Extensions
        {
            auto Result = Context.enumerateInstanceExtensionProperties();
            if (!Result.has_value())
            { LOG_FATAL("Failed to enumerate instance extension properties!"); }

            auto ExtensionProperties = std::move(*Result);
            for (const auto& Extension : InstanceExtensions)
            {
                if (std::ranges::none_of(ExtensionProperties,
                    [&Extension](auto const& ExtensionProperty)
                    { return strcmp(ExtensionProperty.extensionName, Extension) == 0; }))
                {
                    LOG_FATAL("Required instance extension \"{}\" is not supported!", Extension);
                }
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
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Instance!"); }
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
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Debug Messenger!"); }
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
        if (!Result.has_value())
        { LOG_FATAL("Failed to find PhysicalDevices with Vulkan support!"); }

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
                    auto Result = PhysicalDeviceCandidate.getSurfaceSupportKHR(Idx, *Surface);
                    if (Result.has_value())
                    { GPU.PresentQueueFamilies.insert(std::move(*Result)); }
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
            auto Result = PhysicalDeviceCandidate.enumerateDeviceExtensionProperties();
            if (!Result.has_value())
            {LOG_FATAL("Failed to enumerate device extension properties!"); }

            auto Extensions = std::move(*Result);
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
        auto Feature2 = vk::PhysicalDeviceFeatures2{};

        auto Vulkan13Features =  vk::PhysicalDeviceVulkan13Features{};
        Vulkan13Features.dynamicRendering = vk::True;
        Vulkan13Features.synchronization2 = vk::True;

        auto ExtendedDynamicsStateFeatures = vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{};
        ExtendedDynamicsStateFeatures.extendedDynamicState = vk::True;

#if defined(VISERA_ON_APPLE_SYSTEM)
        auto Vulkan11Features =  vk::PhysicalDeviceVulkan11Features{};
        Vulkan11Features.shaderDrawParameters = vk::True;
#endif

        vk::StructureChain FeatureChain{
            Feature2,
            Vulkan13Features,
            ExtendedDynamicsStateFeatures,
#if defined(VISERA_ON_APPLE_SYSTEM)
            Vulkan11Features,
#endif
        };

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
            if (!Result.has_value())
            { LOG_FATAL("Failed to create Vulkan Device!"); }
            else
            { Device.Context = std::move(*Result); }
        }
        // Get Queues
        {
            Device.GraphicsQueue = Device.Context.getQueue(*GPU.GraphicsQueueFamilies.begin(), 0);
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
            auto Result = GPU.Context.getSurfacePresentModesKHR(Surface);
            if (!Result.has_value())
            { LOG_FATAL("Failed to get the required Present Mode!"); }

            for (const auto PresentModes = std::move(*Result);
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
            auto Result = GPU.Context.getSurfaceFormatsKHR(Surface);
            if (!Result.has_value())
            { LOG_FATAL("Failed to get the required format or color space!"); }

            for (const auto AvailableFormats = std::move(*Result);
                 const auto& AvailableFormat : AvailableFormats)
            {
                if (AvailableFormat.format     == SwapChain.ImageFormat &&
                    AvailableFormat.colorSpace == SwapChain.ColorSpace)
                { bFoundRequiredFormatAndColorSpace = True; break; }
            }
            if (!bFoundRequiredFormatAndColorSpace)
            { LOG_FATAL("Failed to find required format and color space for swapchain images!"); }

        }

        auto Result = GPU.Context.getSurfaceCapabilitiesKHR(Surface);
        if (!Result.has_value())
        { LOG_FATAL("Failed to get the required surface capabilities!"); }

        const auto SurfaceCapabilities = std::move(*Result);
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

        // Create Swapchain
        {
            auto Result = Device.Context.createSwapchainKHR(CreateInfo);
            if (!Result.has_value())
            { LOG_FATAL("Failed to create Vulkan SwapChain!"); }
            else
            { SwapChain.Context = std::move(*Result); }
        }

        LOG_TRACE("Created a SwapChain (extent:[{},{}]).",
                   SwapChain.Extent.width, SwapChain.Extent.height);

        // Retrieve Swapchain Images
        {
            auto Result = SwapChain.Context.getImages();
            if (!Result.has_value())
            { LOG_FATAL("Failed to retrieve Vulkan Swapchain Images!"); }

            TArray<vk::Image> SwapChainImages = std::move(*Result);
            for (UInt32 Idx = 0; Idx < SwapChainImages.size(); Idx++)
            {
                SwapChain.Images.emplace_back(MakeShared<FVulkanImageWrapper>(
                    SwapChainImages[Idx],
                    EVulkanImageType::e2D,
                    FVulkanExtent3D{SwapChain.Extent.width, SwapChain.Extent.height, 1},
                    SwapChain.ImageFormat,
                    SwapChain.ImageUsage));
            }
        }
        // Convert Swapchain Images' layout
        {
            auto Cmd = CreateCommandBuffer(EVulkanQueue::eGraphics);
            auto Fence = CreateFence(False);

            Cmd->Begin();
            {
                for (auto& Image : SwapChain.Images)
                {
                    Cmd->ConvertImageLayout(Image,
                        EVulkanImageLayout::ePresentSrcKHR,
                        EVulkanPipelineStage::eTopOfPipe,
                        EVulkanAccess::eNone,
                        EVulkanPipelineStage::eBottomOfPipe,
                        EVulkanAccess::eNone
                    );
                }
            }
            Cmd->End();
            Submit(Cmd, Fence);

            if (!Fence->Wait())
            { LOG_FATAL("Failed to wait for convert swapchain image layouts!"); }
        }
        // Create Image Views
        for (const auto& Image : SwapChain.Images)
        {
            SwapChain.ImageViews.emplace_back(MakeShared<FVulkanImageView>(
                Image,
                EVulkanImageViewType::e2D,
                EVulkanImageAspect::eColor));
        }

        // Create Present Semaphores
        SwapChain.PresentSemaphores.reserve(SwapChain.Images.size());
        for (auto& Image : SwapChain.Images)
        {
            auto Result = Device.Context.createSemaphore({});
            if (!Result.has_value())
            { LOG_FATAL("Failed to create Vulkan Semaphore for swapchain presentation!"); }
            else
            { SwapChain.PresentSemaphores.emplace_back(std::move(*Result)); }
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

    Bool FVulkanDriver::
    BeginFrame()
    {
        auto& Fence = InFlightFences[SwapChain.Cursor];
        if (!Fence->Wait())
        {
            LOG_ERROR("Failed to wait for the frame({})!", SwapChain.Cursor);
            return False;
        }
        auto NextImageAcquireInfo = vk::AcquireNextImageInfoKHR{}
            .setSwapchain   (SwapChain.Context)
            .setTimeout     (Math::UpperBound<UInt64>())
            .setSemaphore   (SwapChain.PresentSemaphores[SwapChain.Cursor])
            .setFence       (nullptr)
            .setDeviceMask  (1)
        ;
        auto Result = Device.Context.acquireNextImage2KHR(NextImageAcquireInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to acquire the next Vulkan SwapChain Image!"); }
        auto Status = static_cast<vk::Result>(Result.value);
        if (Status == vk::Result::eErrorOutOfDateKHR ||
            Status == vk::Result::eSuboptimalKHR)
        {
            LOG_WARN("Failed to acquire next swapchain image -- RECREATION!");
            return False;
        }

        return Fence->Reset();
    }

    Bool FVulkanDriver::
    EndFrame()
    {
        auto& Fence = InFlightFences[SwapChain.Cursor];

        auto Cmd = CreateCommandBuffer(EVulkanQueue::eGraphics);
        Cmd->Begin();
        {
            auto OldColorRTLayout   = ColorRTs[0]->GetLayout();
            auto OldSwapChainLayout = SwapChain.Images[0]->GetLayout();

            Cmd->ConvertImageLayout(
                ColorRTs[0]->GetHandle(),
                EVulkanImageLayout::eTransferSrcOptimal,
                EVulkanPipelineStage::eTopOfPipe,
                EVulkanAccess::eNone,
                EVulkanPipelineStage::eTransfer,
                EVulkanAccess::eTransferWrite
            );

            Cmd->ConvertImageLayout(
                SwapChain.Images[0],
                EVulkanImageLayout::eTransferDstOptimal,
                EVulkanPipelineStage::eTopOfPipe,
                EVulkanAccess::eNone,
                EVulkanPipelineStage::eTransfer,
                EVulkanAccess::eTransferWrite
            );

            Cmd->BlitImage(ColorRTs[0]->GetHandle(),
                           SwapChain.Images[0]);

            Cmd->ConvertImageLayout(
                ColorRTs[0]->GetHandle(),
                OldColorRTLayout,
                EVulkanPipelineStage::eTopOfPipe,
                EVulkanAccess::eNone,
                EVulkanPipelineStage::eTransfer,
                EVulkanAccess::eTransferWrite
            );

            Cmd->ConvertImageLayout(
                SwapChain.Images[0],
                OldSwapChainLayout,
                EVulkanPipelineStage::eTopOfPipe,
                EVulkanAccess::eNone,
                EVulkanPipelineStage::eTransfer,
                EVulkanAccess::eTransferWrite
            );
        }
        Cmd->End();

        Submit(Cmd, Fence);
        return Fence->Wait();
    }

    Bool FVulkanDriver::
    Present()
    {
        auto PresentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount  (0)
            .setPWaitSemaphores     (nullptr)
            .setSwapchainCount      (1)
            .setPSwapchains         (&(*SwapChain.Context))
            .setPImageIndices       (&SwapChain.Cursor)
            .setPResults            (nullptr) // NOT necessary if using a single swapchain.
        ;
        const auto Result = Device.GraphicsQueue.presentKHR(PresentInfo);
        if (Result == vk::Result::eErrorOutOfDateKHR ||
            Result == vk::Result::eSuboptimalKHR)
        {
            LOG_WARN("Failed to present current swapchain image -- RECREATION!");
            return False;
        }

        SwapChain.Cursor = (SwapChain.Cursor + 1) % SwapChain.Images.size();
        return True;
    }

    void FVulkanDriver::
    Submit(TSharedPtr<FVulkanCommandBuffer> I_CommandBuffer,
           TSharedPtr<FVulkanFence>         I_Fence /* = {} */)
    {
        VISERA_ASSERT(I_CommandBuffer->IsReadyToSubmit());

        auto& CommandBuffer = I_CommandBuffer->GetHandle();
        auto  Fence = I_Fence? *I_Fence->GetHandle() : nullptr;

        auto SubmitInfo = vk::SubmitInfo{}
            .setCommandBufferCount(1)
            .setPCommandBuffers(&(*CommandBuffer))
        ;
        Device.GraphicsQueue.submit(SubmitInfo, Fence);
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
    CreateRenderPass(const FText&                    I_Name,
                     TSharedRef<FVulkanShaderModule> I_VertexShader,
                     TSharedRef<FVulkanShaderModule> I_FragmentShader)
    {
        LOG_DEBUG("Creating a Vulkan Render Pass (name:{}, vertex:{}, fragment:{})",
                  I_Name, I_VertexShader->GetPath(), I_FragmentShader->GetPath());
        auto RenderPass = MakeShared<FVulkanRenderPass>(
               I_Name,
               Device.Context,
               I_VertexShader,
               I_FragmentShader,
               PipelineCache);
        RenderPass->SetColorRT(ColorRTs[SwapChain.Cursor]);
        return RenderPass;
    }

    TSharedPtr<FVulkanFence> FVulkanDriver::
    CreateFence(Bool I_bSignaled)
    {
        LOG_TRACE("Creating a Vulkan Fence (signaled:{})", I_bSignaled);
        return MakeShared<FVulkanFence>(Device.Context, I_bSignaled);
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
            if (!Result.has_value())
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
            PipelineCache = MakeUnique<FVulkanPipelineCache>(
                GPU.Context,
                Device.Context,
                GPlatform->GetExecutableDirectory() / PATH("VulkanPipelines.cache")
            );
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
