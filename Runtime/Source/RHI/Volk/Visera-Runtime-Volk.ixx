module;
#include <Visera-Runtime.hpp>
#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vulkan/vulkan.hpp>
export module Visera.Runtime.RHI.Volk;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera
{
    class FVolk
    {
    public:
        void inline
        Load(const vk::Instance& I_Instance) const;
        void inline
        Load(const vk::Device& I_Device) const;

        FVolk();
        ~FVolk();
    };

    void FVolk::
    Load(const vk::Instance& I_Instance) const
    {
        volkLoadInstance(I_Instance);
    }

    void FVolk::
    Load(const vk::Device& I_Device) const
    {
        volkLoadDevice(I_Device);
    }

    FVolk::
    FVolk()
    {
        if (volkInitialize() != VK_SUCCESS)
        {
            throw SRuntimeError("Failed to initialize the Volk!");
        }
    }

    FVolk::
    ~FVolk()
    {
        volkFinalize();
    }

}
