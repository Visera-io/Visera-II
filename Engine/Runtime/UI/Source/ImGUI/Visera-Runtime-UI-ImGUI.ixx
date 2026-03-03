module;
#include <Visera-UI.hpp>
#include "DEFAULT_FONT_COMPRESSED.inl"
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
export module Visera.Runtime.UI.ImGUI;
#define VISERA_MODULE_NAME "Runtime.UI.ImGUI"
import Visera.Runtime.UI.Context;
import Visera.Runtime.Global.Service;
import Visera.Runtime.Graphics;
import Visera.Runtime.RHI;
import Visera.Runtime.Window;
import Visera.Runtime.Input;
import Visera.Core.Log;
import Visera.Core.Types.String;

export namespace Visera
{
    class VISERA_RUNTIME_API FImGUIContext : public IUIContext
    {
    public:
        struct VISERA_RUNTIME_API FDebugWindow
        {
            Bool Status = False;
            FDebugWindow() = delete;
            VISERA_NOINLINE
            FDebugWindow(FStringView I_Title) { Status = ImGui::Begin(I_Title.Data()); }
            VISERA_NOINLINE
            ~FDebugWindow() { ImGui::End(); }
            [[nodiscard]] explicit
            operator Bool() const { return Status; }
        };
        [[nodiscard]] VISERA_NOINLINE FDebugWindow
        Window(FStringView I_Title) const { return FDebugWindow{I_Title};  }
        VISERA_NOINLINE void
        Text(FStringView I_Text) const { ImGui::TextUnformatted(I_Text.Data()); }
        VISERA_NOINLINE Bool
        Button(FStringView I_Label) const { return ImGui::Button(I_Label.Data()); }
        VISERA_NOINLINE Bool
        Slider(FStringView I_Label, Float* I_Value, Float I_Min, Float I_Max) const { return ImGui::SliderFloat(I_Label.Data(), I_Value, I_Min, I_Max); }

        void BeginFrame() override
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
        void EndFrame() override
        {
            ImGui::EndFrame();
        }

    private:
        const FRHI* RHI         {nullptr};
        FString     RuntimeName {"Unknown"};

    public:
        FImGUIContext(FWindow* I_Window, FGraphics* I_Graphics, FInput* I_Input, FStringView I_RuntimeName = "Unknown")
            : RuntimeName(I_RuntimeName)
        {
            (void)I_Input; // Reserved for input routing from FInput to ImGui
            RHI = I_Graphics ? I_Graphics->GetRHI() : nullptr;
            if (!RHI) { LOG_FATAL("DebugUI requires Graphics with RHI!"); return; }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            auto& IO = ImGui::GetIO();

            ImFontConfig FontConfig;
            if (!IO.Fonts->AddFontFromMemoryCompressedTTF(
                DEFAULT_FONT_COMPRESSED,
                sizeof(DEFAULT_FONT_COMPRESSED)))
            { LOG_ERROR("({}) Failed to load the default font!", RuntimeName); }

            IO.DisplaySize = ImVec2(
                I_Window->GetWidth(),
                I_Window->GetHeight()
            );
            IO.DisplayFramebufferScale = ImVec2(
                I_Window->GetScaleX(),
                I_Window->GetScaleY()
            );

            ImGui::StyleColorsDark();

            auto Vulkan = RHI->GetDriver();
            auto* SC = Vulkan->GetSwapChain(I_Window);
            if (!SC) { LOG_FATAL("DebugUI requires swapchain for window!"); }

            const VkFormat ColorRTFormat = static_cast<VkFormat>(ERHIFormat::R8G8B8A8_sRGB);
            ImGui_ImplVulkan_InitInfo CreateInfo
            {
                .Instance		= *Vulkan->GetInstance(),
                .PhysicalDevice = *Vulkan->GetGPU().Context,
                .Device			= *Vulkan->GetDevice().Context,
                .QueueFamily	= Vulkan->GetDevice().GraphicsQueueFamilyIndex,
                .Queue			= *Vulkan->GetDevice().GraphicsQueue,
                .DescriptorPool	= nullptr,
                .DescriptorPoolSize = 100,

                .MinImageCount	= SC->MinimalImageCount,
                .ImageCount		= static_cast<UInt32>(SC->Images.GetSize()),
                .PipelineCache	= *Vulkan->GetPipelineCache()->GetHandle(),

                .RenderPass		= nullptr,
                .Subpass		= 0,
                .MSAASamples	= VK_SAMPLE_COUNT_1_BIT,

                .UseDynamicRendering = True,
                .PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .colorAttachmentCount    = 1,
                    .pColorAttachmentFormats = &ColorRTFormat,
                },
                .Allocator		= nullptr,
                .CheckVkResultFn = [](VkResult I_Error)
                {
                    if (I_Error != VK_SUCCESS)
                    { LOG_FATAL("ImGUI Vulkan backend Error Code: {}", static_cast<UInt32>(I_Error)); }
                },
            };

            LOG_TRACE("({}) Initializing Dear ImGUI GLFW backend.", RuntimeName);
            if (!ImGui_ImplGlfw_InitForVulkan(
                static_cast<GLFWwindow*>(I_Window->GetPlatformWindow()->GetHandle()),
                True
            ))
            { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }

            LOG_TRACE("({}) Initializing Dear ImGUI Vulkan backend.", RuntimeName);
            if (!ImGui_ImplVulkan_Init(&CreateInfo))
            { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }
        }

        ~FImGUIContext() override
        {
            if (RHI)
            {
                RHI->GetDriver()->WaitIdle();
            }
            LOG_TRACE("({}) Terminating Dear ImGUI Vulkan backend.", RuntimeName);
            ImGui_ImplVulkan_Shutdown();
            LOG_TRACE("({}) Terminating Dear ImGUI GLFW backend.", RuntimeName);
            ImGui_ImplGlfw_Shutdown();

            ImGui::DestroyContext();
        }
    };
}
