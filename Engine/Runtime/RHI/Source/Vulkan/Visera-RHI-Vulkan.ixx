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
export import Visera.RHI.Vulkan.CommandPool;
export import Visera.RHI.Vulkan.Image;
export import Visera.RHI.Vulkan.Buffer;
export import Visera.RHI.Vulkan.Sampler;
export import Visera.RHI.Vulkan.Pipeline;
export import Visera.RHI.Vulkan.RenderTarget;
export import Visera.RHI.Vulkan.DescriptorSet;
export import Visera.RHI.Vulkan.DescriptorSetLayout;
export import Visera.RHI.Vulkan.Sync;
       import Visera.RHI.Vulkan.Loader;
       import Visera.RHI.Vulkan.Allocator;
       import Visera.RHI.Vulkan.ShaderModule;
       import Visera.RHI.Types.Shader;
       import Visera.Core.Math.Arithmetic;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Set;
       import Visera.Core.Types.Array;
       import Visera.Core.Traits.Flags;
       import Visera.Global.Log;
       import Visera.Platform;
       import vulkan_hpp;

export namespace Visera
{
    using EVulkanMemoryProperty = EVMAMemoryProperty;

    class VISERA_RHI_API FVulkanDriver
    {
    public:
        // return SwapChain.Cursor
        [[nodiscard]] inline UInt32
        WaitNextFrame();
        [[nodiscard]] inline Bool
        Present();
        template<Concepts::CommandBuffer CommandBufferType> void
        Submit(CommandBufferType*    I_CommandBuffer,
               FVulkanSemaphore*     I_WaitSemaphore,
               FVulkanSemaphore*     I_SignalSemaphore,
               FVulkanFence*         I_Fence);

        template<EVulkanQueueFamily QueueFamily> [[nodiscard]] FVulkanCommandPool<QueueFamily>
        CreateCommandPool(Bool I_bTransient);
        [[nodiscard]] FVulkanShaderModule
        CreateShaderModule(const TArray<FByte>& I_SPIRVShader);
        [[nodiscard]] FVulkanPipelineLayout
        CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                             const TArray<vk::PushConstantRange>&   I_PushConstants);
        [[nodiscard]] FVulkanRenderPipeline
        CreateRenderPipeline(FVulkanPipelineLayout* I_PipelineLayout,
                             FVulkanShaderModule*   I_VertexShader,
                             FVulkanShaderModule*   I_FragmentShader);
        [[nodiscard]] FVulkanComputePipeline
        CreateComputePipeline(FVulkanPipelineLayout* I_PipelineLayout,
                              FVulkanShaderModule*   I_ComputeShader);
        [[nodiscard]] FVulkanFence
        CreateFence(Bool I_bSignaled);
        [[nodiscard]] FVulkanSemaphore
        CreateSemaphore();
        [[nodiscard]] FVulkanImage
        CreateImage(const vk::ImageCreateInfo& I_CreateInfo,
                    EVulkanMemoryProperty      I_MemoryProperties);
        [[nodiscard]] FVulkanImageView
        CreateImageView(FVulkanImage*           I_Image,
                        vk::ImageViewType       I_ViewType,
                        vk::ImageAspectFlags    I_Aspect,
                        TClosedInterval<UInt8>  I_MipmapRange,
                        TClosedInterval<UInt8>  I_ArrayRange,
                        vk::ComponentMapping    I_Swizzle = {});
        [[nodiscard]] FVulkanSampler
        CreateImageSampler(vk::Filter             I_Filter,
                           vk::SamplerAddressMode I_AddressMode,
                           Float                  I_MaxAnisotropy = 1.0);
        [[nodiscard]] FVulkanSampler
        CreateCompareSampler(vk::Filter      I_Filter,
                             vk::CompareOp   I_CompareOp,
                             vk::BorderColor I_BorderColor);
        [[nodiscard]] FVulkanBuffer
        CreateBuffer(const vk::BufferCreateInfo& I_CreateInfo,
                     EVulkanMemoryProperty       I_MemoryProperties);
        [[nodiscard]] FVulkanDescriptorSetLayout
        CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings);
        [[nodiscard]] const TUniqueRef<FVulkanPipelineCache>
        GetPipelineCache() const { return PipelineCache; }
        [[nodiscard]] inline const auto&
        GetInstance() const { return Instance; }
        [[nodiscard]] inline const auto&
        GetGPU() const { return GPU; }
        [[nodiscard]] inline const auto&
        GetDevice() const { return Device; }
        [[nodiscard]] inline const auto&
        GetSwapChain() const { return SwapChain; }
        [[nodiscard]] inline auto&
        GetSwapChain() { return SwapChain; }
        void inline
        WaitIdle() const { auto Result = Device.Context.waitIdle(); }

    private:
        TUniquePtr<FVulkanPipelineCache> PipelineCache;

        vk::ApplicationInfo              AppInfo;
        vk::raii::Context                Context;

        vk::raii::Instance               Instance        {nullptr};
        TArray<const char*>              InstanceLayers;
        TArray<const char*>              InstanceExtensions;
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
            vk::raii::Device    Context        {nullptr};
            TArray<const char*> Layers;
            TArray<const char*> Extensions;
            vk::raii::Queue     GraphicsQueue  {nullptr};
            vk::raii::Queue     TransferQueue  {nullptr};
            vk::raii::Queue     ComputeQueue   {nullptr};
        }Device;

        TUniquePtr<FVulkanLoader>    Loader;
        TUniquePtr<FVulkanAllocator> Allocator;

