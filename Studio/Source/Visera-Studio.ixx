module;
#include <Visera-Studio.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
export module Visera.Studio;
#define VISERA_MODULE_NAME "Studio"

namespace Visera
{
    class VISERA_STUDIO_API FStudio
    {
    public:
        void inline
        Bootstrap();
        void inline
        Terminate();

    private:

    };

    export inline VISERA_STUDIO_API
    TUniquePtr<FStudio> GStudio = MakeUnique<FStudio>();

    void FStudio::
    Bootstrap()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto& IO = ImGui::GetIO();

        // Configuration
        // FileSystem::CreateFileIfNotExists(FPath{LayoutFilePath.c_str()});
        // IO.IniFilename =  LayoutFilePath.c_str();
        // IO.ConfigFlags |= Configurations;
        //
        // ImFont* EditorFont = IO.Fonts->AddFontFromFileTTF(VISERA_ENGINE_FONTS_DIR"/HelveticaNeueMedium.ttf", 14);
        //
        // ImGui::StyleColorsDark();
        // //ImGui::StyleColorsLight();
        //
        // auto* API = RHI::GetAPI();
        // auto& QueueFamily = API->GetDevice().GetQueueFamily(RHI::EQueueFamily::Graphics);
        //
        // ImGui_ImplVulkan_InitInfo CreateInfo
        // {
        //     .Instance		= API->GetInstance().GetHandle(),
        //     .PhysicalDevice = API->GetGPU().GetHandle(),
        //     .Device			= API->GetDevice().GetHandle(),
        //     .QueueFamily	= QueueFamily.Index,
        //     .Queue			= QueueFamily.Queues[0],
        //     .DescriptorPool	= RHI::GetGlobalDescriptorPool()->GetHandle(),
        //     .RenderPass		= EditorRenderPass->GetHandle(), // Ignored if using dynamic rendering
				    //
        //     .MinImageCount	= UInt32(RHI::GetSwapchainFrameCount()),
        //     .ImageCount		= UInt32(RHI::GetSwapchainFrameCount()),
        //     .MSAASamples	= VK_SAMPLE_COUNT_1_BIT,
        //
        //     .PipelineCache	= API->GetGraphicsPipelineCache().GetHandle(),
        //     .Subpass		= 0,
        //
        //     .DescriptorPoolSize = 0,
        //
        //     .Allocator		= API->GetAllocationCallbacks(),
        //     .CheckVkResultFn = [](VkResult Err)
        //     {
        //         if (Err != VK_SUCCESS)
        //         { VE_LOG_ERROR("ImGUI Vulkan Error Code: {}", UInt32(Err)); }
        //     },
        // };
        //
        // ImGui_ImplGlfw_InitForVulkan(Window::GetHandle(), True); // Install callbacks via ImGUI
			     //
        // auto VulkanInstanceHandle = API->GetInstance().GetHandle();
        // ImGui_ImplVulkan_Init(&CreateInfo);
        //
        // ImGuiDescriptorSetLayout = RHI::CreateDescriptorSetLayout()
        //     ->AddBinding(0, RHI::EDescriptorType::CombinedImageSampler, 1, RHI::EShaderStage::Fragment)
        //     ->Build();
    }

    void FStudio::
    Terminate()
    {

    }

}
