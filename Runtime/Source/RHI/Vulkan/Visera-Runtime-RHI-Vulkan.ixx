module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
#if defined(CreateSemaphore)
#undef CreateSemaphore
#endif
#if !defined(VISERA_OFFSCREEN_MODE)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Vulkan.Common;
export import Visera.Runtime.RHI.Vulkan.CommandBuffer;
export import Visera.Runtime.RHI.Vulkan.Image;
export import Visera.Runtime.RHI.Vulkan.Buffer;
       import Visera.Runtime.RHI.Vulkan.Loader;
       import Visera.Runtime.RHI.Vulkan.Allocator;
       import Visera.Runtime.RHI.Vulkan.Fence;
       import Visera.Runtime.RHI.Vulkan.Semaphore;
       import Visera.Runtime.RHI.Vulkan.ShaderModule;
       import Visera.Runtime.RHI.Vulkan.PipelineCache;
       import Visera.Runtime.RHI.Vulkan.RenderPass;
       import Visera.Runtime.RHI.Vulkan.RenderTarget;
       import Visera.Runtime.RHI.Vulkan.DescriptorSet;
       import Visera.Runtime.RHI.SPIRV;
       import Visera.Runtime.Platform;
       import Visera.Runtime.Window;
       import Visera.Core.Log;
       import Visera.Core.Math.Arithmetic;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.Name;

namespace Visera::RHI
{
    export using EVulkanMemoryPoolFlag = EVulkanMemoryPoolFlagBits;

    export class VISERA_RUNTIME_API FVulkanDriver
    {
    public:
        vk::ApplicationInfo              AppInfo;
        vk::raii::Context                Context;

