module;
#include <Visera-RHI.hpp>
#if defined(CreateSemaphore)
#undef CreateSemaphore
#endif
#if !defined(VISERA_OFFSCREEN_MODE)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
export module Visera.RHI.Vulkan;
#define VISERA_MODULE_NAME "RHI.Vulkan"
export import Visera.RHI.Vulkan.Common;
export import Visera.RHI.Vulkan.CommandBuffer;
export import Visera.RHI.Vulkan.Image;
export import Visera.RHI.Vulkan.Buffer;
export import Visera.RHI.Vulkan.Sampler;
export import Visera.RHI.Vulkan.Pipeline;
export import Visera.RHI.Vulkan.RenderTarget;
export import Visera.RHI.Vulkan.DescriptorSet;
export import Visera.RHI.Vulkan.DescriptorSetLayout;
       import Visera.RHI.Vulkan.Loader;
       import Visera.RHI.Vulkan.Allocator;
       import Visera.RHI.Vulkan.Sync;
       import Visera.RHI.Vulkan.ShaderModule;
       import Visera.RHI.Types.Shader;
       import Visera.Platform;
       import Visera.Runtime.Log;
       import Visera.Core.Math.Arithmetic;
       import Visera.Core.Types.Path;
       import Visera.Runtime.Name;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Set;
       import Visera.Core.Types.Array;
       import Visera.Core.Traits.Flags;
       import vulkan_hpp;

namespace Visera
{
    export using EVulkanMemoryPoolFlags = EVMAMemoryPoolFlags;

    export class VISERA_RHI_API FVulkanDriver
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
            vk::PhysicalDeviceProperties  Properties;
            vk::PhysicalDeviceProperties2 Properties2;
            vk::PhysicalDeviceDescriptorIndexingProperties DescriptorIndexingProperties;
            vk::PhysicalDeviceFeatures    Features;
            vk::PhysicalDeviceFeatures2   Features2;
        }GPU;

        struct
        {
            vk::raii::Device Context        {nullptr};
            vk::raii::Queue  GraphicsQueue  {nullptr};
            vk::raii::Queue  TransferQueue  {nullptr};
        }Device;

        TUniquePtr<FVulkanLoader>    Loader;

