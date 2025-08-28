module;
#include <Visera-Runtime.hpp>
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
        using FPhysicalDevice = vk::raii::PhysicalDevice;
        using FDevice         = vk::raii::Device;

        [[nodiscard]] inline const FInstance&
        GetInstance() const { return Instance; }
        [[nodiscard]] inline const FDevice&
        GetDevice()   const { return Device; }

        FVulkan();
        ~FVulkan();

    private:
        FContext        Context;
        FInstance       Instance        {nullptr};
        FDebugMessenger DebugMessenger  {nullptr};
        FPhysicalDevice GPU             {nullptr};
        FDevice         Device          {nullptr};

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
        GVolk->Bootstrap();

        CollectInstanceLayersAndExtensions();
        CreateInstance();
        GVolk->Load(Instance);

        CreateDebugMessenger();

        CollectDeviceLayersAndExtensions();
        PickPhysicalDevice();
        CreateDevice();
        //GVolk->Load(Device);
    }

    FVulkan::
    ~FVulkan()
    {
        GVolk->Terminate();
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
    PickPhysicalDevice()
    {
        VISERA_ASSERT(Instance != nullptr);

        auto GPUCandidates = Instance.enumeratePhysicalDevices();
        if (GPUCandidates.empty())
        { throw SRuntimeError("Failed to find GPUs with Vulkan support!"); }

        const auto SelectedGPU =
            std::ranges::find_if(GPUCandidates,
       [&](auto const & GPUCandidate)
            {
                auto QueueFamilies = GPUCandidate.getQueueFamilyProperties();
                Bool bSuitable = GPUCandidate.getProperties().apiVersion >= VK_API_VERSION_1_3;
                const auto QueueFamilyPropertiesIter =
                        std::ranges::find_if(QueueFamilies,
                    [](const vk::QueueFamilyProperties& qfp )
                        {
                            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
                        } );

                bSuitable = bSuitable && (QueueFamilyPropertiesIter != QueueFamilies.end());
                auto Extensions = GPUCandidate.enumerateDeviceExtensionProperties( );
                Bool bFound = False;
                for (auto const & RequiredExtension : DeviceExtensions)
                {
                    auto ExtensionIter =
                        std::ranges::find_if(Extensions,
                    [RequiredExtension](auto const & Extension)
                        {return strcmp(Extension.extensionName, RequiredExtension) == 0;}
                        );
                    bFound |= (ExtensionIter != Extensions.end());
                }
                bSuitable |= bFound;

                if (bSuitable) { GPU = GPUCandidate; }
                return bSuitable;
        });
        if (SelectedGPU == GPUCandidates.end())
        { throw SRuntimeError("failed to find a suitable GPU!"); }

        LOG_INFO("Selected GPU: {}", GPU.getProperties().deviceName.data());
    }

    void FVulkan::
    CreateDevice()
    {
        VISERA_ASSERT(GPU != nullptr);

        auto CreateInfo = vk::DeviceCreateInfo{};
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