        vk::raii::Instance               Instance        {nullptr};
        vk::raii::DebugUtilsMessengerEXT DebugMessenger  {nullptr};
#if !defined(VISERA_OFFSCREEN_MODE)
        vk::raii::SurfaceKHR             Surface         {nullptr};
#endif

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

#if !defined(VISERA_OFFSCREEN_MODE)
        struct
        {
            vk::raii::SwapchainKHR  Context     {nullptr};
            vk::raii::SwapchainKHR  OldContext  {nullptr};
            vk::Extent2D            Extent      {0U, 0U};
            TArray<TSharedPtr<FVulkanImageWrapper>> Images     {}; // SwapChain manages Images so do NOT use RAII here.
            TArray<TSharedPtr<FVulkanImageView>>    ImageViews {};
            TArray<TSharedPtr<FVulkanSemaphore>>    ReadyToPresentSemaphores;
            UInt32                                  Cursor     {0};
            vk::ImageUsageFlags     ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eTransferDst};
            vk::Format              ImageFormat {EVulkanFormat::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR       ColorSpace  {EVulkanColorSpace::eSrgbNonlinear};
            UInt32                  MinimalImageCount{3};
            vk::PresentModeKHR      PresentMode {EVulkanPresentMode::eMailbox};
            vk::SharingMode         SharingMode {EVulkanSharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                          bClipped       {True};
        }SwapChain;
#endif

        struct FInFlightFrame
        {
            TSharedPtr<FVulkanFence>         Fence;
            TSharedPtr<FVulkanRenderTarget>  ColorRT;
            TSharedPtr<FVulkanCommandBuffer> DrawCalls;
#if !defined(VISERA_OFFSCREEN_MODE)
            TSharedPtr<FVulkanSemaphore>     SwapChainImageAvailable;
#endif
        };

    public:
        [[nodiscard]] inline Bool
        BeginFrame();
        [[nodiscard]] inline const FInFlightFrame&
        GetCurrentFrame() { return InFlightFrames[InFlightFrameIndex]; }
        [[nodiscard]] inline Bool
        EndFrame();
        [[nodiscard]] inline Bool
        Present();
        inline void
        Submit(TSharedRef<FVulkanCommandBuffer> I_CommandBuffer,
               TSharedRef<FVulkanSemaphore>     I_WaitSemaphore,
               TSharedRef<FVulkanSemaphore>     I_SignalSemaphore,
               TSharedRef<FVulkanFence>         I_Fence = {});
        [[nodiscard]] TSharedPtr<FVulkanShaderModule>
        CreateShaderModule(TSharedRef<FSPIRVShader> I_Shader);
        [[nodiscard]] TSharedPtr<FVulkanRenderPass>
        CreateRenderPass(const FName&                    I_Name,
                         TSharedRef<FVulkanShaderModule> I_VertexShader,
                         TSharedRef<FVulkanShaderModule> I_FragmentShader);
        [[nodiscard]] TSharedPtr<FVulkanFence>
        CreateFence(Bool        I_bSignaled,
                    FStringView I_Description);
        [[nodiscard]] TSharedPtr<FVulkanSemaphore>
        CreateSemaphore(FStringView I_Name);
        [[nodiscard]] TSharedPtr<FVulkanImage>
        CreateImage(EVulkanImageType       I_ImageType,
                    const FVulkanExtent3D& I_Extent,
                    EVulkanFormat          I_Format,
                    EVulkanImageUsage      I_Usages);
        [[nodiscard]] TSharedPtr<FVulkanBuffer>
        CreateBuffer(UInt64                I_Size,
                     EVulkanBufferUsage    I_Usage,
                     EVulkanMemoryPoolFlags I_MemoryPoolFlags = EVulkanMemoryPoolFlagBits::eNone);
        [[nodiscard]] TSharedPtr<FVulkanCommandBuffer>
        CreateCommandBuffer(EVulkanQueue I_Queue);
        [[nodiscard]] const vk::raii::DescriptorPool&
        GetDescriptorPool() const { return DescriptorPool; }
        [[nodiscard]] const TUniqueRef<FVulkanPipelineCache>
        GetPipelineCache() const { return PipelineCache; }
        [[nodiscard]] UInt32
        GetFrameCount() const { return InFlightFrames.size(); }
        [[nodiscard]] inline const vk::raii::Instance&
        GetNativeInstance() const { return Instance;       };
        [[nodiscard]] inline const vk::raii::Device&
        GetNativeDevice() const { return Device.Context; };
        void inline
        WaitIdle() const { auto Result = Device.Context.waitIdle(); }

    private:
        const FVulkanExtent2D                       ColorRTRes { 1920, 1080 };
        TArray<FInFlightFrame>                      InFlightFrames;
        UInt8                                       InFlightFrameIndex {0};

        TMap<FString, TSharedPtr<FVulkanSemaphore>> SemaphorePool;
        vk::raii::DescriptorPool                    DescriptorPool      {nullptr};
        vk::raii::CommandPool                       GraphicsCommandPool {nullptr};

        TArray<TSharedPtr<FVulkanRenderPass>>       RenderPasses;
        TUniquePtr<FVulkanPipelineCache>            PipelineCache;

        TArray<const char*> InstanceLayers;
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceLayers;
        TArray<const char*> DeviceExtensions;

    private:
        void inline CreateInstance();       void inline DestroyInstance();
        void inline CreateDebugMessenger(); void inline DestroyDebugMessenger();
        void inline CreateSurface();        void inline DestroySurface();
        void inline PickPhysicalDevice();
        void inline CreateDevice();         void inline DestroyDevice();
        void inline CreateSwapChain();      void inline DestroySwapChain();
        void inline CreateCommandPools();   void inline DestroyCommandPools();
        void inline CreateDescriptorPools(); void inline DestroyDescriptorPools();
        void inline CreatePipelineCache();  void inline DestroyPipelineCache();
        void inline CreateInFlightFrames(); void inline DestroyInFlightFrames();
        void inline CreateOtherResource();  void inline DestroyOtherResource();

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
            if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            { LOG_ERROR("{}", I_CallbackData->pMessage); }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            { LOG_WARN("{}", I_CallbackData->pMessage); }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            { LOG_DEBUG("{}", I_CallbackData->pMessage); }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
            { LOG_TRACE("{}", I_CallbackData->pMessage); }
            else
            { LOG_ERROR("Unknown Vulkan Debug Message Severity {}", static_cast<Int32>(I_Severity)); }
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
        .setPApplicationName    (reinterpret_cast<const char*>(VISERA_APP))
        .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName         (reinterpret_cast<const char*>(u8"Visera"))
        .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion          (vk::ApiVersion13);

#if defined(VISERA_ON_APPLE_SYSTEM)
        if (!GPlatform->SetEnvironmentVariable(
            "VK_ICD_FILENAMES",
            VISERA_VULKAN_SDK_PATH "/share/vulkan/icd.d/MoltenVK_icd.json"))
        { LOG_FATAL("Failed to set \"VK_ICD_FILENAMES\"!"); }
        if (!GPlatform->SetEnvironmentVariable(
            "VK_LAYER_PATH",
            VISERA_VULKAN_SDK_PATH "/share/vulkan/explicit_layer.d"))
        { LOG_FATAL("Failed to set \"VK_LAYER_PATH\"!"); }
#endif

        Loader = MakeUnique<FVulkanLoader>();
        CollectInstanceLayersAndExtensions();
        CreateInstance();
        Loader->Load(Instance);
        CreateDebugMessenger();
        CreateSurface();
        CollectDeviceLayersAndExtensions();
        PickPhysicalDevice();
        CreateDevice();
        Loader->Load(Device.Context);
        CreateCommandPools();
        CreateDescriptorPools();
        GVulkanAllocator = MakeUnique<FVulkanAllocator>
        (
            AppInfo.apiVersion,
            Instance,
            GPU.Context,
            Device.Context
        );
        CreateSwapChain();
        CreatePipelineCache();
        CreateInFlightFrames();
        CreateOtherResource();
    };

