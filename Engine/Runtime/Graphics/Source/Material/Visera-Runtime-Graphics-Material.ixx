module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Material;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Core.Types.Pointer;
	   import Visera.Core.Types.JSON;
	   import Visera.Core.Containers.Array;
	   import Visera.Core.Containers.Map;
	   import Visera.Core.Types.String;
	   import Visera.Core.Types.Path;
	   import Visera.Core.Image;
	   import Visera.Core.Log;
	   import Visera.Runtime.RHI;
	   import Visera.Runtime.AssetHub;

export namespace Visera
{
	enum class ESurfaceType : UInt8
	{
		Opaque,
		Masked,
		Transparent,
	};

	class VISERA_RUNTIME_API FMaterial
	{
	public:
		/** Paths in the JSON (Shader.Vert, Shader.Frag, Textures.BaseColor) are resolved relative to the material file's directory (e.g. "../Shader/X" from Material/Test.vmaterial -> Material/../Shader/X). */
		[[nodiscard]] static TSharedPtr<FMaterial>
		Create(const FJSON& I_Description, FAssetHub* I_AssetHub, FRHI* I_RHI, const FPath& I_MaterialFile);

		[[nodiscard]] const FRHIShaderID&
		GetVertexShader() const noexcept { return VertexShader; }
		[[nodiscard]] const FRHIShaderID&
		GetFragmentShader() const noexcept { return FragmentShader; }
		[[nodiscard]] const FRHIDescriptorSetID&
		GetDescriptorSet() const noexcept { return DescriptorSet; }
		[[nodiscard]] ESurfaceType
		GetSurface() const noexcept { return Surface; }
		[[nodiscard]] Bool
		IsValid() const noexcept { return bValid; }

		FMaterial(FRHIShaderID        I_VertexShader,
		          FRHIShaderID        I_FragmentShader,
		          FRHIDescriptorSetID I_DescriptorSet,
		          FRHISamplerID       I_Sampler,
		          FRHITextureID       I_BaseColor,
		          ESurfaceType        I_Surface)
			: VertexShader   (std::move(I_VertexShader))
			, FragmentShader (std::move(I_FragmentShader))
			, DescriptorSet  (std::move(I_DescriptorSet))
			, Sampler        (std::move(I_Sampler))
			, BaseColor      (std::move(I_BaseColor))
			, Surface        (I_Surface)
		{}

	private:
		FRHIShaderID        VertexShader;
		FRHIShaderID        FragmentShader;
		FRHIDescriptorSetID DescriptorSet;
		FRHISamplerID       Sampler;
		FRHITextureID       BaseColor;
		ESurfaceType        Surface {ESurfaceType::Opaque};
		Bool                bValid  {True};

	public:
		~FMaterial() = default;
		FMaterial(const FMaterial&) = default;
		FMaterial& operator=(const FMaterial&) = default;
		FMaterial(FMaterial&&) = default;
		FMaterial& operator=(FMaterial&&) = default;

	private:
		static ESurfaceType
		ParseSurfaceType(const FString& I_Str)
		{
			if (I_Str == "Masked")      { return ESurfaceType::Masked; }
			if (I_Str == "Transparent") { return ESurfaceType::Transparent; }
			return ESurfaceType::Opaque;
		}

		static ERHIFormat
		PixelFormatToRHIFormat(EPixelFormat I_Fmt)
		{
			switch (I_Fmt)
			{
			case EPixelFormat::RGBA8_UNorm:  return ERHIFormat::R8G8B8A8_UNorm;
			case EPixelFormat::BGRA8_UNorm:  return ERHIFormat::B8G8R8A8_UNorm;
			case EPixelFormat::RGBA16_Float: return ERHIFormat::R16G16B16A16_Float;
			default:
				LOG_WARN("PixelFormatToRHIFormat: unsupported format, defaulting to R8G8B8A8_UNorm.");
				return ERHIFormat::R8G8B8A8_UNorm;
			}
		}
	};

	inline FPath ResolveRelativeToMaterialDir(const FPath& I_MaterialFile, const FString& I_RelativePath)
	{
		auto Base = I_MaterialFile.GetParent();
		if (!Base.HasValue()) { return FPath{I_RelativePath}; }
		return FPath::Normalized(FPath::Merge(Base.GetValue(), FPath{I_RelativePath}));
	}

