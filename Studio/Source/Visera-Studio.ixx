module;
#include <Visera-Studio.hpp>
#if !defined(VISERA_OFFSCREEN_MODE)
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#endif
export module Visera.Studio;
#define VISERA_MODULE_NAME "Studio"
import Visera.Core.Global;
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Engine.Event;
import Visera.Runtime.RHI;
import Visera.Runtime.Window;

namespace Visera
{
    export class VISERA_STUDIO_API FStudio : public IGlobalSingleton<FStudio>
    {
    public:
        void inline
        BeginFrame()
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();
            ImGui::Begin("こんにちは");
            ImGui::End();
#endif
        }
        void inline
        EndFrame()
        {
#if !defined(VISERA_OFFSCREEN_MODE)
            static auto& Driver = GRHI->GetDriver();

            ImGui::Render();

            auto& CurrentFrame = Driver->GetCurrentFrame();
            auto& Cmds = *CurrentFrame.DrawCalls->GetHandle();
            auto& Extent = Driver->SwapChain.Extent;
            auto& SwapChainImage = Driver->GetCurrentSwapChainImage();

            auto& CurrentColorRT = CurrentFrame.ColorRT;
            auto  OldColorRTLayout   = CurrentColorRT->GetLayout();

            CurrentFrame.DrawCalls->ConvertImageLayout(
                CurrentColorRT->GetImage(),
                EImageLayout::eTransferSrcOptimal,
                EPipelineStage::eTopOfPipe,
                EAccess::eNone,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite
            );
            CurrentFrame.DrawCalls->ConvertImageLayout(
                SwapChainImage->GetImage(),
                EImageLayout::eTransferDstOptimal,
                EPipelineStage::eTopOfPipe,
                EAccess::eNone,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite
            );
            CurrentFrame.DrawCalls->BlitImage(
                CurrentColorRT->GetImage(),
                SwapChainImage->GetImage());

            CurrentFrame.DrawCalls->ConvertImageLayout(
                CurrentColorRT->GetImage(),
                OldColorRTLayout,
                EPipelineStage::eTopOfPipe,
                EAccess::eNone,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite
            );
            CurrentFrame.DrawCalls->ConvertImageLayout(
                SwapChainImage->GetImage(),
                EImageLayout::ePresentSrcKHR,
                EPipelineStage::eTopOfPipe,
                EAccess::eNone,
                EPipelineStage::eTransfer,
                EAccess::eTransferWrite
            );
            VkRenderingAttachmentInfo ColorAttachment
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = *SwapChainImage->GetHandle(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            };
            VkRenderingInfo RenderingInfo
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = { {0, 0}, Extent },
                .layerCount = 1,
                .colorAttachmentCount   = 1,
                .pColorAttachments      = &ColorAttachment,
            };
            vkCmdBeginRendering(Cmds, &RenderingInfo);

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Cmds);

            vkCmdEndRendering(Cmds);

            ImGui::EndFrame();
#endif
        }
    private:
        FPath Font;

    public:
        FStudio() : IGlobalSingleton("Studio") {}
        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_STUDIO_API TUniquePtr<FStudio>
    GStudio = MakeUnique<FStudio>();

    void FStudio::
    Bootstrap()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        Font = FPath{VISERA_STUDIO_DIR "/Assets/Font/TsangerYunHei.ttf"};

        GEvent->OnFrameBegin.Subscribe([this](){ BeginFrame(); });
        GEvent->OnFrameEnd.Subscribe([this](){ EndFrame(); });

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
        if (!IO.Fonts->AddFontFromFileTTF(Font.GetUTF8Path().c_str()))
        { LOG_ERROR("Failed to load the font \"{}\"!", Font); }
        { LOG_DEBUG("Using font: {}", Font); }

        IO.DisplaySize = ImVec2(
            GWindow->GetWidth(),
            GWindow->GetHeight()
        );
        IO.DisplayFramebufferScale = ImVec2(
            GWindow->GetScaleX(),
            GWindow->GetScaleY()
        );
        //FileSystem::CreateFileIfNotExists(FPath{LayoutFilePath.c_str()});
        //IO.IniFilename =  LayoutFilePath.c_str();
        //IO.ConfigFlags |= Configurations;

        //ImFont* EditorFont = IO.Fonts->AddFontFromFileTTF(VISERA_ENGINE_FONTS_DIR"/HelveticaNeueMedium.ttf", 14);

        ImGui::StyleColorsDark();

        auto& Vulkan = GRHI->GetDriver();

        auto& QueueFamily = *Vulkan->GPU.GraphicsQueueFamilies.begin();
        auto& Queue = Vulkan->Device.GraphicsQueue;

        VkFormat StudioRTFormat = static_cast<VkFormat>(Vulkan->SwapChain.ImageFormat);
        ImGui_ImplVulkan_InitInfo CreateInfo
        {
            .Instance		= *Vulkan->Instance,
            .PhysicalDevice = *Vulkan->GPU.Context,
            .Device			= *Vulkan->Device.Context,
            .QueueFamily	= QueueFamily,
            .Queue			= *Queue,
            .DescriptorPool	= *Vulkan->GetDescriptorPool(),
            .DescriptorPoolSize = 0,

            .MinImageCount	= Vulkan->SwapChain.MinimalImageCount,
            .ImageCount		= Vulkan->GetFrameCount(),
            .PipelineCache	= *Vulkan->GetPipelineCache()->GetHandle(),

            .RenderPass		= nullptr, // Ignored if using dynamic rendering
            .Subpass		= 0,
            .MSAASamples	= VK_SAMPLE_COUNT_1_BIT,

            .UseDynamicRendering = True,
            .PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount    = 1,
                .pColorAttachmentFormats = &StudioRTFormat,
            },
            .Allocator		= nullptr,
            .CheckVkResultFn = [](VkResult I_Error)
            {
                if (I_Error != VK_SUCCESS)
                { LOG_FATAL("ImGUI Vulkan backend Error Code: {}", static_cast<UInt32>(I_Error)); }
            },
        };

        VISERA_ASSERT(GWindow->IsBootstrapped() &&
                      GWindow->GetType() == EWindowType::GLFW);
        LOG_TRACE("Initializing Dear ImGUI GLFW backend.");
        if (!ImGui_ImplGlfw_InitForVulkan(
            static_cast<GLFWwindow*>(GWindow->GetHandle()),
            True // Install callbacks via ImGUI
        ))
        { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }

        LOG_TRACE("Initializing Dear ImGUI Vulkan backend.");
        if (!ImGui_ImplVulkan_Init(&CreateInfo))
        { LOG_FATAL("Failed to initialize Dear ImGUI Vulkan backend!"); }
#endif
    }

    void FStudio::
    Terminate()
    {
#if !defined(VISERA_OFFSCREEN_MODE)
        GRHI->GetDriver()->WaitIdle();
        LOG_TRACE("Terminating Dear ImGUI Vulkan backend.");
        ImGui_ImplVulkan_Shutdown();
        LOG_TRACE("Terminating Dear ImGUI GLFW backend.");
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();
#endif
    }

}