#if !defined(VISERA_OFFSCREEN_MODE)
        struct
        {
            vk::raii::SwapchainKHR  Context     {nullptr};
            vk::raii::SwapchainKHR  OldContext  {nullptr};
            vk::Extent2D            Extent      {0U, 0U};
            TArray<FVulkanImageWrapper> Images     {}; // SwapChain manages Images so do NOT use RAII here.
            TArray<FVulkanImageView>    ImageViews {};
            TArray<FVulkanSemaphore>    ReadyToPresentSemaphores;
            UInt32                      Cursor     {0};
            vk::ImageUsageFlags     ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eTransferDst};
            vk::Format              ImageFormat {vk::Format::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR       ColorSpace  {vk::ColorSpaceKHR::eSrgbNonlinear};
            UInt32                  MinimalImageCount{3};
            vk::PresentModeKHR      PresentMode {vk::PresentModeKHR::eMailbox};
            vk::SharingMode         SharingMode {vk::SharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                          bClipped       {True};

            [[nodiscard]] inline FVulkanImageView*
            GetCurrentImageView() { return &ImageViews[Cursor]; }
        }SwapChain;
#endif

        struct FInFlightFrame
        {
            // Store as value types for async GPU execution safety
            // These resources must remain valid until GPU completes execution
            FVulkanFence         Fence;
            FVulkanImage         ColorRTImage;      // Internal resource, not registered
            FVulkanImageView     ColorRTImageView; // Internal resource, not registered
            FVulkanRenderTarget  ColorRT;
            FVulkanCommandBuffer DrawCalls;
#if !defined(VISERA_OFFSCREEN_MODE)
            FVulkanSemaphore     SwapChainImageAvailable;
#endif
        };

    public:
        [[nodiscard]] inline Bool
        BeginFrame();
        [[nodiscard]] inline FInFlightFrame&
        GetCurrentFrame() { return InFlightFrames[InFlightFrameIndex % InFlightFrames.size()]; }
        [[nodiscard]] inline UInt64
        GetFrameIndex() { return InFlightFrameIndex; }
        inline void
        EndFrame();
        [[nodiscard]] inline Bool
        Present();
        inline void
        Submit(FVulkanCommandBuffer* I_CommandBuffer,
               FVulkanSemaphore*     I_WaitSemaphore,
               FVulkanSemaphore*     I_SignalSemaphore,
               FVulkanFence*         I_Fence = nullptr);
        [[nodiscard]] FVulkanShaderModule*
        CreateShaderModule(const TArray<FByte>& I_SPIRVShader);
        [[nodiscard]] FVulkanPipelineLayout*
        CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                             const TArray<vk::PushConstantRange>&   I_PushConstants);
        [[nodiscard]] FVulkanRenderPipeline*
        CreateRenderPipeline(FVulkanPipelineLayout* I_PipelineLayout,
                             FVulkanShaderModule*   I_VertexShader,
                             FVulkanShaderModule*   I_FragmentShader);
        [[nodiscard]] FVulkanComputePipeline*
        CreateComputePipeline(FVulkanPipelineLayout* I_PipelineLayout,
                              FVulkanShaderModule*   I_ComputeShader);
        [[nodiscard]] FVulkanFence
        CreateFence(Bool        I_bSignaled,
                    FStringView I_Description);
        [[nodiscard]] FVulkanSemaphore
        CreateSemaphore(FStringView I_Name);
        // Resource creation (returns resources by value for registration)
        [[nodiscard]] FVulkanImage
        CreateImage(vk::ImageType       I_ImageType,
                    const vk::Extent3D& I_Extent,
                    vk::Format          I_Format,
                    vk::ImageUsageFlags     I_Usages);
        [[nodiscard]] FVulkanImageView
        CreateImageView(FVulkanImage*                  I_Image,
                         vk::ImageViewType               I_ViewType,
                         vk::ImageAspectFlags            I_Aspect,
                         TClosedInterval<UInt8>          I_MipmapRange = {0,0},
                         TClosedInterval<UInt8>          I_ArrayRange  = {0,0},
                         TOptional<vk::ComponentMapping> I_Swizzle     = {});
        [[nodiscard]] FVulkanSampler
        CreateImageSampler(vk::Filter             I_Filter,
                           vk::SamplerAddressMode I_AddressMode,
                           Float                  I_MaxAnisotropy = 1.0);
        [[nodiscard]] FVulkanSampler
        CreateCompareSampler(vk::Filter      I_Filter,
                             vk::CompareOp   I_CompareOp,
                             vk::BorderColor I_BorderColor);
        [[nodiscard]] FVulkanBuffer
        CreateBuffer(UInt64                  I_Size,
                     vk::BufferUsageFlags    I_Usages,
                     EVulkanMemoryPoolFlags  I_MemoryPoolFlags = EVulkanMemoryPoolFlags::None);
        
        [[nodiscard]] FVulkanCommandBuffer
        CreateCommandBuffer(vk::QueueFlagBits I_Queue);
        [[nodiscard]] FVulkanDescriptorSetLayout*
        CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
        [[nodiscard]] FVulkanDescriptorSet*
        CreateDescriptorSet(FVulkanDescriptorSetLayout* I_DescriptorSetLayout);
        [[nodiscard]] const vk::raii::DescriptorPool&
        GetDescriptorPool() const { return DescriptorPool; }
        [[nodiscard]] const TUniqueRef<FVulkanPipelineCache>
        GetPipelineCache() const { return PipelineCache; }
        [[nodiscard]] UInt32
        GetFrameCount() const { return InFlightFrames.size(); }
        [[nodiscard]] inline const vk::raii::Instance&
        GetNativeInstance() const { return Instance; };
        [[nodiscard]] inline const vk::raii::Device&
        GetNativeDevice() const { return Device.Context; };
        void inline
        WaitIdle() const { auto Result = Device.Context.waitIdle(); }
    private:
        const vk::Extent2D                  ColorRTRes { 1920, 1080 };
        TArray<FInFlightFrame>              InFlightFrames;
        UInt64                              InFlightFrameIndex {0};

        vk::raii::DescriptorPool            DescriptorPool      {nullptr};
        vk::raii::CommandPool               GraphicsCommandPool {nullptr};

        TArray<FVulkanShaderModule>         ShaderModules;
        TArray<FVulkanPipelineLayout>       PipelineLayouts;
        TArray<FVulkanDescriptorSetLayout>  DescriptorSetLayouts;
        TArray<FVulkanRenderPipeline>       RenderPipelines;
        TArray<FVulkanComputePipeline>      ComputePipelines;
        TUniquePtr<FVulkanPipelineCache>    PipelineCache;

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
        void inline CreateDescriptorPools();void inline DestroyDescriptorPools();
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
#if defined(VISERA_ON_APPLE_SYSTEM)
        auto VulkanICDPath = FPath{ GPlatform->GetResourceDirectory() / FPath{"Vulkan/MoltenVK_icd.json"}}.GetUTF8Path();
        if (!GPlatform->SetEnvironmentVariable(
            "VK_ICD_FILENAMES", VulkanICDPath))
        { LOG_FATAL("Failed to set \"VK_ICD_FILENAMES\" as {}!", VulkanICDPath); }
