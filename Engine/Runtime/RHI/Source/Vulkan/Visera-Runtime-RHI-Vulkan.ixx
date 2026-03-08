module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Vulkan.Common;
export import Visera.Runtime.RHI.Vulkan.CommandBuffer;
export import Visera.Runtime.RHI.Vulkan.CommandPool;
export import Visera.Runtime.RHI.Vulkan.Image;
export import Visera.Runtime.RHI.Vulkan.Buffer;
export import Visera.Runtime.RHI.Vulkan.Sampler;
export import Visera.Runtime.RHI.Vulkan.Pipeline;
export import Visera.Runtime.RHI.Vulkan.ShaderModule;
export import Visera.Runtime.RHI.Vulkan.RenderTarget;
export import Visera.Runtime.RHI.Vulkan.DescriptorPool;
export import Visera.Runtime.RHI.Vulkan.DescriptorSet;
export import Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
export import Visera.Runtime.RHI.Vulkan.Sync;
       import Visera.Runtime.RHI.Vulkan.Loader;
       import Visera.Runtime.RHI.Vulkan.Allocator;
       import Visera.Core.Log;
       import Visera.Core.Algorithm;
       import Visera.Core.Math.Arithmetic;
       import Visera.Core.Types.Path;
       import Visera.Core.Containers.Set;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer.Shared;
       import Visera.Runtime.Window;
       import Visera.Platform;
       import vulkan_hpp;

export namespace Visera
{
    using EVulkanMemoryProperty = EVMAMemoryProperty;
    using FVulkanGraphicsCommandPool = FVulkanCommandPool<EVulkanQueueFamily::Graphics>;
    using FVulkanTransferCommandPool = FVulkanCommandPool<EVulkanQueueFamily::Transfer>;
    using FVulkanComputeCommandPool  = FVulkanCommandPool<EVulkanQueueFamily::Compute>;

    class VISERA_RUNTIME_API FVulkanDriver
    {
    public:
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
        CreateRenderPipeline(FVulkanPipelineLayout*       I_PipelineLayout,
                             FVulkanShaderModule*         I_VertexShader,
                             FVulkanShaderModule*         I_FragmentShader,
                             const TArray<vk::Format>&    I_ColorFormats,
                             vk::Format                   I_DepthStencilFormat = vk::Format::eUndefined);
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
        [[nodiscard]] FVulkanDescriptorPool
        CreateDescriptorPool(const TArray<vk::DescriptorPoolSize>& I_PoolSizes);

        // Exposed init/destroy steps so FRHI can orchestrate a staged startup.
        void CreateInstance(const FString& I_ApplicationName, UInt32 I_ApplicationVersion, const TArray<const char*>& I_InstanceExtensions);
        void inline DestroyInstance();
        void inline CreateDebugMessenger(); void inline DestroyDebugMessenger();
        void CreateDevice(const FString& I_PreferredGPUName = {});
        void inline DestroyDevice();
        void inline CreateAllocator();      void inline DestroyAllocator();
        void inline CreatePipelineCache();  void inline DestroyPipelineCache();

        void
        WaitDeviceIdle() const;
        void
        WaitSwapChainIdle(FWindow* I_Window) const;

    private:
        FString    ApplicationName    {"Visera"};
        UInt32     ApplicationVersion {vk::makeVersion(1, 0, 0)};
        vk::raii::Context     Context;

        vk::raii::Instance               Instance        {nullptr};
        TArray<const char*>              InstanceLayers;
        TArray<const char*>              InstanceExtensions;
        vk::raii::DebugUtilsMessengerEXT DebugMessenger  {nullptr};
        struct FGPU
        {
            vk::raii::PhysicalDevice Context {nullptr};
            TSet<UInt32> GraphicsQueueFamilies{};
            TSet<UInt32> ComputeQueueFamilies {};
            TSet<UInt32> TransferQueueFamilies{};
            TArray<vk::QueueFamilyProperties> QueueFamilyProperties{};
            vk::PhysicalDeviceProperties  Properties;
            vk::PhysicalDeviceProperties2 Properties2;
            vk::PhysicalDeviceDescriptorIndexingProperties DescriptorIndexingProperties;
            vk::PhysicalDeviceFeatures    Features;
            vk::PhysicalDeviceFeatures2   Features2;
        }GPU;

        struct FDevice
        {
            vk::raii::Device    Context        {nullptr};
            TArray<const char*> Layers;
            TArray<const char*> Extensions;
            UInt32              GraphicsQueueFamilyIndex = 0U;
            vk::raii::Queue     GraphicsQueue  {nullptr};
            UInt32              TransferQueueFamilyIndex = 0U;
            vk::raii::Queue     TransferQueue  {nullptr};
            UInt32              ComputeQueueFamilyIndex  = 0U;
            vk::raii::Queue     ComputeQueue   {nullptr};
        }Device;

        FVulkanLoader*        Loader         {nullptr};
        FVulkanAllocator*     Allocator      {nullptr};
        FVulkanPipelineCache* PipelineCache  {nullptr};

        struct FSwapChain
        {
            vk::raii::SurfaceKHR            Surface     {nullptr};
            vk::raii::SwapchainKHR          Context     {nullptr};
            vk::raii::SwapchainKHR          OldContext  {nullptr};
            vk::Extent2D                    Extent      {0U, 0U};
            TArray<FVulkanSwapChainImage>   Images      {};
            TArray<FVulkanImageView>        ImageViews;
            UInt32                          Cursor      {0};
            vk::ImageUsageFlags             ImageUsage  {vk::ImageUsageFlagBits::eColorAttachment |
                                                         vk::ImageUsageFlagBits::eTransferDst};
            vk::Format                      ImageFormat {vk::Format::eB8G8R8A8Srgb};
            vk::ColorSpaceKHR               ColorSpace  {vk::ColorSpaceKHR::eSrgbNonlinear};
            UInt32                          MinimalImageCount{3};
            vk::PresentModeKHR              PresentMode {vk::PresentModeKHR::eFifo};
            vk::SharingMode                 SharingMode {vk::SharingMode::eExclusive};
            vk::CompositeAlphaFlagBitsKHR   CompositeAlpha {vk::CompositeAlphaFlagBitsKHR::eOpaque};
            Bool                            bClipped       {True};

