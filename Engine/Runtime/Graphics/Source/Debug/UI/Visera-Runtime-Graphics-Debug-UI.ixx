module;
#include <Visera-Graphics.hpp>
#if !defined(VISERA_OFFSCREEN_MODE)
#include "DEFAULT_FONT_COMPRESSED.inl"
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#endif
export module Visera.Runtime.Graphics.Debug.UI;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.Global.Service;
import Visera.Runtime.RHI;
import Visera.Runtime.Window;
import Visera.Core.Log;
import Visera.Core.Types.String;

export namespace Visera::Graphics
{
    class VISERA_RUNTIME_API FDebugUI
    {
#if !defined(VISERA_OFFSCREEN_MODE)
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
        Slider(FStringView I_Label, TMutable<Float> I_Value, Float I_Min, Float I_Max) const { return ImGui::SliderFloat(I_Label.Data(), I_Value, I_Min, I_Max); }
#endif
        void inline
        BeginFrame()
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
#endif
        }
        void inline
        EndFrame()
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            ImGui::EndFrame();
#endif
        }

    private:
        FRHI*       RHI         {nullptr};
        FString     RuntimeName {"Unknown"};

    public:
        FDebugUI(FWindow* I_Window, FRHI* I_RHI, FStringView I_RuntimeName = "Unknown") : RHI(I_RHI), RuntimeName(I_RuntimeName)
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            auto& IO = ImGui::GetIO();

            /*[FreeType Blending]
            FreeType assumes blending in linear space rather than gamma space.
            For correct results you need to be using sRGB and convert to linear space in the pixel shader output.
            The default Dear ImGui styles will be impacted by this change (alpha values will need tweaking).
            [FT_Render_Glyph](https://freetype.org/freetype2/docs/reference/ft2-glyph_retrieval.html#ft_render_glyph)
            */
            ImFontConfig FontConfig;
            //FontConfig.FontLoaderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
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
            //FileSystem::CreateFileIfNotExists(FPath{LayoutFilePath.c_str()});
            //IO.IniFilename =  LayoutFilePath.c_str();
            //IO.ConfigFlags |= Configurations;

            //ImFont* EditorFont = IO.Fonts->AddFontFromFileTTF(VISERA_ENGINE_FONTS_DIR"/HelveticaNeueMedium.ttf", 14);

            ImGui::StyleColorsDark();

            auto Vulkan = I_RHI->GetDriver();
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
                .DescriptorPool	= nullptr, //[TODO] :Add a descriptor pool for ImGui
                .DescriptorPoolSize = 100,

                .MinImageCount	= SC->MinimalImageCount,
                .ImageCount		= static_cast<UInt32>(SC->Images.GetSize()),
                .PipelineCache	= *Vulkan->GetPipelineCache()->GetHandle(),

                .RenderPass		= nullptr, // Ignored if using dynamic rendering
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
                True // Install callbacks via ImGUI
            ))
            { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }

            LOG_TRACE("({}) Initializing Dear ImGUI Vulkan backend.", RuntimeName);
            if (!ImGui_ImplVulkan_Init(&CreateInfo))
            { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }

            // if (!GRHI->DebugUIDrawCalls.TryBind([&I_Window]
            // (FRHIDrawCalls* I_CommandBuffer, FRHIImageView* I_ColorRT)
            // {
            //     ImGui::Render();
            //
            //     auto Cmds = I_CommandBuffer->GetHandle();
            //
            //     VkRenderingAttachmentInfo ColorAttachment
            //     {
            //         .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            //         .imageView   = I_ColorRT->GetHandle(),
            //         .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            //         .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
            //         .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            //     };
            //     VkRenderingInfo RenderingInfo
            //     {
            //         .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            //         .renderArea = { {0, 0}, { I_Window->GetWidth(), I_Window->GetHeight() } },
            //         .layerCount = 1,
            //         .colorAttachmentCount   = 1,
            //         .pColorAttachments      = &ColorAttachment,
            //     };
            //     vkCmdBeginRendering(Cmds, &RenderingInfo);
            //
            //     ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Cmds);
            //
            //     vkCmdEndRendering(Cmds);
            // }))
            // { LOG_FATAL("Failed to bind ImGui drawcalls to backend!"); }
#endif
        }

        ~FDebugUI()
        {
    #if !defined(VISERA_OFFSCREEN_MODE)
            if (RHI)
            {
                RHI->GetDriver()->WaitIdle();
            }
            LOG_TRACE("({}) Terminating Dear ImGUI Vulkan backend.", RuntimeName);
            ImGui_ImplVulkan_Shutdown();
            LOG_TRACE("({}) Terminating Dear ImGUI GLFW backend.", RuntimeName);
            ImGui_ImplGlfw_Shutdown();

            ImGui::DestroyContext();
    #endif
        }
    };
}