#if !defined(VISERA_RELEASE_MODE)
        auto VulkanLayerPath = FPath{ GPlatform->GetResourceDirectory() / FPath{"Vulkan"}}.GetUTF8Path();
        if (!GPlatform->SetEnvironmentVariable(
            "VK_LAYER_PATH", VulkanLayerPath))
        { LOG_FATAL("Failed to set \"VK_LAYER_PATH\" as {}!", VulkanLayerPath); }
#endif
#endif
        AppInfo = vk::ApplicationInfo{}
        .setPApplicationName    ("Visera")
        .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName         ("Visera")
        .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion          (vk::ApiVersion13)
        ;
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
        // Resources are owned by registry at RHI level, not by driver
        DestroyOtherResource();
        DestroyInFlightFrames();
        DestroyPipelineCache();
        DestroySwapChain();
        GVulkanAllocator.reset();
        DestroyDescriptorPools();
        DestroyCommandPools();
        DestroyDevice();
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
        ShaderModules.clear();
        PipelineLayouts.clear();
        RenderPipelines.clear();
        ComputePipelines.clear();
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
        // Query bindless descriptor limits
        const auto MaxBindlessDescriptors       = GPU.DescriptorIndexingProperties.maxUpdateAfterBindDescriptorsInAllPools;
        const auto MaxBindlessSampledImages     = GPU.DescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages;
        const auto MaxBindlessStorageImages     = GPU.DescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageImages;
        const auto MaxBindlessStorageBuffers    = GPU.DescriptorIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers;

        // Use device limits for bindless, but clamp to reasonable defaults if limits are very high
        constexpr UInt32 MaxReasonableBindlessCount     = 100000;
        const auto BindlessSampledImageCount    = Math::Min(MaxBindlessSampledImages, MaxReasonableBindlessCount);
        const auto BindlessStorageImageCount    = Math::Min(MaxBindlessStorageImages, MaxReasonableBindlessCount);
        const auto BindlessStorageBufferCount   = Math::Min(MaxBindlessStorageBuffers, MaxReasonableBindlessCount);
        const auto BindlessCombinedImageSamplerCount = BindlessSampledImageCount; // Combined uses sampled image slot

        enum : UInt32 { SampledImage, StorageImage, CombinedImageSampler, UniformBuffer, StorageBuffer, MAX_DESCRIPTOR_ENUM };

        vk::DescriptorPoolSize PoolSizes[MAX_DESCRIPTOR_ENUM];
        PoolSizes[SampledImage]
        .setType            (vk::DescriptorType::eSampledImage)
        .setDescriptorCount (BindlessSampledImageCount);
        PoolSizes[StorageImage]
        .setType            (vk::DescriptorType::eStorageImage)
        .setDescriptorCount (BindlessStorageImageCount);
        PoolSizes[CombinedImageSampler]
        .setType            (vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount (BindlessCombinedImageSamplerCount);
        PoolSizes[UniformBuffer]
        .setType            (vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount (100); // Traditional binding, not bindless
        PoolSizes[StorageBuffer]
        .setType            (vk::DescriptorType::eStorageBuffer)
        .setDescriptorCount (BindlessStorageBufferCount);

        // Clamp max sets to device limit for bindless
        const auto MaxSets = Math::Min(
            GPU.Properties.limits.maxBoundDescriptorSets,
            MaxBindlessDescriptors > 0 ? MaxBindlessDescriptors / 1000 : GPU.Properties.limits.maxBoundDescriptorSets
        );

        auto CreateInfo = vk::DescriptorPoolCreateInfo()
            .setFlags           (vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
                                 vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets         (MaxSets)
            .setPoolSizeCount   (MAX_DESCRIPTOR_ENUM)
            .setPPoolSizes      (PoolSizes)
        ;
        auto Result = Device.Context.createDescriptorPool(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the Vulkan Descriptor Pool!"); }
        else
        { DescriptorPool = std::move(*Result); }

        LOG_TRACE("Created a Vulkan Descriptor Pool for bindless: (max_sets:{}, max_bindless_descriptors:{}, sampled_images:{}, storage_images:{}, storage_buffers:{})",
                  CreateInfo.maxSets, MaxBindlessDescriptors, BindlessSampledImageCount, BindlessStorageImageCount, BindlessStorageBufferCount);
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
                auto Result = PhysicalDeviceCandidate.getSurfaceSupportKHR(Idx, *Surface);
                if (Result.has_value() && *Result)
                { GPU.PresentQueueFamilies.insert(Idx); }
#endif
            }
            bSuitable = !GPU.GraphicsQueueFamilies.empty() &&
                        !GPU.ComputeQueueFamilies.empty()  &&
                        !GPU.TransferQueueFamilies.empty();
#if !defined(VISERA_OFFSCREEN_MODE)
            bSuitable &= !GPU.PresentQueueFamilies.empty();
#endif
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
        { LOG_FATAL("Failed to find a suitable PhysicalDevice!"); }

        GPU.Properties  = GPU.Context.getProperties();
        auto Properties2 = GPU.Context.getProperties2<
                        vk::PhysicalDeviceProperties2,
                        vk::PhysicalDeviceDescriptorIndexingProperties>();
        GPU.Properties2 = std::move(Properties2.get<vk::PhysicalDeviceProperties2>());
        GPU.DescriptorIndexingProperties = std::move(Properties2.get<vk::PhysicalDeviceDescriptorIndexingProperties>());
        GPU.Features    = GPU.Context.getFeatures();
        GPU.Features2   = GPU.Context.getFeatures2();
        LOG_INFO("Selected GPU: {}", GPU.Properties.deviceName.data());
    }

    void FVulkanDriver::
    CreateDevice()
    {
        constexpr Float Priority = 0.0f;

        TArray<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfos(1);
        auto GraphicsFamilyIter  = GPU.GraphicsQueueFamilies.begin();
        auto GraphicsFamilyIndex = *GraphicsFamilyIter;
        DeviceQueueCreateInfos[0] = vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(GraphicsFamilyIndex)
            .setQueueCount      (1)
            .setQueuePriorities ({Priority})
        ;
        auto TransferFamilyIter  = GPU.TransferQueueFamilies.begin();
        auto TransferFamilyIndex = *TransferFamilyIter;
        if (TransferFamilyIndex == GraphicsFamilyIndex)
        {
            if (++TransferFamilyIter != GPU.TransferQueueFamilies.end())
            {
                TransferFamilyIndex = *TransferFamilyIter;

                DeviceQueueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo{}
                    .setQueueFamilyIndex(TransferFamilyIndex)
                    .setQueueCount      (1)
                    .setQueuePriorities ({Priority}));
            }
            else
            {
                LOG_WARN("Failed to find a separated Transfer Queue Family, "
                         "using the Graphics Queue Family!");
            }
        }
        // First build each feature struct explicitly
        auto Vulkan11Features =  vk::PhysicalDeviceVulkan11Features{}
            .setShaderDrawParameters                        (vk::True)
        ;
        auto Vulkan12Features =  vk::PhysicalDeviceVulkan12Features{}
            .setDescriptorIndexing                          (vk::True)
            .setRuntimeDescriptorArray                      (vk::True)
            .setShaderSampledImageArrayNonUniformIndexing   (vk::True)
            .setDescriptorBindingPartiallyBound             (vk::True)
            .setDescriptorBindingVariableDescriptorCount    (vk::True)
            .setDescriptorBindingUpdateUnusedWhilePending   (vk::True)
        ;
        auto Vulkan13Features =  vk::PhysicalDeviceVulkan13Features{}
            .setDynamicRendering                            (vk::True)
            .setSynchronization2                            (vk::True)
        ;
        vk::StructureChain FeatureChain
        {
            GPU.Features2,   // Bindless
            Vulkan11Features,
            Vulkan12Features,
            Vulkan13Features,
        };
        const auto CreateInfo = vk::DeviceCreateInfo{}
            .setPNext                   (&FeatureChain.get<vk::PhysicalDeviceFeatures2>())
            .setQueueCreateInfoCount    (DeviceQueueCreateInfos.size())
            .setPQueueCreateInfos       (DeviceQueueCreateInfos.data())
            .setEnabledExtensionCount   (DeviceExtensions.size())
            .setPpEnabledExtensionNames (DeviceExtensions.data())
        ;
        //Create Device
        auto Result = GPU.Context.createDevice(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Device!"); }
        else
        { Device.Context = std::move(*Result); }
        // Get Queues
        Device.GraphicsQueue = Device.Context.getQueue(GraphicsFamilyIndex, 0);
        Device.TransferQueue = Device.Context.getQueue(TransferFamilyIndex, 0);
    }


    void FVulkanDriver::
    CollectInstanceLayersAndExtensions()
    {
        // Layers
        this
#if !defined(VISERA_RELEASE_MODE)
            ->AddInstanceLayer("VK_LAYER_KHRONOS_validation")
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
                LOG_WARN("Failed to find required present mode {} for SwapChain!"
                         "-- Using FIFO by default.", SwapChain.PresentMode);
                SwapChain.PresentMode = vk::PresentModeKHR::eFifo;
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
        // Swapchain Image Extent
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
            .setClipped         (SwapChain.bClipped)
        ;
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
                SwapChain.Images.emplace_back(
                    SwapChainImages[Idx],
                    vk::ImageType::e2D,
                    vk::Extent3D{SwapChain.Extent.width, SwapChain.Extent.height, 1},
                    SwapChain.ImageFormat,
                    SwapChain.ImageUsage);
            }
        }
        // Create Image Views
        // SwapChain Images are not in registry, so use InvalidVulkanResourceHandle
        for (auto& Image : SwapChain.Images)
        {
            SwapChain.ImageViews.emplace_back(
                &Image,  // Direct pointer to swapchain image
                vk::ImageViewType::e2D,
                vk::ImageAspectFlagBits::eColor);
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
            ->AddDeviceExtension(vk::KHRMaintenance6ExtensionName) // BindDescriptorSets2() [TODO]: Remove in Vulkan1.4
#if defined(VISERA_ON_APPLE_SYSTEM)
            ->AddDeviceExtension("VK_KHR_portability_subset")
#endif
#if !defined(VISERA_OFFSCREEN_MODE)
            ->AddDeviceExtension(vk::KHRSwapchainExtensionName)
#endif
        ;
    }

    Bool FVulkanDriver::
    BeginFrame()
    {
        auto& CurrentFrame = GetCurrentFrame();
        if (!CurrentFrame.Fence.Wait())
        {
            LOG_FATAL("Failed to wait the fence (desc:{})!",
                      CurrentFrame.Fence.GetDescription());
        }
        CurrentFrame.DrawCalls.Reset();
        CurrentFrame.DrawCalls.Begin();

#if !defined(VISERA_OFFSCREEN_MODE)
        auto NextImageAcquireInfo = vk::AcquireNextImageInfoKHR{}
            .setSwapchain   (SwapChain.Context)
            .setTimeout     (Math::UpperBound<UInt64>())
            .setSemaphore   (CurrentFrame.SwapChainImageAvailable.GetHandle())
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
        for (auto& RenderPipeline : RenderPipelines)
        {
            RenderPipeline.SetColorRT(&CurrentColorRT);
        }

        return CurrentFrame.Fence.Reset();
    }

    void FVulkanDriver::
    EndFrame()
    {
        auto& CurrentFrame = GetCurrentFrame();

#if !defined(VISERA_OFFSCREEN_MODE)
        auto* CurrentSwapChainImageView = SwapChain.GetCurrentImageView();
        VISERA_ASSERT(CurrentSwapChainImageView != nullptr);
        // SwapChain images are not in registry, so GetImage() will return nullptr
        // We need to get the Image directly from ImageView's stored handle/pointer
        // For now, since swapchain ImageView has InvalidVulkanResourceHandle,
        // we need a different way to get the Image
        // Actually, let's check if ImageView can return the Image pointer for swapchain
        // For swapchain, ImageView stores InvalidVulkanResourceHandle, so GetImage() returns nullptr
        // We need to handle this specially - maybe ImageView should store pointer for non-registry images
        // For now, let's get Image from the SwapChain.Images array directly
        auto* CurrentSwapChainImage = &SwapChain.Images[SwapChain.Cursor];
        VISERA_ASSERT(CurrentSwapChainImage != nullptr);
        
        auto* CurrentColorRTImage = CurrentFrame.ColorRT.GetImage();
        VISERA_ASSERT(CurrentColorRTImage != nullptr);

        auto  OldColorRTLayout   = CurrentColorRTImage->GetLayout();
        VISERA_ASSERT(OldColorRTLayout == vk::ImageLayout::eColorAttachmentOptimal);

        CurrentFrame.DrawCalls.ConvertImageLayout(
                CurrentColorRTImage,
                vk::ImageLayout::eTransferSrcOptimal,
                vk::PipelineStageFlagBits2::eTopOfPipe,
                vk::AccessFlagBits2::eNone,
                vk::PipelineStageFlagBits2::eTransfer,
                vk::AccessFlagBits2::eTransferWrite
            );
        CurrentFrame.DrawCalls.ConvertImageLayout(
            CurrentSwapChainImage,
            vk::ImageLayout::eTransferDstOptimal,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eTransfer,
            vk::AccessFlagBits2::eTransferWrite
        );
        CurrentFrame.DrawCalls.BlitImage(
            CurrentColorRTImage,
            CurrentSwapChainImage);

        CurrentFrame.DrawCalls.ConvertImageLayout(
            CurrentColorRTImage,
            OldColorRTLayout,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            vk::AccessFlagBits2::eNone
        );
        CurrentFrame.DrawCalls.ConvertImageLayout(
            CurrentSwapChainImage,
            vk::ImageLayout::ePresentSrcKHR,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            vk::AccessFlagBits2::eNone
        );

        CurrentFrame.DrawCalls.End();

        Submit(&CurrentFrame.DrawCalls,
               &CurrentFrame.SwapChainImageAvailable,
               &SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor],
               &CurrentFrame.Fence);
#else
        CurrentFrame.DrawCalls.End();

        Submit(&CurrentFrame.DrawCalls,
               nullptr,
               nullptr,
               &CurrentFrame.Fence);

        if (!CurrentFrame.Fence.Wait())
        {
            LOG_FATAL("Failed to wait for fence (desc:{})!",
                      CurrentFrame.Fence.GetDescription());
        }
#endif
        InFlightFrameIndex += 1;
    }

    Bool FVulkanDriver::
    Present()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        const auto PresentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount  (1)
            .setPWaitSemaphores     (&(*SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor].GetHandle()))
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
#else