	TSharedPtr<FMaterial> FMaterial::
	Create(const FJSON& I_Description, FAssetHub* I_AssetHub, FRHI* I_RHI, const FPath& I_MaterialFile)
	{
		if (!I_AssetHub || !I_RHI)
		{ LOG_ERROR("FMaterial::Create: AssetHub or RHI is null."); return nullptr; }

		const FString VertPath = I_Description.GetString(TJSONRoute<"Shader.Vert">());
		const FString FragPath = I_Description.GetString(TJSONRoute<"Shader.Frag">());
		FString SurfStr = "Opaque";
		if (auto opt = I_Description.TryGetString(TJSONRoute<"Surface.Type">()); opt.HasValue())
			SurfStr = std::move(opt.GetValue());
		else if (auto opt = I_Description.TryGetString("Surface"); opt.HasValue())
			SurfStr = std::move(opt.GetValue());
		const FString BaseColorPath = I_Description.GetString(TJSONRoute<"Textures.BaseColor">());

		if (VertPath.IsEmpty() || FragPath.IsEmpty() || BaseColorPath.IsEmpty())
		{ LOG_ERROR("FMaterial::Create: missing required fields (Shader.Vert, Shader.Frag, Textures.BaseColor)."); return nullptr; }

		const FPath VertResolved  = ResolveRelativeToMaterialDir(I_MaterialFile, VertPath);
		const FPath FragResolved = ResolveRelativeToMaterialDir(I_MaterialFile, FragPath);
		const FPath BaseColorResolved = ResolveRelativeToMaterialDir(I_MaterialFile, BaseColorPath);

		auto VertAsset = I_AssetHub->LoadShader(VertResolved);
		auto FragAsset = I_AssetHub->LoadShader(FragResolved);
		if (!VertAsset || !FragAsset)
		{ LOG_ERROR("FMaterial::Create: failed to load shaders ({}, {}).", VertResolved, FragResolved); return nullptr; }

		auto ImageAsset = I_AssetHub->LoadImage(BaseColorResolved);
		if (!ImageAsset)
		{ LOG_ERROR("FMaterial::Create: failed to load base color texture {}.", BaseColorResolved); return nullptr; }

		FRHIShaderID VertShader = I_RHI->CreateShader(FRHIShaderCreateInfo{
			.SPIRV      = VertAsset->GetSPIRV(),
			.Reflection = VertAsset->GetReflection(),
		});
		FRHIShaderID FragShader = I_RHI->CreateShader(FRHIShaderCreateInfo{
			.SPIRV      = FragAsset->GetSPIRV(),
			.Reflection = FragAsset->GetReflection(),
		});

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

		const auto& Img = ImageAsset->GetImage();
		ERHIFormat TexFormat = PixelFormatToRHIFormat(Img.GetPixelFormat());
		FRHITextureID BaseColorTex = I_RHI->CreateTexture(FRHITextureCreateInfo{
			.Width    = Img.GetWidth(),
			.Height   = Img.GetHeight(),
			.Depth    = 1,
			.Format   = TexFormat,
			.Type     = ERHIImageType::Image2D,
			.Usages   = ERHIImageUsage::ShaderResource | ERHIImageUsage::TransferDst,
			.ViewType = ERHIImageViewType::Image2D,
		});

		I_RHI->UploadTexture(BaseColorTex, Img.GetData(), Img.GetSizeInBytes());

		FRHISamplerID Sampler = I_RHI->CreateSampler(FRHISamplerCreateInfo{
			.Type        = ERHISamplerType::Linear,
			.AddressMode = ERHISamplerAddressMode::Repeat,
		});

		FRHIDescriptorSetID DescSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
			.Bindings = std::move(DSBindings),
		});

		I_RHI->TransitionTexture(BaseColorTex, ERHIImageLayout::ShaderReadOnly, ERHIImageLayout::ShaderReadOnly);

		for (const auto& [_, Res] : MergedResources)
		{
			const UInt32 Binding = Res.Binding;
			if (Res.Type == ERHIDescriptorType::SampledImage && Res.Name == "BaseColor")
			{ I_RHI->WriteDescriptorSampledImage(DescSet, Binding, BaseColorTex); }
			else if (Res.Type == ERHIDescriptorType::Sampler && Res.Name == "BaseColorSampler")
			{ I_RHI->WriteDescriptorSampler(DescSet, Binding, Sampler); }
			else if (Res.Type == ERHIDescriptorType::CombinedImageSampler && Res.Name == "BaseColor")
			{ I_RHI->WriteDescriptorCombinedImageSampler(DescSet, Binding, BaseColorTex, Sampler); }
		}

		return MakeShared<FMaterial>(
			std::move(VertShader),
			std::move(FragShader),
			std::move(DescSet),
			std::move(Sampler),
			std::move(BaseColorTex),
			ParseSurfaceType(SurfStr));
	}
}