            [[nodiscard]] inline FVulkanImageView*
            GetCurrentImageView() { return &ImageViews[Cursor]; }
            [[nodiscard]] inline FVulkanImage*
            GetCurrentImage() { return &Images[Cursor]; }
        };
        TMap<FWindow*, FSwapChain*> SwapChains;

        void inline PickPhysicalDevice(const FString& I_PreferredGPUName);
        inline FVulkanDriver*
        AddInstanceLayer(const char* I_Layer)           { LOG_TRACE("Adding instance layer: {}", I_Layer); InstanceLayers.EmplaceBack(I_Layer);         return this; }
        inline FVulkanDriver*
        AddInstanceExtension(const char* I_Extension)   { LOG_TRACE("Adding instance extension: {}", I_Extension); InstanceExtensions.PushBack(I_Extension);    return this; }
        inline FVulkanDriver*
        AddDeviceLayer(const char* I_Layer)             { LOG_TRACE("Adding device layer: {}", I_Layer); Device.Layers.PushBack(I_Layer);              return this; }
        inline FVulkanDriver*
        AddDeviceExtension(const char* I_Extension)     { LOG_TRACE("Adding device extension: {}", I_Extension); Device.Extensions.PushBack(I_Extension);      return this; }

        void inline
        CollectInstanceLayersAndExtensions(const TArray<const char*>& I_InstanceExtensions);
        void inline
        CollectDeviceLayersAndExtensions();

        static vk::Bool32
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
        [[nodiscard]] const FVulkanPipelineCache*
        GetPipelineCache() const { return PipelineCache; }
        [[nodiscard]] inline const vk::raii::Instance&
        GetInstance() const { return Instance; }
        [[nodiscard]] inline const FGPU&
        GetGPU() const { return GPU; }
        [[nodiscard]] inline const FDevice&
        GetDevice() const { return Device; }
        [[nodiscard]] FSwapChain*
        GetSwapChain(FWindow* I_Window);
        [[nodiscard]] const FSwapChain*
        GetSwapChain(FWindow* I_Window) const;
        [[nodiscard]] Bool
        HasSwapChain(FWindow* I_Window) const;
        void
        CreateSwapChain(FWindow* I_Window);
        void
        DestroySwapChain(FWindow* I_Window);
        void
        RecreateSwapChain(FWindow* I_Window);
        [[nodiscard]] Bool
        WaitNextFrame(FWindow* I_Window, FVulkanSemaphore* I_SignalSemaphore);
        [[nodiscard]] Bool
        Present(FWindow* I_Window, FVulkanSemaphore* I_WaitSemaphore);
    public:
        FVulkanDriver();
        ~FVulkanDriver();

        // Driver should not be copied or moved
        FVulkanDriver(const FVulkanDriver&) = delete;
        FVulkanDriver& operator=(const FVulkanDriver&) = delete;
        FVulkanDriver(FVulkanDriver&&) = delete;
        FVulkanDriver& operator=(FVulkanDriver&&) = delete;
    };

    FVulkanDriver::
    FVulkanDriver()
    {
#if defined(VISERA_ON_APPLE_SYSTEM)
        FPath VulkanICDPath = FPlatform::GetResourceDirectory() / FPath{"Vulkan/MoltenVK_icd.json"};
        if (!FPlatform::SetEnvironmentVariable(
            "VK_ICD_FILENAMES", FText{VulkanICDPath.GetString()}))
        { LOG_FATAL("Failed to set VK_ICD_FILENAMES as {}!", VulkanICDPath); }
#if !defined(VISERA_RELEASE_MODE)
        FPath VulkanLayerPath = FPlatform::GetResourceDirectory() / FPath{"Vulkan"};
        if (!FPlatform::SetEnvironmentVariable(
            "VK_LAYER_PATH", FText{VulkanLayerPath.GetString()}))
        { LOG_FATAL("Failed to set VK_LAYER_PATH as {}!", VulkanLayerPath); }
#endif
#endif
        Loader = new FVulkanLoader();
    };

    FVulkanDriver::
    ~FVulkanDriver()
    {
        if (*Device.Context) { WaitDeviceIdle(); }
        DestroyPipelineCache();
        for (auto& [W, SC] : SwapChains)
        {
            if (SC)
            {
                SC->ImageViews.Clear();
                SC->Images.Clear();
                SC->Context.clear();
                delete SC;
            }
        }
        SwapChains.Clear();
        DestroyAllocator();
        DestroyDevice();
        DestroyDebugMessenger();
        DestroyInstance();
        delete Loader;
    }

    void FVulkanDriver::
    CreateAllocator()
    {
        if (Allocator) { return; }
        Allocator = new FVulkanAllocator
        (
            vk::ApiVersion13,
            Instance,
            GPU.Context,
            Device.Context
        );
    }

    void FVulkanDriver::
    DestroyAllocator()
    {
        if (!Allocator) { return; }
        delete Allocator;
        Allocator = nullptr;
    }

    void FVulkanDriver::
    DestroyInstance()
    {
        if (!*Instance) { return; }
        Instance.clear();
    }

    void FVulkanDriver::
    DestroyDebugMessenger()
    {
        if (!*DebugMessenger) { return; }
        DebugMessenger.clear();
    }

    void FVulkanDriver::
    DestroyDevice()
    {
        if (!*Device.Context) { return; }
        Device.Context.clear();
    }

