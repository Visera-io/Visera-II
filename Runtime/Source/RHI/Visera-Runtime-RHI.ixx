module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
export module Visera.Runtime.RHI;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan;
import Visera.Runtime.RHI.Volk;
import Visera.Runtime.RHI.VMA;
import Visera.Runtime.Window;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_RUNTIME_API FRHI : public IGlobalSingleton
    {
    public:
        inline FRHI*
        AddInstanceExtension(const char* I_Extension) { InstanceExtensions.push_back(I_Extension); return this; };
        inline FRHI*
        AddDeviceExtension(const char* I_Extension)   { DeviceExtensions.push_back(I_Extension); return this;  };

    private:
        TArray<const char*> InstanceExtensions;
        TArray<const char*> DeviceExtensions;

        TUniquePtr<FVulkan> Vulkan;

    public:
        FRHI() : IGlobalSingleton("RHI") {}
        ~FRHI() override;
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

    private:
        void inline
        CollectInstanceExtensions();
        void inline
        CollectDeviceExtensions();
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping RHI.");

        try
        {
            GVolk->Bootstrap();

            CollectInstanceExtensions();
            CollectDeviceExtensions();

            Vulkan = MakeUnique<FVulkan>(InstanceExtensions, DeviceExtensions);
            GVolk->Load(Vulkan->GetInstance());
        }
        catch (const SRuntimeError& Error)
        {
            LOG_FATAL("{}", Error.what());
        }

        Statue = EStatues::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_DEBUG("Terminating RHI.");
        Vulkan.reset();

        GVolk->Terminate();
        Statue = EStatues::Terminated;
    }

    FRHI::
    ~FRHI()
    {
        if (IsBootstrapped())
        {
            std::cerr << "[FATAL] RHI must be terminated properly!\n";
            std::exit(EXIT_FAILURE);
        }
    }

    void FRHI::
    CollectInstanceExtensions()
    {
#if defined(VISERA_DEBUG_MODE)
        // Debug Extensions
        this->AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
            ->AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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

    void FRHI::
    CollectDeviceExtensions()
    {

    }

}
