module;
#include <Visera-Runtime.hpp>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
export module Visera.Runtime.RHI.Vulkan;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Volk;
import Visera.Runtime.Window;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkan
    {
    public:
        using FContext        = vk::raii::Context;
        using FInstance       = vk::raii::Instance;
        using FDebugMessenger = vk::raii::DebugUtilsMessengerEXT;
        using FSurface        = vk::raii::SurfaceKHR;
        using FPhysicalDevice = vk::raii::PhysicalDevice;
        using FDevice         = vk::raii::Device;
        using FQueue          = vk::raii::Queue;

        [[nodiscard]] inline const FInstance&
        GetInstance() const { return Instance; }
        [[nodiscard]] inline const FDevice&
        GetDevice()   const { return Device.Handle; }

        FVulkan();
        ~FVulkan();

    private:
        vk::ApplicationInfo AppInfo;
        FContext        Context;
        FInstance       Instance        {nullptr};
        FDebugMessenger DebugMessenger  {nullptr};

        FSurface        Surface         {nullptr};

        struct
        {
            FPhysicalDevice Handle {nullptr};
            TSet<UInt32> GraphicsQueueFamilies{};
            TSet<UInt32> PresentQueueFamilies {};
            TSet<UInt32> ComputeQueueFamilies {};
            TSet<UInt32> TransferQueueFamilies{};
        }GPU;

        struct
        {
            FDevice Handle          {nullptr};
            FQueue  GraphicsQueue   {nullptr};
        }Device;


        TArray<const char*> InstanceLayers;
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceLayers;
        TArray<const char*> DeviceExtensions;

    private:
        void inline
        CreateInstance();
        void inline
        CreateDebugMessenger();
        void inline
        CreateSurface();
        void inline
        PickPhysicalDevice();
        void inline
        CreateDevice();

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
                //LOG_INFO("\n{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            {
                LOG_DEBUG("\n{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            {
                LOG_WARN("\n{}", I_CallbackData->pMessage);
            }
            else if (I_Severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            {
                LOG_ERROR("\n{}", I_CallbackData->pMessage);
            }
            else
            {
                LOG_FATAL("Unknown Vulkan Debug Message Severity {}", Int32(I_Severity));
            }
            return vk::False; // Always return VK_FALSE
        }
    };

    FVulkan::
    FVulkan()
    {
        AppInfo = vk::ApplicationInfo{}
                .setPApplicationName    ("Visera")
                .setApplicationVersion  (VK_MAKE_VERSION(1, 0, 0))
                .setPEngineName         ("Visera")
                .setEngineVersion       (VK_MAKE_VERSION(1, 0, 0))
                .setApiVersion          (vk::ApiVersion13);

        GVolk->Bootstrap();

        CollectInstanceLayersAndExtensions();
        CreateInstance();
        GVolk->Load(Instance);

        CreateDebugMessenger();

        CreateSurface();

        CollectDeviceLayersAndExtensions();
        PickPhysicalDevice();
        CreateDevice();
        GVolk->Load(Device.Handle);
    }

    FVulkan::
    ~FVulkan()
    {
        GVolk->Terminate();
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

        auto CreateInfo = vk::InstanceCreateInfo{}
            .setPApplicationInfo        (&AppInfo)
            .setEnabledLayerCount       (InstanceLayers.size())
            .setPpEnabledLayerNames     (InstanceLayers.data())
            .setEnabledExtensionCount   (InstanceExtensions.size())
            .setPpEnabledExtensionNames (InstanceExtensions.data())
            .setFlags                   (Flags)
        ;

        Instance = FInstance(Context, CreateInfo);
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
        if (!GWindow->IsBootstrapped())
        { return LOG_DEBUG("Skipped creating VkSurfaceKHR because the GWindow is NOT bootstrapped."); }

        VkSurfaceKHR SurfaceHandle;

        if(glfwCreateWindowSurface(
            *Instance,
            GWindow->GetHandle(),
            nullptr,
            &SurfaceHandle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create Vulkan Surface!"); }

        Surface = FSurface(Instance, SurfaceHandle);
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
            { GPU.Handle = PhysicalDeviceCandidate; }
            return bSuitable;
        });

        if (SelectedPhysicalDevice == PhysicalDeviceCandidates.end())
        { throw SRuntimeError("Failed to find a suitable PhysicalDevice!"); }

        LOG_INFO("Selected PhysicalDevice: {}", GPU.Handle.getProperties().deviceName.data());
    }

    void FVulkan::
    CreateDevice()
    {
        VISERA_ASSERT(GPU.Handle != nullptr);

        FString Buffer;
        for (auto& GQueue : GPU.GraphicsQueueFamilies) {
                Buffer += std::format("{} ", GQueue);
        }
        LOG_INFO("Graphics QFs {}", Buffer);
        Buffer.clear();
        for (auto& GQueue : GPU.ComputeQueueFamilies) {
            Buffer += std::format("{} ", GQueue);
        }
        LOG_INFO("Compute QFs {}", Buffer);
        Buffer.clear();
        for (auto& GQueue : GPU.PresentQueueFamilies) {
            Buffer += std::format("{} ", GQueue);
        }
        LOG_INFO("Present QFs {}", Buffer);
        Buffer.clear();
        for (auto& GQueue : GPU.TransferQueueFamilies) {
            Buffer += std::format("{} ", GQueue);
        }
        LOG_INFO("Transfer QFs {}", Buffer);
        Buffer.clear();

        const Float Priority = 0.0f;
        auto DeviceQueueCreateInfo = vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(*GPU.GraphicsQueueFamilies.begin())
            .setQueueCount(1)
            .setQueuePriorities({Priority});

        // Create a chain of feature structures
        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        FeatureChain =
        {
            {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
            {.dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
            {.extendedDynamicState = true }   // Enable extended dynamic state from the extension
        };

        vk::DeviceCreateInfo CreateInfo
        {
            .pNext                      = &FeatureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount       = 1,
            .pQueueCreateInfos          = &DeviceQueueCreateInfo,
            .enabledExtensionCount      = static_cast<uint32_t>(DeviceExtensions.size()),
            .ppEnabledExtensionNames    = DeviceExtensions.data()
        };

        Device.Handle = FDevice{GPU.Handle, CreateInfo};

        Device.GraphicsQueue = FQueue{Device.Handle, *GPU.GraphicsQueueFamilies.begin(), 0};
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
