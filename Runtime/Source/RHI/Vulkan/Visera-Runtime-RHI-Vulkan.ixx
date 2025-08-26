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

        CreateInstance();
        GVolk->Load(Instance);

        CreateDebugMessenger();

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
        CollectInstanceLayersAndExtensions();

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
#endif()

        auto CreateInfo = vk::InstanceCreateInfo{}
            .setPApplicationInfo(&AppInfo)
            .setEnabledLayerCount(InstanceLayers.size())
            .setPpEnabledLayerNames(InstanceLayers.data())
            .setEnabledExtensionCount(InstanceExtensions.size())
            .setPpEnabledExtensionNames(InstanceExtensions.data())
            .setFlags(Flags)
        ;

        Instance = FInstance(Context, CreateInfo);
    }

    void FVulkan::
    CreateDebugMessenger()
    {
#if defined(VISERA_DEBUG_MODE)
        vk::DebugUtilsMessageSeverityFlagsEXT SeverityFlags
        {

        };
        vk::DebugUtilsMessageTypeFlagsEXT MessageTypeFlags
        {

        };
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
#endif()
    }

    void FVulkan::
    PickPhysicalDevice()
    {
        VISERA_ASSERT(Instance != nullptr);

        auto GPUCandidates = Instance.enumeratePhysicalDevices();
        if (GPUCandidates.empty())
        { throw SRuntimeError("failed to find GPUs with Vulkan support!"); }

        for (const auto& GPUCandidate : GPUCandidates)
        {
            GPU = FPhysicalDevice(GPUCandidate);
            break;
        }
    }

    void FVulkan::
    CreateDevice()
    {
        VISERA_ASSERT(Instance != nullptr);

        CollectDeviceLayersAndExtensions();

        auto CreateInfo = vk::DeviceCreateInfo{};
    }


    void FVulkan::
    CollectInstanceLayersAndExtensions()
    {
#if defined(VISERA_DEBUG_MODE)
        // Debug Layers
        this->AddInstanceLayer("VK_LAYER_KHRONOS_validation")
        ;
        // Debug Extensions
        this->AddInstanceExtension(vk::EXTDebugUtilsExtensionName)
        ;
#else
        // Release Extensions
#endif

#if defined(VISERA_ON_APPLE_SYSTEM)
        this->AddInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif()

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
        if (GWindow->IsBootstrapped())
        {

        }
    }
}