    FVulkanDriver::FSwapChain* FVulkanDriver::
    GetSwapChain(FWindow* I_Window)
    {
        auto It = SwapChains.Find(I_Window);
        return (It != SwapChains.end() && It->second) ? It->second : nullptr;
    }

    const FVulkanDriver::FSwapChain* FVulkanDriver::
    GetSwapChain(FWindow* I_Window) const
    {
        auto It = SwapChains.Find(I_Window);
        return (It != SwapChains.end() && It->second) ? It->second : nullptr;
    }

    Bool FVulkanDriver::
    HasSwapChain(FWindow* I_Window) const
    {
        auto It = SwapChains.Find(I_Window);

        if (It != SwapChains.end() && It->second)
        { return It->second->Images.GetSize() > 0; }

        return False;
    }

    void FVulkanDriver::
    CreateSwapChain(FWindow* I_Window)
    {
        VISERA_ASSERT(*Instance && I_Window);

        FSwapChain* SC = nullptr;
        Bool bRecreate = False;
        if (auto It = SwapChains.Find(I_Window); It != SwapChains.end())
        {
            bRecreate = True;
            SC = It->second;
            SC->Extent = vk::Extent2D{ I_Window->GetWidth(), I_Window->GetHeight() };
            SC->Cursor = 0;
            WaitDeviceIdle();
            SC->ImageViews.Clear();
            SC->Images.Clear();
            SC->Context.clear();
        }
        else
        {
            auto InstanceHandle = *Instance;
            auto SurfaceHandle  = static_cast<vk::SurfaceKHR::NativeType>(
                I_Window->GetPlatformWindow()->CreateVulkanSurface(InstanceHandle));
            if (!SurfaceHandle) { LOG_ERROR("Failed to create vulkan surface!"); return; }
            SC = new FSwapChain();
            SC->Surface = vk::raii::SurfaceKHR(Instance, SurfaceHandle);
            SC->Extent = vk::Extent2D{ I_Window->GetWidth(), I_Window->GetHeight() };
            SC->PresentMode = (I_Window->GetPresentMode() == EPresentMode::VSync)
                ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
            SwapChains[I_Window] = SC;
        }

        if (!*Device.Context || !SC) { return; }

        FSwapChain& Entry = *SC;
        const vk::SurfaceKHR SurfaceHandle = *Entry.Surface;

        if (!bRecreate)
        {
            Bool bFoundRequiredPresentMode {False};
            auto Result = GPU.Context.getSurfacePresentModesKHR(SurfaceHandle);
            if (!Result.has_value())
            { LOG_FATAL("Failed to get the required Present Mode!"); }
            for (const auto PresentModes = std::move(*Result);
                 const auto& PresentMode : PresentModes)
            {
                if (PresentMode == Entry.PresentMode)
                { bFoundRequiredPresentMode = True; break; }
            }
            if (!bFoundRequiredPresentMode)
            {
                LOG_WARN("Failed to find required present mode for SwapChain! Using FIFO by default.");
                Entry.PresentMode = vk::PresentModeKHR::eFifo;
            }
            Bool bFoundRequiredFormatAndColorSpace {False};
            auto FormatResult = GPU.Context.getSurfaceFormatsKHR(SurfaceHandle);
            if (!FormatResult.has_value())
            { LOG_FATAL("Failed to get the required format or color space!"); }
            for (const auto AvailableFormats = std::move(*FormatResult);
                 const auto& AvailableFormat : AvailableFormats)
            {
                if (AvailableFormat.format == Entry.ImageFormat && AvailableFormat.colorSpace == Entry.ColorSpace)
                { bFoundRequiredFormatAndColorSpace = True; break; }
            }
            if (!bFoundRequiredFormatAndColorSpace)
            { LOG_FATAL("Failed to find required format and color space for swapchain images!"); }
        }

        auto Result = GPU.Context.getSurfaceCapabilitiesKHR(SurfaceHandle);
        if (!Result.has_value())
        { LOG_FATAL("Failed to get the required surface capabilities!"); }
        const auto SurfaceCapabilities = std::move(*Result);
        {
            if (SurfaceCapabilities.currentExtent.width != Math::UpperBound<UInt32>())
            { Entry.Extent = SurfaceCapabilities.currentExtent; }
            else
            {
                Math::Clamp(&Entry.Extent.width,  SurfaceCapabilities.minImageExtent.width,  SurfaceCapabilities.maxImageExtent.width);
                Math::Clamp(&Entry.Extent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);
            }
        }
        if (Entry.Extent.width == 0 || Entry.Extent.height == 0)
        {
            LOG_TRACE("({}) Skip SwapChain creation while minimized (extent:[{},{}]).", ApplicationName, Entry.Extent.width, Entry.Extent.height);
            return;
        }
        Entry.MinimalImageCount = Math::Max(SurfaceCapabilities.minImageCount, Entry.MinimalImageCount);
        const auto CreateInfo = vk::SwapchainCreateInfoKHR{}
            .setSurface         (SurfaceHandle)
            .setMinImageCount   (Entry.MinimalImageCount)
            .setImageFormat     (Entry.ImageFormat)
            .setImageColorSpace (Entry.ColorSpace)
            .setImageExtent     (Entry.Extent)
            .setImageUsage      (Entry.ImageUsage)
            .setImageSharingMode(Entry.SharingMode)
            .setImageArrayLayers(1U)
            .setPreTransform   (SurfaceCapabilities.currentTransform)
            .setCompositeAlpha (Entry.CompositeAlpha)
            .setPresentMode    (Entry.PresentMode)
            .setClipped       (Entry.bClipped);
        {
            auto R = Device.Context.createSwapchainKHR(CreateInfo);
            if (!R.has_value())
            { LOG_FATAL("Failed to create Vulkan SwapChain!"); }
            else
            { Entry.Context = std::move(*R); }
        }
        {
            auto R = Entry.Context.getImages();
            if (!R.has_value())
            { LOG_FATAL("Failed to retrieve Vulkan Swapchain Images!"); }
            auto SwapChainImages = std::move(*R);
            Entry.Images.Clear();
            Entry.Images.Reserve(SwapChainImages.size());
            for (UInt8 Idx = 0; Idx < SwapChainImages.size(); ++Idx)
            {
                Entry.Images.EmplaceBack(
                    Allocator,
                    SwapChainImages[Idx],
                    vk::ImageType::e2D,
                    vk::Extent3D{ Entry.Extent.width, Entry.Extent.height, 1 },
                    Entry.ImageFormat,
                    Entry.ImageUsage);
            }
        }
        for (auto& Image : Entry.Images)
        {
            Entry.ImageViews.EmplaceBack(&Image, vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
        }
        LOG_DEBUG("({}) Created SwapChain for window (title:{})", ApplicationName, I_Window->GetTitle());
    }

    void FVulkanDriver::
    DestroySwapChain(FWindow* I_Window)
    {
        auto It = SwapChains.Find(I_Window);
        if (It == SwapChains.end() || !It->second) { return; }
        if (*Device.Context)
        {
            // RHI has already waited for this swap chain's in-flight fences before calling us.
            It->second->ImageViews.Clear();
            It->second->Images.Clear();
            It->second->Context.clear();
        }
        delete It->second;
        SwapChains.Erase(I_Window);
    }

    void FVulkanDriver::
    WaitDeviceIdle() const
    {
        if (*Device.Context) { (void)Device.Context.waitIdle(); }
    }

    void FVulkanDriver::
    WaitSwapChainIdle(FWindow* I_Window) const
    {
        (void)I_Window;
        if (*Device.Context && *Device.GraphicsQueue)
        { (void)Device.GraphicsQueue.waitIdle(); }
    }

    void FVulkanDriver::
    RecreateSwapChain(FWindow* I_Window)
    {
        if (!I_Window) { return; }
        const UInt32 W = I_Window->GetWidth(), H = I_Window->GetHeight();
        if (W == 0 || H == 0)
        {
            LOG_TRACE("({}) Skip SwapChain recreation while minimized ({}x{}).", ApplicationName, W, H);
            return;
        }
        VISERA_ASSERT(SwapChains.Contains(I_Window));
        CreateSwapChain(I_Window);
        LOG_DEBUG("({}) Recreated SwapChain ({}x{}) for window (title:{}).", ApplicationName, W, H, I_Window->GetTitle());
    }

    void FVulkanDriver::
    DestroyPipelineCache()
    {
        if (!PipelineCache) { return; }
        delete PipelineCache;
        PipelineCache = nullptr;
    }

    void FVulkanDriver::
    CreateInstance(const FString& I_ApplicationName, UInt32 I_ApplicationVersion, const TArray<const char*>& I_InstanceExtensions)
    {
        VISERA_ASSERT(Loader);
        if (*Instance) { return; }
        ApplicationName = I_ApplicationName;
        ApplicationVersion = I_ApplicationVersion;
        CollectInstanceLayersAndExtensions(I_InstanceExtensions);

        // Check Layers
        {
            auto Result = Context.enumerateInstanceLayerProperties();
            if (!Result.has_value())
            { LOG_FATAL("Failed to enumerate instance layer properties!"); }

            const auto LayerProperties = std::move(*Result);
            for (const auto& Layer : InstanceLayers)
            {
                if (Algorithm::NoneOf(LayerProperties,
                    [&Layer](auto const& LayerProperty)
                    { return strcmp(LayerProperty.layerName, Layer) == 0; }))
                {
                    LOG_FATAL("Required instance layer {} is not supported!", Layer);
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
                if (Algorithm::NoneOf(ExtensionProperties,
                    [&Extension](auto const& ExtensionProperty)
                    { return strcmp(ExtensionProperty.extensionName, Extension) == 0; }))
                {
                    LOG_FATAL("Required instance extension {} is not supported!", Extension);
                }
            }
        }

        auto Flags = vk::InstanceCreateFlags{};
#if defined(VISERA_ON_APPLE_SYSTEM)
        Flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

        const auto AppInfo = vk::ApplicationInfo{}
            .setPApplicationName    (ApplicationName.Data())
            .setApplicationVersion  (ApplicationVersion)
            .setPEngineName         ("Visera")
            .setEngineVersion       (vk::makeVersion(1, 0, 0))
            .setApiVersion          (vk::ApiVersion13);
        const auto CreateInfo = vk::InstanceCreateInfo{}
            .setPApplicationInfo        (&AppInfo)
            .setEnabledLayerCount       (InstanceLayers.GetSize())
            .setPpEnabledLayerNames     (InstanceLayers.Data())
            .setEnabledExtensionCount   (InstanceExtensions.GetSize())
            .setPpEnabledExtensionNames (InstanceExtensions.Data())
            .setFlags                   (Flags)
        ;

        auto Result = Context.createInstance(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Instance!"); }
        else
        {
            Instance = std::move(*Result);
            Loader->Load(Instance);
        }
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
    PickPhysicalDevice(const FString& I_PreferredGPUName)
    {
        auto Result = Instance.enumeratePhysicalDevices();
        if (!Result.has_value())
        { LOG_FATAL("Failed to find PhysicalDevices with Vulkan support!"); }

        auto PhysicalDeviceCandidates = std::move(*Result);

        struct FCandidate
        {
            vk::raii::PhysicalDevice Dev{nullptr};
            vk::PhysicalDeviceProperties Props{};
        };

        // Split: Discrete first, then Integrated/others. Prefer discrete; if discrete matches, skip integrated.
        TArray<FCandidate> DiscreteCands;
        TArray<FCandidate> NonDiscreteCands;
        for (auto& Dev : PhysicalDeviceCandidates)
        {
            FCandidate C{};
            C.Dev   = Dev;
            C.Props = Dev.getProperties();
            if (C.Props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            { DiscreteCands.EmplaceBack(std::move(C)); }
            else
            { NonDiscreteCands.EmplaceBack(std::move(C)); }
        }

        auto TryFindSuitable = [&](TArray<FCandidate>& I_Cands) -> Bool
        {
            const auto It = Algorithm::FindIf(I_Cands, [&](auto const& Candidate)
        {
            LOG_TRACE("Current GPU candidate: {}.", Candidate.Props.deviceName.data());

            LOG_TRACE("Checking Vulkan API Version...");
            Bool bSuitable = Candidate.Props.apiVersion >= vk::ApiVersion13;
            if (!bSuitable) { return False; }

            LOG_TRACE("Checking Queue Families...");
            auto QueueFamilies = Candidate.Dev.getQueueFamilyProperties();
            GPU.QueueFamilyProperties.Clear();
            for (const auto& QueueFamily : QueueFamilies)
            {
                GPU.QueueFamilyProperties.EmplaceBack(QueueFamily);
            }

            GPU.GraphicsQueueFamilies.Clear();
            GPU.ComputeQueueFamilies.Clear();
            GPU.TransferQueueFamilies.Clear();

            for (UInt32 Idx = 0; Idx < QueueFamilies.size(); ++Idx)
            {
                auto& QueueFamily = QueueFamilies[Idx];
                if (vk::QueueFlagBits::eGraphics & QueueFamily.queueFlags)
                { GPU.GraphicsQueueFamilies.Insert(Idx); }
                if (vk::QueueFlagBits::eCompute & QueueFamily.queueFlags)
                { GPU.ComputeQueueFamilies.Insert(Idx); }
                if (vk::QueueFlagBits::eTransfer & QueueFamily.queueFlags)
                { GPU.TransferQueueFamilies.Insert(Idx); }
            }
            bSuitable = !GPU.GraphicsQueueFamilies.IsEmpty() &&
                        !GPU.ComputeQueueFamilies.IsEmpty()  &&
                        !GPU.TransferQueueFamilies.IsEmpty();
            if (!bSuitable) { return False; }

            LOG_TRACE("Checking Extension supporting...");
            auto Result = Candidate.Dev.enumerateDeviceExtensionProperties();
            if (!Result.has_value())
            {LOG_FATAL("Failed to enumerate device extension properties!"); }

            auto Extensions = std::move(*Result);
            // Build hash set for O(1) lookup instead of O(n) linear search
            TSet<FStringView> ExtSet;
            for (const auto& Ext : Extensions)
            {
                ExtSet.Emplace(Ext.extensionName.data());
            }

            for (const char* RequiredExtension : Device.Extensions)
            {
                if (!ExtSet.Contains(RequiredExtension))
                { bSuitable = False; break; }
            }

            if (bSuitable)
            {
                GPU.Context = Candidate.Dev;
            }
            return bSuitable;
            });
            return It != I_Cands.end();
        };

        auto MatchesGPUName = [&](const FCandidate& C) -> Bool
        {
            const FStringView Name(C.Props.deviceName.data());
            return Name.Contains(FStringView(I_PreferredGPUName));
        };

        auto PickByGPUName = [&](TArray<FCandidate>& I_Cands) -> Bool
        {
            const auto It = Algorithm::FindIf(I_Cands, MatchesGPUName);
            if (It == I_Cands.end()) { return False; }
            const auto& Candidate = *It;
            DEBUG_ONLY_FIELD(
            {
                LOG_TRACE("Expected GPU found: {}; running full suitability checks.", Candidate.Props.deviceName.data());
                auto QueueFamilies = Candidate.Dev.getQueueFamilyProperties();
                GPU.QueueFamilyProperties.Clear();
                for (const auto& QueueFamily : QueueFamilies)
                { GPU.QueueFamilyProperties.EmplaceBack(QueueFamily); }
                GPU.GraphicsQueueFamilies.Clear();
                GPU.ComputeQueueFamilies.Clear();
                GPU.TransferQueueFamilies.Clear();
                for (UInt32 Idx = 0; Idx < QueueFamilies.size(); ++Idx)
                {
                    auto& QF = QueueFamilies[Idx];
                    if (vk::QueueFlagBits::eGraphics & QF.queueFlags) GPU.GraphicsQueueFamilies.Insert(Idx);
                    if (vk::QueueFlagBits::eCompute & QF.queueFlags) GPU.ComputeQueueFamilies.Insert(Idx);
                    if (vk::QueueFlagBits::eTransfer & QF.queueFlags) GPU.TransferQueueFamilies.Insert(Idx);
                }
                Bool bSuitable = !GPU.GraphicsQueueFamilies.IsEmpty() && !GPU.ComputeQueueFamilies.IsEmpty() && !GPU.TransferQueueFamilies.IsEmpty();
                if (!bSuitable) { LOG_FATAL("Expected GPU {} failed suitability checks!", Candidate.Props.deviceName.data()); }
                auto ExtResult = Candidate.Dev.enumerateDeviceExtensionProperties();
                if (!ExtResult.has_value()) { LOG_FATAL("Failed to enumerate device extension properties!"); }
                TSet<FStringView> ExtSet;
                for (const auto& Ext : *ExtResult) ExtSet.Emplace(Ext.extensionName.data());
                for (const char* Req : Device.Extensions)
                { if (std::strcmp(Req, vk::KHRSurfaceExtensionName) == 0) continue; if (!ExtSet.Contains(Req)) { LOG_FATAL("Expected GPU {} missing extension {}!", Candidate.Props.deviceName.data(), Req); } }
            });
            GPU.Context = Candidate.Dev;
            return True;
        };

        if (!I_PreferredGPUName.IsEmpty())
        {
            if (!PickByGPUName(DiscreteCands) && !PickByGPUName(NonDiscreteCands))
            { LOG_FATAL("Expected GPU \"{}\" not found!", I_PreferredGPUName); }
        }
        else if (!TryFindSuitable(DiscreteCands) && !TryFindSuitable(NonDiscreteCands))
        { LOG_FATAL("Failed to find a suitable PhysicalDevice!"); }

        GPU.Properties  = GPU.Context.getProperties();
        auto Properties2 = GPU.Context.getProperties2<
                        vk::PhysicalDeviceProperties2,
                        vk::PhysicalDeviceDescriptorIndexingProperties>();
        GPU.Properties2 = std::move(Properties2.get<vk::PhysicalDeviceProperties2>());
        GPU.DescriptorIndexingProperties = std::move(Properties2.get<vk::PhysicalDeviceDescriptorIndexingProperties>());
        GPU.Features    = GPU.Context.getFeatures();
        GPU.Features2   = GPU.Context.getFeatures2();
        LOG_INFO("({}) Selected GPU: {}", ApplicationName, GPU.Properties.deviceName.data());
    }

    void FVulkanDriver::
    CreateDevice(const FString& I_PreferredGPUName)
    {
        VISERA_ASSERT(Loader);
        if (*Device.Context) { return; }
        CollectDeviceLayersAndExtensions();
        PickPhysicalDevice(I_PreferredGPUName);

        // ---- Decide queue families and queue indices (prefer same family, different queue index) ----
        struct FQueueSelection
        {
            UInt32 Family = vk::QueueFamilyIgnored;
            UInt32 Index  = 0;
        };

        auto GetQueueCount = [&](UInt32 I_Family) -> UInt32
        {
            VISERA_ASSERT(I_Family != vk::QueueFamilyIgnored);
            VISERA_ASSERT(I_Family < GPU.QueueFamilyProperties.GetSize());
            return GPU.QueueFamilyProperties[I_Family].queueCount;
        };

        auto PickFirstFamily = [&](const TSet<UInt32>& I_Families) -> UInt32
        {
            VISERA_ASSERT(!I_Families.IsEmpty());
            return *I_Families.begin();
        };

        const UInt32 GraphicsFamily = PickFirstFamily(GPU.GraphicsQueueFamilies);

        FQueueSelection GraphicsQueueCandidate{ GraphicsFamily, 0 };
        FQueueSelection TransferQueueCandidate{ GraphicsFamily, 0 };
        FQueueSelection ComputeQueueCandidate { PickFirstFamily(GPU.ComputeQueueFamilies), 0 };

        // How many queues we will request from GraphicsFamily.
        UInt32 GraphicsFamilyRequestedQueues = 1;

        // Try to allocate an extra queue from the SAME graphics family for Transfer
        // (so no ownership transfer is needed; only semaphore sync).
        if (GetQueueCount(GraphicsFamily) >= 2)
        {
            TransferQueueCandidate = { GraphicsFamily, 1 };
            GraphicsFamilyRequestedQueues = 2;
        }
        else
        {
            // Fallback: try a different transfer family (may require ownership transfer later).
            UInt32 TransferFamily = PickFirstFamily(GPU.TransferQueueFamilies);
            if (TransferFamily == GraphicsFamily)
            {
                auto It = GPU.TransferQueueFamilies.begin();
                ++It;
                if (It != GPU.TransferQueueFamilies.end())
                {
                    TransferFamily = *It;
                }
                else
                {
                    LOG_WARN("No extra queue in Graphics family and no separate Transfer family found; "
                             "using Graphics queue as Transfer.");
                    TransferQueueCandidate = { GraphicsFamily, 0 };
                    TransferFamily = GraphicsFamily;
                }
            }
            TransferQueueCandidate = { TransferFamily, 0 };
        }

        // Try to allocate another queue from Graphics family for Compute, if possible.
        // Prefer queue index 2 if we already used 0 (graphics) and 1 (transfer).
        if (ComputeQueueCandidate.Family == GraphicsFamily)
        {
            const UInt32 WantIndex = (TransferQueueCandidate.Family == GraphicsFamily && TransferQueueCandidate.Index == 1) ? 2U : 1U;
            if (GetQueueCount(GraphicsFamily) > WantIndex)
            {
                ComputeQueueCandidate = { GraphicsFamily, WantIndex };
                GraphicsFamilyRequestedQueues = Math::Max(GraphicsFamilyRequestedQueues, WantIndex + 1);
            }
            else
            {
                // Fallback: try a different compute family.
                UInt32 ComputeFamily = PickFirstFamily(GPU.ComputeQueueFamilies);
                if (ComputeFamily == GraphicsFamily)
                {
                    auto It = GPU.ComputeQueueFamilies.begin();
                    ++It;
                    if (It != GPU.ComputeQueueFamilies.end())
                    {
                        ComputeFamily = *It;
                    }
                    else
                    {
                        LOG_WARN("Failed to find separate Compute family and no spare queue in Graphics family; "
                                 "using Graphics queue as Compute.");
                        ComputeQueueCandidate = { GraphicsFamily, 0 };
                        ComputeFamily = GraphicsFamily;
                    }
                }
                ComputeQueueCandidate = { ComputeFamily, 0 };
            }
        }

        // ---- Build unique vk::DeviceQueueCreateInfo per family ----
        // We need stable memory for priorities arrays.
        // For each family we request queueCount N with priorities[N].
        struct FFamilyQueues
        {
            UInt32 Family = vk::QueueFamilyIgnored;
            UInt32 Count  = 0;
            TArray<Float> Priorities{};
            vk::DeviceQueueCreateInfo Info;
        };

        // Collect required queue counts per family
        // (Graphics family might request 1~3 queues; others request 1).
        TArray<FFamilyQueues> Families;
        Families.Reserve(3);

        auto AddOrUpdateFamily = [&](UInt32 I_Family, UInt32 I_Count)
        {
            FFamilyQueues* FoundFamily = nullptr;
            for (auto& Family : Families)
            {
                if (Family.Family == I_Family)
                {
                    FoundFamily = &Family;
                    break;
                }
            }
            if (FoundFamily == nullptr)
            {
                Families.EmplaceBack();
                auto& NewFamily = Families[Families.GetSize() - 1];
                NewFamily.Family = I_Family;
                NewFamily.Count  = 0;
                FoundFamily = &NewFamily;
            }
            FoundFamily->Count = Math::Max(FoundFamily->Count, I_Count);
        };

        AddOrUpdateFamily(GraphicsFamily, GraphicsFamilyRequestedQueues);
        if (TransferQueueCandidate.Family != GraphicsFamily) { AddOrUpdateFamily(TransferQueueCandidate.Family, 1); }
        if (ComputeQueueCandidate.Family  != GraphicsFamily && ComputeQueueCandidate.Family != TransferQueueCandidate.Family)
        {
            AddOrUpdateFamily(ComputeQueueCandidate.Family, 1);
        }

        // Fill queue create infos
        constexpr Float Priority = 1.0f;
        TArray<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfos;
        DeviceQueueCreateInfos.Reserve(Families.GetSize());

        for (auto& Family : Families)
        {
            Family.Priorities.Clear();
            Family.Priorities.Reserve(Family.Count);
            for (UInt32 Idx = 0; Idx < Family.Count; ++Idx)
            {
                Family.Priorities.EmplaceBack(Priority);
            }

            Family.Info = vk::DeviceQueueCreateInfo{}
                .setQueueFamilyIndex(Family.Family)
                .setQueueCount     (Family.Count)
                .setPQueuePriorities(Family.Priorities.Data())
            ;

            DeviceQueueCreateInfos.EmplaceBack(Family.Info);
        }
        // First build each feature struct explicitly
        auto GPUFeatures = vk::PhysicalDeviceFeatures2{};
        GPUFeatures.features
            .setSamplerAnisotropy                           (GPU.Features2.features.samplerAnisotropy)
        ;
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
            GPUFeatures,
            Vulkan11Features,
            Vulkan12Features,
            Vulkan13Features,
        };
        const auto CreateInfo = vk::DeviceCreateInfo{}
            .setPNext                   (&FeatureChain.get<vk::PhysicalDeviceFeatures2>())
            .setQueueCreateInfoCount    (DeviceQueueCreateInfos.GetSize())
            .setPQueueCreateInfos       (DeviceQueueCreateInfos.Data())
            .setEnabledExtensionCount   (Device.Extensions.GetSize())
            .setPpEnabledExtensionNames (Device.Extensions.Data())
        ;
        //Create Device
        auto Result = GPU.Context.createDevice(CreateInfo);
        if (!Result.has_value())
        { LOG_FATAL("Failed to create Vulkan Device!"); }
        else
        { Device.Context = std::move(*Result); }
        
        // Store queue family indices
        Device.GraphicsQueueFamilyIndex = GraphicsQueueCandidate.Family;
        Device.TransferQueueFamilyIndex = TransferQueueCandidate.Family;
        Device.ComputeQueueFamilyIndex  = ComputeQueueCandidate.Family;

        // Get Queues (family + queueIndex)
        Device.GraphicsQueue = Device.Context.getQueue(GraphicsQueueCandidate.Family, GraphicsQueueCandidate.Index);
        Device.TransferQueue = Device.Context.getQueue(TransferQueueCandidate.Family, TransferQueueCandidate.Index);
        Device.ComputeQueue  = Device.Context.getQueue(ComputeQueueCandidate.Family,  ComputeQueueCandidate.Index);
        Loader->Load(Device.Context);

        LOG_DEBUG("({}) Selected Device Queues: "
                  "Graphics(family:{}, idx:{}) "
                  "Transfer(family:{}, idx:{}) "
                  "Compute(family:{}, idx:{})",
                  ApplicationName,
                  GraphicsQueueCandidate.Family, GraphicsQueueCandidate.Index,
                  TransferQueueCandidate.Family, TransferQueueCandidate.Index,
                  ComputeQueueCandidate.Family,  ComputeQueueCandidate.Index);
    }

    void FVulkanDriver::
    CollectInstanceLayersAndExtensions(const TArray<const char*>& I_InstanceExtensions)
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

        for (const char* Ext : I_InstanceExtensions)
        {
            this->AddInstanceExtension(Ext);
        }
    }

    void FVulkanDriver::
    CollectDeviceLayersAndExtensions()
    {
        this->AddDeviceExtension(vk::EXTDescriptorIndexingExtensionName)
            ->AddDeviceExtension(vk::KHRMaintenance6ExtensionName)
            ->AddDeviceExtension(vk::KHRSwapchainExtensionName)
#if defined(VISERA_ON_APPLE_SYSTEM)
            ->AddDeviceExtension("VK_KHR_portability_subset")
#endif
        ;
    }

    Bool FVulkanDriver::
    WaitNextFrame(FWindow* I_Window, FVulkanSemaphore* I_SignalSemaphore)
    {
        VISERA_ASSERT(I_Window && I_SignalSemaphore);
        auto* SC = GetSwapChain(I_Window);
        if (!SC) { LOG_ERROR("Failed to find swapchain for window (title:{}).", I_Window->GetTitle()); VISERA_ASSERT(False); return True; }
        const auto AcquireInfo = vk::AcquireNextImageInfoKHR{}
            .setSwapchain   (SC->Context)
            .setTimeout     (kAcquireTimeoutNs)
            .setSemaphore   (I_SignalSemaphore->GetHandle())
            .setFence       (nullptr)
            .setDeviceMask  (1)
        ;
        uint32_t NextImageIndex = 0;
        const auto RawResult = static_cast<vk::Result>(
            Device.Context.getDispatcher()->vkAcquireNextImage2KHR(
                static_cast<vk::Device::NativeType>(*Device.Context),
                reinterpret_cast<const vk::AcquireNextImageInfoKHR::NativeType*>(&AcquireInfo),
                &NextImageIndex));
        if (RawResult == vk::Result::eSuccess || RawResult == vk::Result::eSuboptimalKHR)
        {
            SC->Cursor = NextImageIndex;
            return True;
        }
        if (RawResult == vk::Result::eErrorOutOfDateKHR)
        {
            LOG_TRACE("({}) AcquireNextImage returned {}.", ApplicationName, RawResult);
            return False;
        }
        if (RawResult == vk::Result::eTimeout)
        {
            LOG_WARN("({}) AcquireNextImage timed out (swapchain may be blocked).", ApplicationName);
            return False;
        }
        LOG_ERROR("({}) Failed to acquire the next Vulkan SwapChain Image! VkResult={}", ApplicationName, RawResult);
        return False;
    }

    Bool FVulkanDriver::
    Present(FWindow* I_Window, FVulkanSemaphore* I_WaitSemaphore)
    {
        auto* SC = GetSwapChain(I_Window);
        if (!SC) { return True; }
        VISERA_ASSERT(I_WaitSemaphore != nullptr);
        auto WaitSemaphore = I_WaitSemaphore->GetHandle();
        const auto PresentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount  (1)
            .setPWaitSemaphores     (&WaitSemaphore)
            .setSwapchainCount      (1)
            .setPSwapchains         (&(*SC->Context))
            .setPImageIndices       (&SC->Cursor)
            .setPResults            (nullptr)
        ;
        const auto Result = static_cast<vk::Result>(
            Device.GraphicsQueue.getDispatcher()->vkQueuePresentKHR(*Device.GraphicsQueue, &(*PresentInfo)));
        if (Result == vk::Result::eErrorOutOfDateKHR)
        {
            LOG_TRACE("({}) Present returned {}.", ApplicationName, Result);
            return False;
        }
        if (Result != vk::Result::eSuccess && Result != vk::Result::eSuboptimalKHR)
        {
            LOG_ERROR("({}) Present failed with VkResult {}", ApplicationName, Result);
            return False;
        }
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
        auto WaitSemaphore   = I_WaitSemaphore  ? I_WaitSemaphore->GetHandle()   : nullptr;
        auto SignalSemaphore = I_SignalSemaphore? I_SignalSemaphore->GetHandle() : nullptr;
        auto Fence           = I_Fence          ? I_Fence->GetHandle()           : nullptr;

        auto WaitSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(WaitSemaphore)
            .setStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
        ;
        auto SignalSemaphoreInfo = vk::SemaphoreSubmitInfo{}
            .setSemaphore(SignalSemaphore)
            .setStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
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
            QueueFamilyIndex = Device.GraphicsQueueFamilyIndex;
            break;
        case EVulkanQueueFamily::Transfer:
            QueueFamilyIndex = Device.TransferQueueFamilyIndex;
            break;
        case EVulkanQueueFamily::Compute:
            QueueFamilyIndex = Device.ComputeQueueFamilyIndex;
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
        VISERA_ASSERT(!I_SPIRVShader.IsEmpty());
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
    CreateRenderPipeline(FVulkanPipelineLayout*       I_PipelineLayout,
                         FVulkanShaderModule*         I_VertexShader,
                         FVulkanShaderModule*         I_FragmentShader,
                         const TArray<vk::Format>&     I_ColorFormats,
                         vk::Format                   I_DepthStencilFormat)
    {
        LOG_TRACE("Creating a Vulkan Render Pipeline.");
        VISERA_ASSERT(I_PipelineLayout != nullptr);
        VISERA_ASSERT(I_VertexShader != nullptr);
        VISERA_ASSERT(I_FragmentShader != nullptr);
        VISERA_ASSERT(I_ColorFormats.GetSize() > 0 && I_ColorFormats.GetSize() <= 8);
        auto NewRenderPipeline = FVulkanRenderPipeline(
               std::move(*I_PipelineLayout),
               std::move(*I_VertexShader),
               std::move(*I_FragmentShader));
        NewRenderPipeline.Settings.ColorRTFormats = I_ColorFormats;
        NewRenderPipeline.Settings.DepthRTFormat  = I_DepthStencilFormat;
        NewRenderPipeline.Settings.StencilRTFormat = I_DepthStencilFormat;
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
        if (PipelineCache) { return; }
        FString UUIDString;
        for (size_t i = 0; i < vk::UuidSize; ++i)
        { UUIDString += FString::Format("{:02x}", static_cast<UInt32>(GPU.Properties.pipelineCacheUUID[i])); }
        const FPath CachePath = FPlatform::GetCacheDirectory() / FPath{FString::Format("RHIPipelines.{}.cache", UUIDString)};
        PipelineCache = new FVulkanPipelineCache(GPU.Context, Device.Context, CachePath);
    }

    FVulkanDescriptorSetLayout FVulkanDriver::
    CreateDescriptorSetLayout(const TArray<vk::DescriptorSetLayoutBinding>& I_Bindings)
    {
        return FVulkanDescriptorSetLayout{Device.Context, I_Bindings};
    }

    FVulkanDescriptorPool FVulkanDriver::
    CreateDescriptorPool(const TArray<vk::DescriptorPoolSize>& I_PoolSizes)
    {
        return FVulkanDescriptorPool(Device.Context, I_PoolSizes, GPU.Properties.limits.maxBoundDescriptorSets);
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