#if !defined(VISERA_OFFSCREEN_MODE)
        struct
        {
            vk::raii::SwapchainKHR          Context     {nullptr};
            vk::raii::SwapchainKHR          OldContext  {nullptr};
            vk::Extent2D                    Extent      {0U, 0U};
            TArray<FVulkanSwapChainImage>   Images      {}; // SwapChain manages Images so do NOT use RAII here.
            TArray<FVulkanImageView>        ImageViews  {};
            TArray<FVulkanSemaphore>        ReadyToPresentSemaphores;
            UInt32                          Cursor      {0};
            vk::ImageUsageFlags             ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                         vk::ImageUsageFlagBits::eTransferDst};
            vk::Format                      ImageFormat {vk::Format::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR               ColorSpace  {vk::ColorSpaceKHR::eSrgbNonlinear};
            UInt32                          MinimalImageCount{3};
            vk::PresentModeKHR              PresentMode {vk::PresentModeKHR::eMailbox};
            vk::SharingMode                 SharingMode {vk::SharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR   CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                            bClipped       {True};

            [[nodiscard]] inline FVulkanImageView*
            GetCurrentImageView() { return &ImageViews[Cursor]; }
        }SwapChain;
#endif

    private:
        void inline CreateInstance();       void inline DestroyInstance();
        void inline CreateDebugMessenger(); void inline DestroyDebugMessenger();
        void inline CreateSurface();        void inline DestroySurface();
        void inline PickPhysicalDevice();
        void inline CreateDevice();         void inline DestroyDevice();
        void inline CreateAllocator();      void inline DestroyAllocator();
        void inline CreateSwapChain();      void inline DestroySwapChain();
        void inline CreatePipelineCache();  void inline DestroyPipelineCache();

        inline FVulkanDriver*
        AddInstanceLayer(const char* I_Layer)           { InstanceLayers.emplace_back(I_Layer);         return this; }
        inline FVulkanDriver*
        AddInstanceExtension(const char* I_Extension)   { InstanceExtensions.push_back(I_Extension);    return this; }
        inline FVulkanDriver*
        AddDeviceLayer(const char* I_Layer)             { Device.Layers.push_back(I_Layer);              return this; }
        inline FVulkanDriver*
        AddDeviceExtension(const char* I_Extension)     { Device.Extensions.push_back(I_Extension);      return this; }

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
        CreateInstance();
        Loader->Load(Instance);
        CreateDebugMessenger();
        CreateSurface();
        CreateDevice();
        Loader->Load(Device.Context);
        CreateAllocator();
        CreateSwapChain();
        CreatePipelineCache();
    };

    FVulkanDriver::
    ~FVulkanDriver()
    {
        WaitIdle();
        // Resources are owned by registry at RHI level, not by driver
        DestroyPipelineCache();
        DestroySwapChain();
        DestroyAllocator();
        DestroyDevice();
        DestroySurface();
        DestroyDebugMessenger();
        DestroyInstance();
        Loader.reset();
    }

    void FVulkanDriver::
    CreateAllocator()
    {
        Allocator = MakeUnique<FVulkanAllocator>
        (
            AppInfo.apiVersion,
            Instance,
            GPU.Context,
            Device.Context
        );
    }

    void FVulkanDriver::
    DestroyAllocator()
    {
        Allocator.reset();
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
    DestroyPipelineCache()
    {
        PipelineCache.reset();
    }

    void FVulkanDriver::
    CreateInstance()
    {
        CollectInstanceLayersAndExtensions();

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
            for (auto const& RequiredExtension : Device.Extensions)
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
        CollectDeviceLayersAndExtensions();
        PickPhysicalDevice();

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
            .setEnabledExtensionCount   (Device.Extensions.size())
            .setPpEnabledExtensionNames (Device.Extensions.data())
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
            SwapChain.Images.resize(SwapChainImages.size());
            for (UInt8 Idx = 0; Idx < SwapChainImages.size(); ++Idx)
            {
                SwapChain.Images[Idx] = FVulkanSwapChainImage(
                    Allocator,
                    SwapChainImages[Idx],
                    vk::ImageType::e2D,
                    vk::Extent3D{SwapChain.Extent.width, SwapChain.Extent.height, 1},
                    SwapChain.ImageFormat,
                    SwapChain.ImageUsage);
            }
        }
        // Create Image Views
        for (auto& Image : SwapChain.Images)
        {
            SwapChain.ImageViews.emplace_back(
                &Image,  // Direct pointer to swapchain image
                vk::ImageViewType::e2D,
                vk::ImageAspectFlagBits::eColor);
        }
        // Create Semaphores and Fences
        SwapChain.ReadyToPresentSemaphores.resize(SwapChain.Images.size());
        for (UInt8 Idx = 0; Idx < SwapChain.Images.size(); ++Idx)
        {
            SwapChain.ReadyToPresentSemaphores[Idx]
            = CreateSemaphore();
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

    UInt32 FVulkanDriver::
    WaitNextFrame()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        auto NextImageAcquireInfo = vk::AcquireNextImageInfoKHR{}
            .setSwapchain   (SwapChain.Context)
            .setTimeout     (Math::UpperBound<UInt64>())
            .setSemaphore   (nullptr)
            .setFence       (nullptr)
            .setDeviceMask  (1)
        ;
        auto NextImageIndex = Device.Context.acquireNextImage2KHR(NextImageAcquireInfo);
        if (NextImageIndex.has_value())
        {
            SwapChain.Cursor = *NextImageIndex;
        }
        else { LOG_ERROR("Failed to acquire the next Vulkan SwapChain Image!"); }
#else
        LOG_WARN("NOT need to call this in the off-screen mode!");
#endif
        return SwapChain.Cursor;
    }

    Bool FVulkanDriver::
    Present()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        auto ReadyToPresent = SwapChain.ReadyToPresentSemaphores[SwapChain.Cursor].GetHandle();
        const auto PresentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount  (1)
            .setPWaitSemaphores     (&ReadyToPresent)
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

    template<Concepts::CommandBuffer CommandBufferType> void FVulkanDriver::
    Submit(CommandBufferType*    I_CommandBuffer,
           FVulkanSemaphore*     I_WaitSemaphore,
           FVulkanSemaphore*     I_SignalSemaphore,
           FVulkanFence*         I_Fence)
    {
        VISERA_ASSERT(I_CommandBuffer != nullptr);
        VISERA_ASSERT(I_CommandBuffer->IsReadyToSubmit());

        auto CommandBuffer   = I_CommandBuffer->GetHandle();
        auto WaitSemaphore   = I_WaitSemaphore? I_WaitSemaphore->GetHandle() : nullptr;
        auto SignalSemaphore = I_SignalSemaphore? I_SignalSemaphore->GetHandle() : nullptr;
        auto Fence           = I_Fence? I_Fence->GetHandle() : nullptr;

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
        switch (CommandBufferType::QueueFamily)
        {
        case EVulkanQueueFamily::Graphics:
            Device.GraphicsQueue.submit2(SubmitInfo, Fence);
            break;
        case EVulkanQueueFamily::Transfer:
            Device.TransferQueue.submit2(SubmitInfo, Fence);
            break;
        case EVulkanQueueFamily::Compute:
            Device.ComputeQueue.submit2(SubmitInfo, Fence);
            break;
        default:
            LOG_FATAL("Invalid Vulkan Queue!");
        }
    }

    template<EVulkanQueueFamily QueueFamily> FVulkanCommandPool<QueueFamily> FVulkanDriver::
    CreateCommandPool(Bool I_bTransient)
    {
        vk::CommandPoolCreateFlags Flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        if (I_bTransient) { Flags |= vk::CommandPoolCreateFlagBits::eTransient; }

        UInt32 QueueFamilyIndex = 0U;
        switch (QueueFamily)
        {
        case EVulkanQueueFamily::Graphics:
            QueueFamilyIndex = *GPU.GraphicsQueueFamilies.begin();
            break;
        case EVulkanQueueFamily::Transfer:
            QueueFamilyIndex = *GPU.TransferQueueFamilies.begin();
            break;
        case EVulkanQueueFamily::Compute:
            QueueFamilyIndex = *GPU.ComputeQueueFamilies.begin();
            break;
        default:
            LOG_FATAL("Invalid Vulkan Queue Family for CommandPool!");
        }

        const auto CreateInfo = vk::CommandPoolCreateInfo{}
            .setQueueFamilyIndex(QueueFamilyIndex)
            .setFlags           (Flags)
            .setPNext           (nullptr)
        ;

        return FVulkanCommandPool<QueueFamily>(Device.Context, CreateInfo);
    }

    FVulkanShaderModule FVulkanDriver::
    CreateShaderModule(const TArray<FByte>& I_SPIRVShader)
    {
        VISERA_ASSERT(!I_SPIRVShader.empty());
        LOG_TRACE("Creating a Vulkan Shader Module");
        return FVulkanShaderModule(Device.Context, I_SPIRVShader);
    }


    FVulkanPipelineLayout FVulkanDriver::
    CreatePipelineLayout(const TArray<vk::DescriptorSetLayout>& I_DescriptorSetLayouts,
                         const TArray<vk::PushConstantRange>&   I_PushConstants)
    {
        LOG_TRACE("Creating a Vulkan Pipeline Layout.");
        return FVulkanPipelineLayout(
                Device.Context,
                I_DescriptorSetLayouts,
                I_PushConstants);
    }

    FVulkanRenderPipeline FVulkanDriver::
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
        auto NewRenderPipeline = FVulkanRenderPipeline(
               std::move(*I_PipelineLayout),
               std::move(*I_VertexShader),
               std::move(*I_FragmentShader));
        // Call Create() to actually create the Vulkan pipeline
        NewRenderPipeline.Create(Device.Context, PipelineCache);
        return NewRenderPipeline;
    }

    FVulkanComputePipeline FVulkanDriver::
    CreateComputePipeline(FVulkanPipelineLayout* I_PipelineLayout,
                          FVulkanShaderModule*   I_ComputeShader)
    {
        LOG_TRACE("Creating a Vulkan Compute Pipeline.");
        VISERA_ASSERT(I_PipelineLayout != nullptr);
        VISERA_ASSERT(I_ComputeShader != nullptr);

        return FVulkanComputePipeline(
            Device.Context,
            I_PipelineLayout,
            I_ComputeShader,
            PipelineCache);
    }

    FVulkanFence FVulkanDriver::
    CreateFence(Bool I_bSignaled)
    {
        return FVulkanFence(Device.Context, I_bSignaled);
    }

    FVulkanSemaphore FVulkanDriver::
    CreateSemaphore()
    {
        return FVulkanSemaphore(Device.Context);
    }

    FVulkanImage FVulkanDriver::
    CreateImage(const vk::ImageCreateInfo& I_CreateInfo,
                EVulkanMemoryProperty      I_MemoryProperties)
    {
        return FVulkanImage(Allocator, I_CreateInfo, I_MemoryProperties);
    }

    FVulkanImageView FVulkanDriver::
    CreateImageView(FVulkanImage*           I_Image,
                    vk::ImageViewType       I_ViewType,
                    vk::ImageAspectFlags    I_Aspect,
                    TClosedInterval<UInt8>  I_MipmapRange,
                    TClosedInterval<UInt8>  I_ArrayRange,
                    vk::ComponentMapping    I_Swizzle)
    {
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
    CreateBuffer(const vk::BufferCreateInfo& I_CreateInfo,
                 EVulkanMemoryProperty       I_MemoryProperties)
    {
        VISERA_ASSERT(I_CreateInfo.size != 0);
        return FVulkanBuffer(Allocator, I_CreateInfo, I_MemoryProperties);
    }

    void FVulkanDriver::
    CreatePipelineCache()
    {
        PipelineCache = MakeUnique<FVulkanPipelineCache>(
            GPU.Context,
            Device.Context,
            GPlatform->GetResourceDirectory() / FPath("Cache/VulkanPipelines.cache")
        );
    }

    FVulkanDescriptorSetLayout FVulkanDriver::
    CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        return FVulkanDescriptorSetLayout{Device.Context, I_Bindings};
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