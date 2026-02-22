module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Debug;
export import Visera.Runtime.Graphics.Scene;
export import Visera.Runtime.Graphics.Material;
export import Visera.Runtime.Graphics.Renderer;
export import Visera.Runtime.Graphics.RenderPass;
export import Visera.Runtime.Graphics.Context;
       import Visera.Runtime.Graphics.Texture;
       import Visera.Runtime.Global;
       import Visera.Runtime.AssetHub;
       import Visera.Runtime.RHI;
       import Visera.Runtime.Window;
       import Visera.Core.Image;
       import Visera.Core.Containers.Map;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.String;
       import Visera.Core.Log;

export namespace Visera
{
   class VISERA_RUNTIME_API FGraphics : public IGlobalService
   {
   public:
      template<Concepts::Renderer T, typename... Args>
      TSharedPtr<T>
      CreateRenderer(Args&&... I_Args)
      {
         auto R = MakeShared<T>(RHI, Scene, std::forward<Args>(I_Args)...);
         Renderers.PushBack(R);
         return R;
      }

      [[nodiscard]] TSharedPtr<FMaterial>
      LoadMaterial(const FPath& I_MaterialFile);

      void
      Render();
      void
      Present();

   private:
      TSharedPtr<FAssetHub> AssetHub;
      TWeakPtr<FWindow>     WindowWeak;
      TSharedPtr<FRHI>      RHI;
      TSharedPtr<FScene>    Scene;
      FRenderContext        RenderContext;
      TArray<TSharedPtr<IRenderer>> Renderers;

      static ESurfaceType
      ParseSurfaceType(const FString& I_Str);

      static ERHIFormat
      ParseFormat(const FString& I_Str);

      static ERHIFormat
      PixelFormatToRHIFormat(EPixelFormat I_Fmt);

   public:
      FGraphics(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
          : IGlobalService(I_Name, I_Registry, I_Config)
      {
         Dependencies =
         {
            EName::AssetHub,
            EName::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
            if (auto RHIWeak = GetService<FRHI>(EName::RHI); auto RHIShared = RHIWeak.Lock())
            { RHI = RHIShared; }
            else
            { LOG_FATAL("Failed to get RHI service!"); return False; }

            if (auto AHWeak = GetService<FAssetHub>(EName::AssetHub); auto AHShared = AHWeak.Lock())
            { AssetHub = AHShared; }
            else
            { LOG_FATAL("Failed to get AssetHub service!"); return False; }

            WindowWeak = GetService<FWindow>(EName::Window);
            Scene = MakeShared<FScene>();
            return True;
         }))
         { LOG_FATAL("Failed to bind bootstrap function!"); }

