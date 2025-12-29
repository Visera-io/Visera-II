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
       import Visera.Core.Math.Arithmetic.Operation;
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
            TArray<TSharedPtr<FVulkanImageWrapper>> Images     {}; // SwapChain manages Images so do NOT use RAII here.
            TArray<TSharedPtr<FVulkanImageView>>    ImageViews {};
            TArray<TSharedPtr<FVulkanSemaphore>>    ReadyToPresentSemaphores;
            UInt32                                  Cursor     {0};
            vk::ImageUsageFlags     ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                 vk::ImageUsageFlagBits::eTransferDst};
            vk::Format              ImageFormat {vk::Format::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR       ColorSpace  {vk::ColorSpaceKHR::eSrgbNonlinear};
            UInt32                  MinimalImageCount{3};
            vk::PresentModeKHR      PresentMode {vk::PresentModeKHR::eMailbox};
            vk::SharingMode         SharingMode {vk::SharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                          bClipped       {True};

            [[nodiscard]] inline const TSharedRef<FVulkanImageView>
            GetCurrentImageView() { return ImageViews[Cursor]; }
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
        inline void
        EndFrame();
        [[nodiscard]] inline Bool
        Present();
        inline void
        Submit(TSharedRef<FVulkanCommandBuffer> I_CommandBuffer,
               TSharedRef<FVulkanSemaphore>     I_WaitSemaphore,
               TSharedRef<FVulkanSemaphore>     I_SignalSemaphore,
               TSharedRef<FVulkanFence>         I_Fence = {});
        [[nodiscard]] TSharedPtr<FVulkanShaderModule>
        CreateShaderModule(const TArray<FByte>& I_SPIRVShader);
        [[nodiscard]] TSharedPtr<FVulkanPipelineLayout>
        CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                             const TArray<vk::PushConstantRange>&   I_PushConstants);
        [[nodiscard]] TSharedPtr<FVulkanRenderPipeline>
        CreateRenderPipeline(TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                             TSharedRef<FVulkanShaderModule>   I_VertexShader,
                             TSharedRef<FVulkanShaderModule>   I_FragmentShader);
        [[nodiscard]] TSharedPtr<FVulkanComputePipeline>
        CreateComputePipeline(TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                              TSharedRef<FVulkanShaderModule>   I_ComputeShader);
        [[nodiscard]] TSharedPtr<FVulkanFence>
        CreateFence(Bool        I_bSignaled,
                    FStringView I_Description);
        [[nodiscard]] TSharedPtr<FVulkanSemaphore>
        CreateSemaphore(FStringView I_Name);
        [[nodiscard]] TSharedPtr<FVulkanImage>
        CreateImage(vk::ImageType       I_ImageType,
                    const vk::Extent3D& I_Extent,
                    vk::Format          I_Format,
                    vk::ImageUsageFlags     I_Usages);
        [[nodiscard]] TSharedPtr<FVulkanImageView>
        CreateImageView(TSharedRef<FVulkanImage>        I_Image,
                        vk::ImageViewType               I_ViewType,
                        vk::ImageAspectFlagBits         I_Aspect,
                        const TPair<UInt8, UInt8>&      I_MipmapRange = {0,0},
                        const TPair<UInt8, UInt8>&      I_ArrayRange  = {0,0},
                        TOptional<vk::ComponentMapping> I_Swizzle     = {});
        [[nodiscard]] TSharedPtr<FVulkanSampler>
        CreateImageSampler(vk::Filter             I_Filter,
                           vk::SamplerAddressMode I_AddressMode,
                           Float                  I_MaxAnisotropy = 1.0);
        [[nodiscard]] TSharedPtr<FVulkanSampler>
        CreateCompareSampler(vk::Filter      I_Filter,
                             vk::CompareOp   I_CompareOp,
                             vk::BorderColor I_BorderColor);
        [[nodiscard]] TSharedPtr<FVulkanBuffer>
        CreateBuffer(UInt64                  I_Size,
                     vk::BufferUsageFlags    I_Usages,
                     EVulkanMemoryPoolFlags  I_MemoryPoolFlags = EVulkanMemoryPoolFlags::None);
        [[nodiscard]] TSharedPtr<FVulkanCommandBuffer>
        CreateCommandBuffer(vk::QueueFlagBits I_Queue);
        [[nodiscard]] TSharedPtr<FVulkanDescriptorSetLayout>
        CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
        [[nodiscard]] TSharedPtr<FVulkanDescriptorSet>
        CreateDescriptorSet(TSharedRef<FVulkanDescriptorSetLayout> I_DescriptorSetLayout);
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
        const vk::Extent2D                          ColorRTRes { 1920, 1080 };
        TArray<FInFlightFrame>                      InFlightFrames;
        UInt8                                       InFlightFrameIndex {0};

        TMap<FString, TSharedPtr<FVulkanSemaphore>> SemaphorePool;
        vk::raii::DescriptorPool                    DescriptorPool      {nullptr};
        vk::raii::CommandPool                       GraphicsCommandPool {nullptr};

        TArray<TSharedPtr<FVulkanRenderPipeline>>   RenderPipelines;
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
        SemaphorePool.clear();
        RenderPipelines.clear();
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
        enum : UInt32 { SampledImage, StorageImage, CombinedImageSampler, UniformBuffer, MAX_DESCRIPTOR_ENUM };

        vk::DescriptorPoolSize PoolSizes[MAX_DESCRIPTOR_ENUM];
        PoolSizes[SampledImage]
        .setType            (vk::DescriptorType::eSampledImage)
        .setDescriptorCount (100);
        PoolSizes[StorageImage]
        .setType            (vk::DescriptorType::eStorageImage)
        .setDescriptorCount (100);
        PoolSizes[CombinedImageSampler]
        .setType            (vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount (100);
        PoolSizes[UniformBuffer]
        .setType            (vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount (100);

        auto CreateInfo = vk::DescriptorPoolCreateInfo()
            .setFlags           (vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind |
                                 vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets         (GPU.Properties.limits.maxBoundDescriptorSets)
            .setPoolSizeCount   (MAX_DESCRIPTOR_ENUM)
            .setPPoolSizes      (PoolSizes)
        ;
        auto Result = Device.Context.createDescriptorPool(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create the Vulkan Descriptor Pool!"); }
        else
        { DescriptorPool = std::move(*Result); }

        LOG_TRACE("Created a Vulkan Descriptor Pool: (max_sets:{})",
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
                if (Result.has_value())
                { GPU.PresentQueueFamilies.insert(std::move(*Result)); }
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
        GPU.Properties2 = GPU.Context.getProperties2();
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
                SwapChain.Images.emplace_back(MakeShared<FVulkanImageWrapper>(
                    SwapChainImages[Idx],
                    vk::ImageType::e2D,
                    vk::Extent3D{SwapChain.Extent.width, SwapChain.Extent.height, 1},
                    SwapChain.ImageFormat,
                    SwapChain.ImageUsage));
            }
        }
        // Create Image Views
        for (const auto& Image : SwapChain.Images)
        {
            SwapChain.ImageViews.emplace_back(MakeShared<FVulkanImageView>(
                Image,
                vk::ImageViewType::e2D,
                vk::ImageAspectFlagBits::eColor));
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
        auto& CurrentFrame = InFlightFrames[InFlightFrameIndex];
        if (!CurrentFrame.Fence->Wait())
        {
            LOG_FATAL("Failed to wait the fence (desc:{})!",
                      CurrentFrame.Fence->GetDescription());
        }
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
        for (auto& RenderPipeline : RenderPipelines)
        {
            RenderPipeline->SetColorRT(CurrentColorRT);
        }

        return CurrentFrame.Fence->Reset();
    }

    void FVulkanDriver::
    EndFrame()
    {
        auto& CurrentFrame = InFlightFrames[InFlightFrameIndex];

#if !defined(VISERA_OFFSCREEN_MODE)
        auto& CurrentSwapChainImage = SwapChain.GetCurrentImageView()->GetImage();
        auto& CurrentColorRT = CurrentFrame.ColorRT->GetImage();

        auto  OldColorRTLayout   = CurrentColorRT->GetLayout();
        VISERA_ASSERT(OldColorRTLayout == vk::ImageLayout::eColorAttachmentOptimal);

        CurrentFrame.DrawCalls->ConvertImageLayout(
                CurrentColorRT,
                vk::ImageLayout::eTransferSrcOptimal,
                vk::PipelineStageFlagBits2::eTopOfPipe,
                vk::AccessFlagBits2::eNone,
                vk::PipelineStageFlagBits2::eTransfer,
                vk::AccessFlagBits2::eTransferWrite
            );
        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentSwapChainImage,
            vk::ImageLayout::eTransferDstOptimal,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eTransfer,
            vk::AccessFlagBits2::eTransferWrite
        );
        CurrentFrame.DrawCalls->BlitImage(
            CurrentColorRT,
            CurrentSwapChainImage);

        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentColorRT,
            OldColorRTLayout,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            vk::AccessFlagBits2::eNone
        );
        CurrentFrame.DrawCalls->ConvertImageLayout(
            CurrentSwapChainImage,
            vk::ImageLayout::ePresentSrcKHR,
            vk::PipelineStageFlagBits2::eTopOfPipe,
            vk::AccessFlagBits2::eNone,
            vk::PipelineStageFlagBits2::eBottomOfPipe,
            vk::AccessFlagBits2::eNone
        );

        CurrentFrame.DrawCalls->End();

        Submit(CurrentFrame.DrawCalls,
               CurrentFrame.SwapChainImageAvailable,
               SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor],
               CurrentFrame.Fence);
#else
        CurrentFrame.DrawCalls->End();

        Submit(CurrentFrame.DrawCalls,
               {},
               {},
               CurrentFrame.Fence);

        if (!CurrentFrame.Fence->Wait())
        {
            LOG_FATAL("Failed to wait for fence (desc:{})!",
                      CurrentFrame.Fence->GetDescription());
        }
#endif
    }

    Bool FVulkanDriver::
    Present()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        const auto PresentInfo = vk::PresentInfoKHR{}
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
#else

#endif
        InFlightFrameIndex = (InFlightFrameIndex + 1) % (InFlightFrames.size());
        return True;
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

    TSharedPtr<FVulkanShaderModule> FVulkanDriver::
    CreateShaderModule(const TArray<FByte>& I_SPIRVShader)
    {
        VISERA_ASSERT(!I_SPIRVShader.empty());
        LOG_TRACE("Creating a Vulkan Shader Module");
        return MakeShared<FVulkanShaderModule>(Device.Context, I_SPIRVShader);
    }


    TSharedPtr<FVulkanPipelineLayout> FVulkanDriver::
    CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                         const TArray<vk::PushConstantRange>&   I_PushConstants)
    {
        LOG_TRACE("Creating a Vulkan Pipeline Layout.");
        return MakeShared<FVulkanPipelineLayout>(
            Device.Context,
            I_DescriptorSetLayouts,
            I_PushConstants);
    }

    TSharedPtr<FVulkanRenderPipeline> FVulkanDriver::
    CreateRenderPipeline(TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                         TSharedRef<FVulkanShaderModule>   I_VertexShader,
                         TSharedRef<FVulkanShaderModule>   I_FragmentShader)
    {
        LOG_TRACE("Creating a Vulkan Render Pipeline.");
        auto& NewRenderPipeline = RenderPipelines.emplace_back(MakeShared<FVulkanRenderPipeline>(
               I_PipelineLayout,
               I_VertexShader,
               I_FragmentShader));
        return NewRenderPipeline; // Not be created!
    }

    TSharedPtr<FVulkanComputePipeline> FVulkanDriver::
    CreateComputePipeline(TSharedRef<FVulkanPipelineLayout> I_PipelineLayout,
                          TSharedRef<FVulkanShaderModule>   I_ComputeShader)
    {
        LOG_TRACE("Creating a Vulkan Compute Pipeline.");
        return MakeShared<FVulkanComputePipeline>(
            Device.Context,
            I_PipelineLayout,
            I_ComputeShader,
            PipelineCache);
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
    CreateImage(vk::ImageType       I_ImageType,
                const vk::Extent3D& I_Extent,
                vk::Format          I_Format,
                vk::ImageUsageFlags     I_Usages)
    {
        VISERA_ASSERT(I_Extent.width && I_Extent.height && I_Extent.depth);
        LOG_TRACE("Creating a Vulkan Image (extent:[{},{},{}]).",
                  I_Extent.width, I_Extent.height, I_Extent.depth);
        return MakeShared<FVulkanImage>(I_ImageType, I_Extent, I_Format, I_Usages);
    }

    TSharedPtr<FVulkanImageView> FVulkanDriver::
    CreateImageView(TSharedRef<FVulkanImage>   I_Image,
                    vk::ImageViewType       I_ViewType,
                    vk::ImageAspectFlagBits         I_Aspect,
                    const TPair<UInt8, UInt8>& I_MipmapRange,
                    const TPair<UInt8, UInt8>& I_ArrayRange,
                    TOptional<vk::ComponentMapping>  I_Swizzle)
    {
        LOG_TRACE("Creating a Vulkan Image View.");
        return MakeShared<FVulkanImageView>(
            I_Image,
            I_ViewType,
            I_Aspect,
            I_MipmapRange,
            I_ArrayRange,
            I_Swizzle);
    }

    TSharedPtr<FVulkanBuffer> FVulkanDriver::
    CreateBuffer(UInt64                  I_Size,
                 vk::BufferUsageFlags    I_Usages,
                 EVulkanMemoryPoolFlags  I_MemoryPoolFlags /* = EVulkanMemoryPoolFlags::None */)
    {
        VISERA_ASSERT(I_Size != 0);
        LOG_TRACE("Creating a Vulkan Buffer ({} bytes).", I_Size);
        return MakeShared<FVulkanBuffer>(I_Size, I_Usages, I_MemoryPoolFlags);
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
#if !defined(VISERA_OFFSCREEN_MODE)
        InFlightFrames.resize(SwapChain.Images.size());
#else
        InFlightFrames.resize(1);
#endif

        LOG_TRACE("Creating {} in-flght frames (extent: [{},{}]).",
                  InFlightFrames.size(), ColorRTRes.width, ColorRTRes.height);

        auto Cmd = CreateCommandBuffer(vk::QueueFlagBits::eGraphics);
        auto Fence = CreateFence(False, "convert render targets layout");
        Cmd->Begin();
        {
            UInt8 Index{0};
            for (auto& InFlightFrame : InFlightFrames)
            {
                auto TargetImage = CreateImage(
                    vk::ImageType::e2D,
                    {ColorRTRes.width, ColorRTRes.height, 1},
                    vk::Format::eR16G16B16A16Sfloat,
                    vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferSrc);

                Cmd->ConvertImageLayout(TargetImage,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::PipelineStageFlagBits2::eTopOfPipe,
                    vk::AccessFlagBits2::eNone,
                    vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                    vk::AccessFlagBits2::eColorAttachmentWrite
                );
                auto TargetView = CreateImageView(TargetImage,
                    vk::ImageViewType::e2D,
                    vk::ImageAspectFlagBits::eColor);
                InFlightFrame.ColorRT = MakeShared<FVulkanRenderTarget>(TargetView);
                InFlightFrame.ColorRT->SetLoadOp(vk::AttachmentLoadOp::eLoad)
                                     ->SetStoreOp(vk::AttachmentStoreOp::eStore);

                // Command Buffers
                InFlightFrame.DrawCalls = CreateCommandBuffer(vk::QueueFlagBits::eGraphics);
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
    CreateCommandBuffer(vk::QueueFlagBits I_Queue)
    {
        switch (I_Queue)
        {
        case vk::QueueFlagBits::eGraphics:
            LOG_TRACE("Creating a new Vulkan Graphics Command Buffer.");
            return MakeShared<FVulkanCommandBuffer>(
                Device.Context,
                GraphicsCommandPool);
        default:
            LOG_FATAL("Invalid Command Buffer Type!");
            return {};
        }
    }

    TSharedPtr<FVulkanDescriptorSetLayout> FVulkanDriver::
    CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        LOG_TRACE("Creating a new Vulkan Descriptor Set Layout.");
        return MakeShared<FVulkanDescriptorSetLayout>(
            Device.Context, I_Bindings);
    }

    TSharedPtr<FVulkanDescriptorSet> FVulkanDriver::
    CreateDescriptorSet(TSharedRef<FVulkanDescriptorSetLayout> I_DescriptorSetLayout)
    {
        LOG_TRACE("Creating a new Vulkan Descriptor Set.");
        return MakeShared<FVulkanDescriptorSet>(
               GetDescriptorPool(),
               I_DescriptorSetLayout);
    }

    TSharedPtr<FVulkanSampler> FVulkanDriver::
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
        auto CreateInfo = vk::SamplerCreateInfo{}
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
        return MakeShared<FVulkanSampler>(Device.Context, CreateInfo);
    }

    TSharedPtr<FVulkanSampler> FVulkanDriver::
    CreateCompareSampler(vk::Filter      I_Filter,
                         vk::CompareOp   I_CompareOp,
                         vk::BorderColor I_BorderColor)
    {
        auto CreateInfo = vk::SamplerCreateInfo{}
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
        return MakeShared<FVulkanSampler>(Device.Context, CreateInfo);
    }
}