    FVulkanDriver::
    ~FVulkanDriver()
    {
        WaitIdle();
        DestroyOtherResource();
        DestroyInFlightFrames();
        DestroyPipelineCache();
        DestroyDescriptorPools();
        DestroyCommandPools();
        DestroySwapChain();
        DestroyDevice();
        GVulkanAllocator.reset();
        DestroySurface();
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
#if !defined(VISERA_OFFSCREEN_MODE)
        Surface.clear();
#endif
    }

    void FVulkanDriver::
    DestroyDevice()
    {
        Device.Context.clear();
    }

    void FVulkanDriver::
    DestroySwapChain()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        SwapChain.ReadyToPresentSemaphores.clear();
        SwapChain.ImageViews.clear();
        SwapChain.Images.clear();
        SwapChain.Context.clear();
#endif
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
    DestroyOtherResource()
    {
        SemaphorePool.clear();
        RenderPasses.clear();
    }

    void FVulkanDriver::
    DestroyInFlightFrames()
    {
        InFlightFrames.clear();
    }

    void FVulkanDriver::
    DestroyDescriptorPools()
    {
        DescriptorPool.clear();
    }

    void FVulkanDriver::
    CreateDescriptorPools()
    {
        enum : UInt32 { SampledImage, CombinedImageSampler, UniformBuffer, MAX_DESCRIPTOR_ENUM };

        vk::DescriptorPoolSize PoolSizes[MAX_DESCRIPTOR_ENUM];
        PoolSizes[SampledImage]
        .setType            (EVulkanDescriptorType::eSampledImage)
        .setDescriptorCount (100);
        PoolSizes[CombinedImageSampler]
        .setType            (EVulkanDescriptorType::eCombinedImageSampler)
        .setDescriptorCount (100);
        PoolSizes[UniformBuffer]
        .setType            (EVulkanDescriptorType::eUniformBuffer)
        .setDescriptorCount (100);

        auto CreateInfo = vk::DescriptorPoolCreateInfo()
            .setFlags           (vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets         (GPU.Context.getProperties().limits.maxBoundDescriptorSets)
            .setPoolSizeCount   (MAX_DESCRIPTOR_ENUM)
            .setPPoolSizes      (PoolSizes)
        ;
        auto Result = Device.Context.createDescriptorPool(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the Vulkan Descriptor Pool!"); }
        else
        { DescriptorPool = std::move(*Result); }

        LOG_DEBUG("Created a Vulkan Descriptor Pool: (max_sets:{})",
                  CreateInfo.maxSets);
    }

    void FVulkanDriver::
    CreateOtherResource()
    {

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
#if !defined(VISERA_OFFSCREEN_MODE)
        VkSurfaceKHR SurfaceHandle {nullptr};
        auto& W = GWindow;
        VISERA_ASSERT(GWindow->GetType() == EWindowType::GLFW);

        if(glfwCreateWindowSurface(
            *Instance,
            static_cast<GLFWwindow*>(GWindow->GetHandle()),
            nullptr,
            &SurfaceHandle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create Vulkan Surface!"); }

        Surface = vk::raii::SurfaceKHR(Instance, SurfaceHandle);
#endif
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

#if !defined(VISERA_OFFSCREEN_MODE)
                VISERA_ASSERT(Surface != nullptr);
                auto Result = PhysicalDeviceCandidate.getSurfaceSupportKHR(Idx, *Surface);
                if (Result.has_value())
                { GPU.PresentQueueFamilies.insert(std::move(*Result)); }
#endif
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

        auto Vulkan11Features =  vk::PhysicalDeviceVulkan11Features{};
        Vulkan11Features.shaderDrawParameters = vk::True;

        vk::StructureChain FeatureChain{
            Feature2,
            Vulkan13Features,
            ExtendedDynamicsStateFeatures,
            Vulkan11Features
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

#if !defined(VISERA_OFFSCREEN_MODE)
        if (!glfwVulkanSupported())
        { LOG_FATAL("The Window does NOT support Vulkan!"); }

        UInt32 WindowExtensionCount = 0;
        auto WindowExtensions = glfwGetRequiredInstanceExtensions(&WindowExtensionCount);

        for (UInt32 Idx = 0; Idx < WindowExtensionCount; ++Idx)
        {
            this->AddInstanceExtension(WindowExtensions[Idx]);
        }
#endif
    }

    void FVulkanDriver::
    CreateSwapChain()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
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
            {
                LOG_ERROR("Failed to find required present mode \"{}\" for SwapChain!"
                          "-- Using FIFO present mode by default.", SwapChain.PresentMode);
                SwapChain.PresentMode = EVulkanPresentMode::eFifo;
            }
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
            auto Fence = CreateFence(False, "convert swapchain image layout");

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
            Submit(Cmd, {}, {}, Fence);

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
        // Create Semaphores
        SwapChain.ReadyToPresentSemaphores.resize(SwapChain.Images.size());
        for (UInt8 Idx = 0; Idx < SwapChain.Images.size(); ++Idx)
        {
            SwapChain.ReadyToPresentSemaphores[Idx]
            = CreateSemaphore(Format("Ready to Present ({})", Idx));
        }
#endif
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
        auto& CurrentFrame = InFlightFrames[InFlightFrameIndex];
        if (!CurrentFrame.Fence->Wait()) { return False; }

        CurrentFrame.DrawCalls->Reset();
        CurrentFrame.DrawCalls->Begin();

#if !defined(VISERA_OFFSCREEN_MODE)
        auto NextImageAcquireInfo = vk::AcquireNextImageInfoKHR{}
            .setSwapchain   (SwapChain.Context)
            .setTimeout     (Math::UpperBound<UInt64>())
            .setSemaphore   (CurrentFrame.SwapChainImageAvailable->GetHandle())
            .setFence       (nullptr)
            .setDeviceMask  (1)
        ;
        auto NextImageIndex = Device.Context.acquireNextImage2KHR(NextImageAcquireInfo);
        if (!NextImageIndex.has_value())
        { LOG_FATAL("Failed to acquire the next Vulkan SwapChain Image!"); }

        SwapChain.Cursor = *NextImageIndex;
#endif
        // Switch Render Targets
        auto& CurrentColorRT = CurrentFrame.ColorRT;
        for (auto& RenderPass : RenderPasses)
        {
            RenderPass->SetColorRT(CurrentColorRT);
        }

        return CurrentFrame.Fence->Reset();
    }

    Bool FVulkanDriver::
    EndFrame()
    {
        auto& CurrentFrame = InFlightFrames[InFlightFrameIndex];

#if !defined(VISERA_OFFSCREEN_MODE)
        auto& CurrentColorRT = CurrentFrame.ColorRT;
        auto OldColorRTLayout   = CurrentColorRT->GetLayout();
        auto& CurrentSwapChainImage = SwapChain.Images[SwapChain.Cursor];
        auto OldSwapChainLayout = CurrentSwapChainImage->GetLayout();

        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentColorRT->GetImage(),
            EVulkanImageLayout::eTransferSrcOptimal,
            EVulkanPipelineStage::eTopOfPipe,
            EVulkanAccess::eNone,
            EVulkanPipelineStage::eTransfer,
            EVulkanAccess::eTransferWrite
        );

        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentSwapChainImage,
            EVulkanImageLayout::eTransferDstOptimal,
            EVulkanPipelineStage::eTopOfPipe,
            EVulkanAccess::eNone,
            EVulkanPipelineStage::eTransfer,
            EVulkanAccess::eTransferWrite
        );

        CurrentFrame.DrawCalls->BlitImage(CurrentColorRT->GetImage(),
                       CurrentSwapChainImage);

        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentColorRT->GetImage(),
            OldColorRTLayout,
            EVulkanPipelineStage::eTopOfPipe,
            EVulkanAccess::eNone,
            EVulkanPipelineStage::eTransfer,
            EVulkanAccess::eTransferWrite
        );

        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentSwapChainImage,
            OldSwapChainLayout,
            EVulkanPipelineStage::eTopOfPipe,
            EVulkanAccess::eNone,
            EVulkanPipelineStage::eTransfer,
            EVulkanAccess::eTransferWrite
        );

        CurrentFrame.DrawCalls->End();

        Submit(CurrentFrame.DrawCalls,
               CurrentFrame.SwapChainImageAvailable,
               SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor],
               CurrentFrame.Fence);