         if (!OnTerminate.TryBind([this]
         {
            for (auto& R : Renderers)
            { if (R) { R->Teardown(); } }
            Renderers.Clear();
            return True;
         }))
         { LOG_FATAL("Failed to bind terminate function!"); }
      }
   };

   // --- FGraphics::LoadMaterial ---

   TSharedPtr<FMaterial> FGraphics::
   LoadMaterial(const FPath& I_MaterialFile)
   {
      auto JSONOpt = FJSON::Load(I_MaterialFile);
      if (!JSONOpt.HasValue())
      { LOG_ERROR("LoadMaterial: failed to parse {}.", I_MaterialFile); return nullptr; }
      const FJSON& JSON = JSONOpt.GetValue();

      const FString VertPath = JSON.GetString(TJSONRoute<"Shader.Vert">());
      const FString FragPath = JSON.GetString(TJSONRoute<"Shader.Frag">());
      // Surface: object { Type, Format } or legacy string (Type only)
      FString SurfStr = "Opaque";
      if (auto opt = JSON.TryGetString(TJSONRoute<"Surface.Type">()); opt.HasValue())
         SurfStr = std::move(opt.GetValue());
      else if (auto opt = JSON.TryGetString("Surface"); opt.HasValue())
         SurfStr = std::move(opt.GetValue());
      const FString FormatStr = JSON.GetString(TJSONRoute<"Surface.Format">(), "B8G8R8A8_sRGB");
      const FString BaseColorPath = JSON.GetString(TJSONRoute<"Textures.BaseColor">());

      if (VertPath.IsEmpty() || FragPath.IsEmpty() || BaseColorPath.IsEmpty())
      { LOG_ERROR("LoadMaterial: missing required fields in {}.", I_MaterialFile); return nullptr; }

      auto VertAsset = AssetHub->LoadShader(FPath{VertPath});
      auto FragAsset = AssetHub->LoadShader(FPath{FragPath});
      if (!VertAsset || !FragAsset)
      { LOG_ERROR("LoadMaterial: failed to load shaders ({}, {}).", VertPath, FragPath); return nullptr; }

      auto ImageAsset = AssetHub->LoadImage(FPath{BaseColorPath});
      if (!ImageAsset)
      { LOG_ERROR("LoadMaterial: failed to load base color texture {}.", BaseColorPath); return nullptr; }

      // Create RHI shader modules
      FRHIShaderID VertShader = RHI->CreateShader(FRHIShaderCreateInfo{
         .SPIRV      = VertAsset->GetSPIRV(),
         .Reflection = VertAsset->GetReflection(),
      });
      FRHIShaderID FragShader = RHI->CreateShader(FRHIShaderCreateInfo{
         .SPIRV      = FragAsset->GetSPIRV(),
         .Reflection = FragAsset->GetReflection(),
      });

      // Build descriptor set from merged (Vert + Frag) reflection, deduplicated by (Set, Binding)
      const auto& VertRefl = VertAsset->GetReflection();
      const auto& FragRefl = FragAsset->GetReflection();
      TMap<UInt64, FRHIShaderLayout::FResource> MergedResources;
      auto AddResource = [&MergedResources](const FRHIShaderLayout::FResource& Res)
      {
         const UInt64 Key = (static_cast<UInt64>(Res.Set) << 32) | Res.Binding;
         auto It = MergedResources.Find(Key);
         if (It != MergedResources.end())
         { It->second.Stages = static_cast<ERHIShaderStage>(static_cast<UInt32>(It->second.Stages) | static_cast<UInt32>(Res.Stages)); }
         else
         { MergedResources.Insert(Key, Res); }
      };
      for (const auto& Res : VertRefl.Resources) { AddResource(Res); }
      for (const auto& Res : FragRefl.Resources) { AddResource(Res); }

      TArray<FRHIDescriptorSetLayoutBinding> DSBindings;
      for (const auto& [_, Res] : MergedResources)
      {
         DSBindings.PushBack(FRHIDescriptorSetLayoutBinding{
            .Binding = static_cast<UInt8>(Res.Binding),
            .Type    = Res.Type,
            .Count   = Res.ArrayCount,
            .Stages  = Res.Stages,
         });
      }

      // Create RHI resources
      const auto& Img = ImageAsset->GetImage();
      ERHIFormat TexFormat = PixelFormatToRHIFormat(Img.GetPixelFormat());
      FRHITextureID BaseColorTex = RHI->CreateTexture(FRHITextureCreateInfo{
         .Width   = Img.GetWidth(),
         .Height  = Img.GetHeight(),
         .Depth   = 1,
         .Format  = TexFormat,
         .Type    = ERHIImageType::Image2D,
         .Usages  = ERHIImageUsage::ShaderResource | ERHIImageUsage::TransferDst,
         .ViewType = ERHIImageViewType::Image2D,
      });

      RHI->UploadTexture(BaseColorTex, Img.GetData(), Img.GetSizeInBytes());

      FRHISamplerID Sampler = RHI->CreateSampler(FRHISamplerCreateInfo{
         .Type        = ERHISamplerType::Linear,
         .AddressMode = ERHISamplerAddressMode::Repeat,
      });

      FRHIDescriptorSetID DescSet = RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
         .Bindings = std::move(DSBindings),
      });

      // Ensure texture is in ShaderReadOnly before descriptor write (UploadTexture already transitions, but explicit sync)
      RHI->TransitionTexture(BaseColorTex, ERHIImageLayout::ShaderReadOnly, ERHIImageLayout::ShaderReadOnly);

      // Write descriptors by resource type and name
      for (const auto& [_, Res] : MergedResources)
      {
         const UInt32 Binding = Res.Binding;
         if (Res.Type == ERHIDescriptorType::SampledImage && Res.Name == "BaseColor")
         { RHI->WriteDescriptorSampledImage(DescSet, Binding, BaseColorTex); }
         else if (Res.Type == ERHIDescriptorType::Sampler && Res.Name == "BaseColorSampler")
         { RHI->WriteDescriptorSampler(DescSet, Binding, Sampler); }
         else if (Res.Type == ERHIDescriptorType::CombinedImageSampler && Res.Name == "BaseColor")
         { RHI->WriteDescriptorCombinedImageSampler(DescSet, Binding, BaseColorTex, Sampler); }
      }

      // Create render pass (pipeline state)
      FRHIRenderPassCreateInfo RPInfo
      {
         .VertexShader   = VertShader.GetHandle(),
         .FragmentShader = FragShader.GetHandle(),
         .Desc           = { .ColorFormat = ParseFormat(FormatStr) },
      };
      FRHIRenderPassID RenderPassID = RHI->CreateRenderPass(std::move(RPInfo));

      auto Mat = MakeShared<FMaterial>(
         std::move(RenderPassID),
         std::move(DescSet),
         std::move(Sampler),
         std::move(BaseColorTex),
         ParseSurfaceType(SurfStr));

      LOG_INFO("LoadMaterial: {} loaded successfully.", I_MaterialFile);
      return Mat;
   }

   // --- FGraphics::Render ---

   void FGraphics::
   Render()
   {
      auto Win = WindowWeak.Lock();
      if (!Win)
      { LOG_ERROR("Render: no window (windowless mode)."); return; }
      if (Win->GetWidth() == 0 || Win->GetHeight() == 0)
      { return; }  // Minimized

      RenderContext.ViewportWidth  = Win->GetWidth();
      RenderContext.ViewportHeight = Win->GetHeight();

      auto SwapChainTex = RHI->AcquireSwapChainTexture();
      auto CommandList  = RHI->CreateCommandList();

      FRHIRenderPassAttachments SwapChainAttachments;
      SwapChainAttachments.ColorTargets.PushBack(FRHIColorAttachmentDesc{
         .Texture    = SwapChainTex,
         .LoadOp     = ERHIAttachmentLoadOp::Clear,
         .StoreOp    = ERHIAttachmentStoreOp::Store,
      });

      CommandList.TransitionTexture({SwapChainTex, ERHIImageLayout::Undefined, ERHIImageLayout::ColorAttachment});

      for (auto& R : Renderers)
      {
         if (R) { R->Render(RenderContext, CommandList, SwapChainAttachments); }
      }

      CommandList.TransitionTexture({SwapChainTex, ERHIImageLayout::ColorAttachment, ERHIImageLayout::Present});
      RHI->Submit(std::move(CommandList));
   }

   // --- FGraphics::Present ---

   void FGraphics::
   Present()
   {
      auto Win = WindowWeak.Lock();
      if (!Win)
      { LOG_ERROR("Present: no window (windowless mode)."); return; }
      if (Win->GetWidth() == 0 || Win->GetHeight() == 0)
      { return; }  // Minimized
      RHI->Present();
   }

   // --- Helpers ---

   ESurfaceType FGraphics::
   ParseSurfaceType(const FString& I_Str)
   {
      if (I_Str == "Masked")      { return ESurfaceType::Masked; }
      if (I_Str == "Transparent") { return ESurfaceType::Transparent; }
      return ESurfaceType::Opaque;
   }

   ERHIFormat FGraphics::
   ParseFormat(const FString& I_Str)
   {
      if (I_Str == "R8G8B8_sRGB")         { return ERHIFormat::R8G8B8_sRGB; }
      if (I_Str == "R8G8B8_UNorm")        { return ERHIFormat::R8G8B8_UNorm; }
      if (I_Str == "B8G8R8_sRGB")         { return ERHIFormat::B8G8R8_sRGB; }
      if (I_Str == "B8G8R8_UNorm")        { return ERHIFormat::B8G8R8_UNorm; }
      if (I_Str == "R8G8B8A8_sRGB")       { return ERHIFormat::R8G8B8A8_sRGB; }
      if (I_Str == "R8G8B8A8_UNorm")      { return ERHIFormat::R8G8B8A8_UNorm; }
      if (I_Str == "B8G8R8A8_sRGB")       { return ERHIFormat::B8G8R8A8_sRGB; }
      if (I_Str == "B8G8R8A8_UNorm")      { return ERHIFormat::B8G8R8A8_UNorm; }
      if (I_Str == "R16G16B16A16_Float")  { return ERHIFormat::R16G16B16A16_Float; }
      if (I_Str == "R32G32_Float")        { return ERHIFormat::R32G32_Float; }
      if (I_Str == "R32G32B32_Float")     { return ERHIFormat::R32G32B32_Float; }
      if (I_Str == "R32G32B32A32_Float")  { return ERHIFormat::R32G32B32A32_Float; }
      return ERHIFormat::B8G8R8A8_sRGB;
   }

   ERHIFormat FGraphics::
   PixelFormatToRHIFormat(EPixelFormat I_Fmt)
   {
      switch (I_Fmt)
      {
      case EPixelFormat::RGBA8_UNorm: return ERHIFormat::R8G8B8A8_UNorm;
      case EPixelFormat::BGRA8_UNorm: return ERHIFormat::B8G8R8A8_UNorm;
      case EPixelFormat::RGBA16_Float: return ERHIFormat::R16G16B16A16_Float;
      default:
         LOG_WARN("PixelFormatToRHIFormat: unsupported format, defaulting to R8G8B8A8_UNorm.");
         return ERHIFormat::R8G8B8A8_UNorm;
      }
   }
}