#endif
        return True;
    }

    void FVulkanDriver::
    Submit(FVulkanCommandBuffer* I_CommandBuffer,
           FVulkanSemaphore*     I_WaitSemaphore,
           FVulkanSemaphore*     I_SignalSemaphore,
           FVulkanFence*         I_Fence /* = nullptr */)
    {
        VISERA_ASSERT(I_CommandBuffer != nullptr);
        VISERA_ASSERT(I_CommandBuffer->IsReadyToSubmit());

        auto CommandBuffer = *I_CommandBuffer->GetHandle();
        auto WaitSemaphore = I_WaitSemaphore? *I_WaitSemaphore->GetHandle() : nullptr;
        auto SignalSemaphore = I_SignalSemaphore? *I_SignalSemaphore->GetHandle() : nullptr;
        auto Fence = I_Fence? *I_Fence->GetHandle() : nullptr;

        auto WaitSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(WaitSemaphore)
            .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        ;
        auto SignalSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(SignalSemaphore)
            .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        ;
        auto CommandBufferInfo = vk::CommandBufferSubmitInfo{}
            .setCommandBuffer(CommandBuffer)
        ;
        auto SubmitInfo = vk::SubmitInfo2{}
            .setWaitSemaphoreInfoCount  (WaitSemaphore?   1 : 0)
            .setPWaitSemaphoreInfos     (&WaitSemaphoreInfo)
            .setSignalSemaphoreInfoCount(SignalSemaphore? 1 : 0)
            .setPSignalSemaphoreInfos   (&SignalSemaphoreInfo)
            .setCommandBufferInfoCount  (1)
            .setPCommandBufferInfos     (&CommandBufferInfo)
        ;
        Device.GraphicsQueue.submit2(SubmitInfo, Fence);
    }

    FVulkanShaderModule* FVulkanDriver::
    CreateShaderModule(const TArray<FByte>& I_SPIRVShader)
    {
        VISERA_ASSERT(!I_SPIRVShader.empty());
        LOG_TRACE("Creating a Vulkan Shader Module");
        auto& NewShaderModule = ShaderModules.emplace_back(Device.Context, I_SPIRVShader);
        return &NewShaderModule;
    }


    FVulkanPipelineLayout* FVulkanDriver::
    CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                         const TArray<vk::PushConstantRange>&   I_PushConstants)
    {
        LOG_TRACE("Creating a Vulkan Pipeline Layout.");
        auto& NewPipelineLayout = PipelineLayouts.emplace_back(
            Device.Context,
            I_DescriptorSetLayouts,
            I_PushConstants);
        return &NewPipelineLayout;
    }

    FVulkanRenderPipeline* FVulkanDriver::
    CreateRenderPipeline(FVulkanPipelineLayout* I_PipelineLayout,
                         FVulkanShaderModule*   I_VertexShader,
                         FVulkanShaderModule*   I_FragmentShader)
    {
        LOG_TRACE("Creating a Vulkan Render Pipeline.");
        VISERA_ASSERT(I_PipelineLayout != nullptr);
        VISERA_ASSERT(I_VertexShader != nullptr);
        VISERA_ASSERT(I_FragmentShader != nullptr);
        // Move the resources from storage arrays into the pipeline
        // Note: This invalidates the pointers, but that's OK since Pipeline owns them now
        auto& NewRenderPipeline = RenderPipelines.emplace_back(
               std::move(*I_PipelineLayout),
               std::move(*I_VertexShader),
               std::move(*I_FragmentShader));
        // Call Create() to actually create the Vulkan pipeline
        NewRenderPipeline.Create(Device.Context, PipelineCache);
        return &NewRenderPipeline;
    }

    FVulkanComputePipeline* FVulkanDriver::
    CreateComputePipeline(FVulkanPipelineLayout* I_PipelineLayout,
                          FVulkanShaderModule*   I_ComputeShader)
    {
        LOG_TRACE("Creating a Vulkan Compute Pipeline.");
        VISERA_ASSERT(I_PipelineLayout != nullptr);
        VISERA_ASSERT(I_ComputeShader != nullptr);
        // Move the resources from storage arrays into the pipeline
        // ComputePipeline constructor creates the pipeline immediately
        auto& NewComputePipeline = ComputePipelines.emplace_back(
            Device.Context,
            std::move(*I_PipelineLayout),
            std::move(*I_ComputeShader),
            PipelineCache);
        return &NewComputePipeline;
    }

    FVulkanFence FVulkanDriver::
    CreateFence(Bool        I_bSignaled,
                FStringView I_Description)
    {
        LOG_TRACE("Creating a Vulkan Fence (description:{}, signaled:{}).", I_Description, I_bSignaled);
        return FVulkanFence(Device.Context, I_bSignaled, I_Description);
    }

    FVulkanSemaphore FVulkanDriver::
    CreateSemaphore(FStringView I_Name)
    {
        LOG_TRACE("Creating a Vulkan Semaphore (name: {}).", I_Name);
        return FVulkanSemaphore(Device.Context);
    }

    FVulkanImage FVulkanDriver::
    CreateImage(vk::ImageType           I_ImageType,
                const vk::Extent3D&     I_Extent,
                vk::Format              I_Format,
                vk::ImageUsageFlags     I_Usages)
    {
        VISERA_ASSERT(I_Extent.width && I_Extent.height && I_Extent.depth);
        LOG_TRACE("Creating a Vulkan Image (extent:[{},{},{}]).",
                  I_Extent.width, I_Extent.height, I_Extent.depth);
        return FVulkanImage(I_ImageType, I_Extent, I_Format, I_Usages);
    }

    FVulkanImageView FVulkanDriver::
    CreateImageView(FVulkanImage*                  I_Image,
                    vk::ImageViewType               I_ViewType,
                    vk::ImageAspectFlags            I_Aspect,
                    TClosedInterval<UInt8>          I_MipmapRange,
                    TClosedInterval<UInt8>          I_ArrayRange,
                    TOptional<vk::ComponentMapping> I_Swizzle)
    {
        LOG_TRACE("Creating a Vulkan Image View.");
        VISERA_ASSERT(I_Image != nullptr);
        return FVulkanImageView(
            I_Image,
            I_ViewType,
            I_Aspect,
            I_MipmapRange,
            I_ArrayRange,
            I_Swizzle);
    }

    FVulkanBuffer FVulkanDriver::
    CreateBuffer(UInt64                  I_Size,
                 vk::BufferUsageFlags    I_Usages,
                 EVulkanMemoryPoolFlags  I_MemoryPoolFlags /* = EVulkanMemoryPoolFlags::None */)
    {
        VISERA_ASSERT(I_Size != 0);
        LOG_TRACE("Creating a Vulkan Buffer ({} bytes).", I_Size);
        return FVulkanBuffer(I_Size, I_Usages, I_MemoryPoolFlags);
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
                GPlatform->GetResourceDirectory() / FPath("Cache/VulkanPipelines.cache")
            );
        }
    }

    void FVulkanDriver::
    CreateInFlightFrames()
    {
        const UInt8 FrameCount = 
#if !defined(VISERA_OFFSCREEN_MODE)
            SwapChain.Images.size();
#else
            1;
#endif

        InFlightFrames.clear();
        InFlightFrames.resize(FrameCount);

        LOG_TRACE("Creating {} in-flght frames (extent: [{},{}]).",
                  FrameCount, ColorRTRes.width, ColorRTRes.height);

        auto Cmd = CreateCommandBuffer(vk::QueueFlagBits::eGraphics);
        auto Fence = CreateFence(False, "convert render targets layout");
        Cmd.Begin();
        {
            for (UInt8 Index = 0; Index < FrameCount; ++Index)
            {
                auto& Frame = InFlightFrames[Index];
                Frame.ColorRTImage = CreateImage(
                    vk::ImageType::e2D,
                    {ColorRTRes.width, ColorRTRes.height, 1},
                    vk::Format::eR16G16B16A16Sfloat,
                    vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferSrc);

                Cmd.ConvertImageLayout(&Frame.ColorRTImage,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::PipelineStageFlagBits2::eTopOfPipe,
                    vk::AccessFlagBits2::eNone,
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    vk::AccessFlagBits2::eColorAttachmentWrite
                );
                Frame.ColorRTImageView = CreateImageView(
                    &Frame.ColorRTImage,
                    vk::ImageViewType::e2D,
                    vk::ImageAspectFlagBits::eColor);
                
                // Create all resources for this frame
                Frame.ColorRT = FVulkanRenderTarget(&Frame.ColorRTImageView);
                Frame.ColorRT
                .SetLoadOp(vk::AttachmentLoadOp::eLoad)
                .SetStoreOp(vk::AttachmentStoreOp::eStore);
                
                Frame.DrawCalls = CreateCommandBuffer(vk::QueueFlagBits::eGraphics);
                Frame.Fence     = CreateFence(True, Format("In-Flight Fence ({})", Index));
#if !defined(VISERA_OFFSCREEN_MODE)
                Frame.SwapChainImageAvailable = CreateSemaphore(Format("SwapChain Image Available ({})", Index));
#else

#endif
            }
        }
        Cmd.End();
        Submit(&Cmd, nullptr, nullptr, &Fence);

        if (!Fence.Wait())
        { LOG_FATAL("Failed to convert the layout of color render targets!"); }
    }

    FVulkanCommandBuffer FVulkanDriver::
    CreateCommandBuffer(vk::QueueFlagBits I_Queue)
    {
        switch (I_Queue)
        {
        case vk::QueueFlagBits::eGraphics:
            LOG_TRACE("Creating a new Vulkan Graphics Command Buffer.");
            return FVulkanCommandBuffer(
                Device.Context,
                GraphicsCommandPool);
        default:
            LOG_FATAL("Invalid Command Buffer Type!");
            // Return default-constructed (will be invalid, but allows compilation)
            return FVulkanCommandBuffer(Device.Context, GraphicsCommandPool);
        }
    }

    FVulkanDescriptorSetLayout* FVulkanDriver::
    CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        LOG_TRACE("Creating a new Vulkan Descriptor Set Layout.");
        auto& NewLayout = DescriptorSetLayouts.emplace_back(Device.Context, I_Bindings);
        return &NewLayout;
    }

    FVulkanDescriptorSet* FVulkanDriver::
    CreateDescriptorSet(FVulkanDescriptorSetLayout* I_DescriptorSetLayout)
    {
        LOG_TRACE("Creating a new Vulkan Descriptor Set.");
        VISERA_ASSERT(I_DescriptorSetLayout != nullptr);
        // DescriptorSet is typically created per frame
        // For now, create it and return pointer (caller must ensure lifetime)
        // In the future, we might want per-frame DescriptorSet storage
        static thread_local TArray<FVulkanDescriptorSet> TempDescriptorSets;
        auto& NewDescriptorSet = TempDescriptorSets.emplace_back(
               GetDescriptorPool(),
               I_DescriptorSetLayout);
        return &NewDescriptorSet;
    }

    FVulkanSampler FVulkanDriver::
    CreateImageSampler(vk::Filter             I_Filter,
                       vk::SamplerAddressMode I_AddressMode,
                       Float                  I_MaxAnisotropy /*= 1.0*/)
    {
        Bool bAnisotropy = I_MaxAnisotropy > 1.0;
        while (bAnisotropy)
        {
            if (!GPU.Properties.limits.maxSamplerAnisotropy)
            {
                LOG_WARN("Current device does NOT support anisotropy!");
                bAnisotropy = False;
                break;
            }
            if (I_MaxAnisotropy > GPU.Properties.limits.maxSamplerAnisotropy)
            {
                I_MaxAnisotropy = GPU.Properties.limits.maxSamplerAnisotropy;
                LOG_WARN("Clamped max anisotropy to the limit of current device {}!",
                         I_MaxAnisotropy);
                break;
            }
            break;
        }
        const auto CreateInfo = vk::SamplerCreateInfo{}
            .setMagFilter       (I_Filter)
            .setMinFilter       (I_Filter)
            .setAddressModeU    (I_AddressMode)
            .setAddressModeV    (I_AddressMode)
            .setAddressModeW    (I_AddressMode)
            .setBorderColor     (vk::BorderColor::eFloatTransparentBlack)
            .setMipmapMode      (vk::SamplerMipmapMode::eLinear)
            .setMipLodBias      (0.0)
            .setMinLod          (0.0)
            .setMaxLod          (1.0)
            .setAnisotropyEnable(bAnisotropy)
            .setMaxAnisotropy   (I_MaxAnisotropy)
            //.setCompareEnable   (False)
            //.setCompareOp       ({})
            //.setUnnormalizedCoordinates()
        ;
        return FVulkanSampler(Device.Context, CreateInfo);
    }

    FVulkanSampler FVulkanDriver::
    CreateCompareSampler(vk::Filter      I_Filter,
                         vk::CompareOp   I_CompareOp,
                         vk::BorderColor I_BorderColor)
    {
        const auto CreateInfo = vk::SamplerCreateInfo{}
            .setMagFilter       (I_Filter)
            .setMinFilter       (I_Filter)
            .setAddressModeU    (vk::SamplerAddressMode::eClampToBorder)
            .setAddressModeV    (vk::SamplerAddressMode::eClampToBorder)
            .setAddressModeW    (vk::SamplerAddressMode::eClampToBorder)
            .setBorderColor     (I_BorderColor)
            .setMipmapMode      (vk::SamplerMipmapMode::eLinear)
            .setMipLodBias      (0.0)
            .setMinLod          (0.0)
            .setMaxLod          (1.0)
            .setAnisotropyEnable(False)
            //.setMaxAnisotropy   (1.0)
            .setCompareEnable   (True)
            .setCompareOp       (I_CompareOp)
        ;
        return FVulkanSampler(Device.Context, CreateInfo);
    }
}