        return True;
#else
        CurrentFrame.DrawCalls->End();

        Submit(CurrentFrame.DrawCalls,
               {},
               {},
               CurrentFrame.Fence);

        return CurrentFrame.Fence->Wait();
#endif
    }

    Bool FVulkanDriver::
    Present()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        auto PresentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount  (1)
            .setPWaitSemaphores     (&(*SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor]->GetHandle()))
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

        InFlightFrameIndex = (InFlightFrameIndex + 1) % (InFlightFrames.size());
        return True;
#else
        return False;
#endif
    }

    void FVulkanDriver::
    Submit(TSharedRef<FVulkanCommandBuffer> I_CommandBuffer,
           TSharedRef<FVulkanSemaphore>     I_WaitSemaphore,
           TSharedRef<FVulkanSemaphore>     I_SignalSemaphore,
           TSharedRef<FVulkanFence>         I_Fence /* = {} */)
    {
        VISERA_ASSERT(I_CommandBuffer->IsReadyToSubmit());

        auto CommandBuffer = I_CommandBuffer? *I_CommandBuffer->GetHandle() : nullptr;
        auto WaitSemaphore = I_WaitSemaphore? *I_WaitSemaphore->GetHandle() : nullptr;
        auto SignalSemaphore = I_SignalSemaphore? *I_SignalSemaphore->GetHandle() : nullptr;
        auto Fence = I_Fence? *I_Fence->GetHandle() : nullptr;

        auto WaitSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(WaitSemaphore)
            .setStageMask(EVulkanPipelineStage::eColorAttachmentOutput)
        ;
        auto SignalSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(SignalSemaphore)
            .setStageMask(EVulkanPipelineStage::eColorAttachmentOutput)
        ;
        auto CommandBufferInfo = vk::CommandBufferSubmitInfo{}
            .setCommandBuffer(CommandBuffer)
        ;
        auto SubmitInfo = vk::SubmitInfo2{}
            .setWaitSemaphoreInfoCount  (WaitSemaphore? 1 : 0)
            .setPWaitSemaphoreInfos     (&WaitSemaphoreInfo)
            .setSignalSemaphoreInfoCount(SignalSemaphore? 1 : 0)
            .setPSignalSemaphoreInfos   (&SignalSemaphoreInfo)
            .setCommandBufferInfoCount  (1)
            .setPCommandBufferInfos    (&CommandBufferInfo)
        ;
        Device.GraphicsQueue.submit2(SubmitInfo, Fence);
    }

    TSharedPtr<FVulkanShaderModule> FVulkanDriver::
    CreateShaderModule(TSharedRef<FSPIRVShader> I_Shader)
    {
        VISERA_ASSERT(!I_Shader->IsEmpty());
        LOG_TRACE("Creating a Vulkan Shader Module.");
        return MakeShared<FVulkanShaderModule>(Device.Context, I_Shader);
    }

    TSharedPtr<FVulkanRenderPass> FVulkanDriver::
    CreateRenderPass(const FName&                    I_Name,
                     TSharedRef<FVulkanShaderModule> I_VertexShader,
                     TSharedRef<FVulkanShaderModule> I_FragmentShader)
    {
        LOG_TRACE("Creating a Vulkan Render Pass (name:{}).", I_Name);
        return RenderPasses.emplace_back(MakeShared<FVulkanRenderPass>(
               I_Name,
               Device.Context,
               I_VertexShader,
               I_FragmentShader,
               PipelineCache));
    }

    TSharedPtr<FVulkanFence> FVulkanDriver::
    CreateFence(Bool        I_bSignaled,
                FStringView I_Description)
    {
        LOG_TRACE("Creating a Vulkan Fence (description:{}, signaled:{}).", I_Description, I_bSignaled);
        return MakeShared<FVulkanFence>(Device.Context, I_bSignaled, I_Description);
    }

    TSharedPtr<FVulkanSemaphore> FVulkanDriver::
    CreateSemaphore(FStringView I_Name)
    {
        LOG_TRACE("Creating a Vulkan Semaphore (name: {}).", I_Name);

        auto& Semaphore = SemaphorePool[I_Name.data()];
        if (Semaphore == nullptr)
        { Semaphore = MakeShared<FVulkanSemaphore>(Device.Context); }
        else
        { LOG_FATAL("Semaphore \"{}\" already exists!", I_Name); }

        return Semaphore;
    }

    TSharedPtr<FVulkanImage> FVulkanDriver::
    CreateImage(EVulkanImageType       I_ImageType,
                const FVulkanExtent3D& I_Extent,
                EVulkanFormat          I_Format,
                EVulkanImageUsage      I_Usages)
    {
        VISERA_ASSERT(I_Extent.width && I_Extent.height && I_Extent.depth);
        LOG_DEBUG("Creating a Vulkan Image (extent:[{},{},{}]).",
                  I_Extent.width, I_Extent.height, I_Extent.depth);
        return MakeShared<FVulkanImage>(I_ImageType, I_Extent, I_Format, I_Usages);
    }

    TSharedPtr<FVulkanBuffer> FVulkanDriver::
    CreateBuffer(UInt64                I_Size,
                 EVulkanBufferUsage    I_Usage,
                 EVulkanMemoryPoolFlags I_MemoryPoolFlags /* = EVulkanMemoryPoolFlagBits::eNone */)
    {
        VISERA_ASSERT(I_Size != 0);
        LOG_DEBUG("Creating a Vulkan Buffer ({} bytes).", I_Size);
        return MakeShared<FVulkanBuffer>(I_Size, I_Usage, I_MemoryPoolFlags);
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

    void FVulkanDriver::
    CreateInFlightFrames()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        InFlightFrames.resize(SwapChain.Images.size());
#else
        InFlightFrames.resize(1);
#endif

        LOG_DEBUG("Creating {} in-flght frames (extent: [{},{}]).",
                  InFlightFrames.size(), ColorRTRes.width, ColorRTRes.height);

        auto Cmd = CreateCommandBuffer(EVulkanQueue::eGraphics);
        auto Fence = CreateFence(False, "convert render targets layout");
        Cmd->Begin();
        {
            UInt8 Index{0};
            for (auto& InFlightFrame : InFlightFrames)
            {
                auto TargetImage = CreateImage(
                    EVulkanImageType::e2D,
                    {ColorRTRes.width, ColorRTRes.height, 1},
                    FVulkanRenderPass::ColorRTFormat,
                    vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferSrc);

                Cmd->ConvertImageLayout(TargetImage,
                    EVulkanImageLayout::eColorAttachmentOptimal,
                    EVulkanPipelineStage::eTopOfPipe,
                    EVulkanAccess::eNone,
                    EVulkanPipelineStage::eColorAttachmentOutput,
                    EVulkanAccess::eColorAttachmentWrite
                );
                InFlightFrame.ColorRT = MakeShared<FVulkanRenderTarget>(TargetImage);
                InFlightFrame.ColorRT->SetLoadOp(EVulkanLoadOp::eClear)
                                     ->SetStoreOp(EVulkanStoreOp::eStore);

                // Command Buffers
                InFlightFrame.DrawCalls = CreateCommandBuffer(EVulkanQueue::eGraphics);
                // Fences
                InFlightFrame.Fence
                = CreateFence(True, Format("In-Flight Fence ({})", InFlightFrameIndex));
                // Semaphores
#if !defined(VISERA_OFFSCREEN_MODE)
                InFlightFrame.SwapChainImageAvailable
                = CreateSemaphore(Format("SwapChain Image Available ({})", InFlightFrameIndex));
#endif
                InFlightFrameIndex = (InFlightFrameIndex + 1) % InFlightFrames.size();
            }
        }
        Cmd->End();
        Submit(Cmd, {}, {}, Fence);

        if (!Fence->Wait())
        { LOG_FATAL("Failed to convert the layout of color render targets!"); }
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