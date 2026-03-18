module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.Material;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Core.Types.Pointer;
       import Visera.Core.Types.JSON;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Types.Optional;
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
        /** Paths in the JSON (Shader, Textures, etc.) are resolved relative to the material file's directory. */
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
        [[nodiscard]] ERHICullMode
        GetCullMode() const noexcept { return CullMode; }
        [[nodiscard]] Bool
        GetZWrite() const noexcept { return bZWrite; }
        [[nodiscard]] Bool
        GetDepthTest() const noexcept { return bDepthTest; }
        [[nodiscard]] ERHICompareOp
        GetDepthCompareOp() const noexcept { return DepthCompareOp; }
        [[nodiscard]] Bool
        IsValid() const noexcept { return bValid; }

        FMaterial(FRHIShaderID         I_VertexShader,
                  FRHIShaderID         I_FragmentShader,
                  FRHIDescriptorSetID  I_DescriptorSet,
                  TArray<FRHISamplerID> I_OwnedSamplers,
                  TArray<FRHITextureID> I_OwnedTextures,
                  TArray<FRHIBufferID>  I_OwnedBuffers,
                  ESurfaceType         I_Surface,
                  ERHICullMode         I_CullMode,
                  Bool                 I_ZWrite,
                  Bool                 I_DepthTest,
                  ERHICompareOp        I_DepthCompareOp)
            : VertexShader   (std::move(I_VertexShader))
            , FragmentShader (std::move(I_FragmentShader))
            , DescriptorSet  (std::move(I_DescriptorSet))
            , OwnedSamplers  (std::move(I_OwnedSamplers))
            , OwnedTextures  (std::move(I_OwnedTextures))
            , OwnedBuffers   (std::move(I_OwnedBuffers))
            , Surface        (I_Surface)
            , CullMode       (I_CullMode)
            , bZWrite        (I_ZWrite)
            , bDepthTest     (I_DepthTest)
            , DepthCompareOp (I_DepthCompareOp)
        {}

    private:
        FRHIShaderID           VertexShader;
        FRHIShaderID           FragmentShader;
        FRHIDescriptorSetID    DescriptorSet;
        TArray<FRHISamplerID>  OwnedSamplers;
        TArray<FRHITextureID>  OwnedTextures;
        TArray<FRHIBufferID>   OwnedBuffers;
        ESurfaceType           Surface        {ESurfaceType::Opaque};
        ERHICullMode           CullMode       {ERHICullMode::Back};
        Bool                   bZWrite        {True};
        Bool                   bDepthTest     {False};
        ERHICompareOp          DepthCompareOp {ERHICompareOp::LessOrEqual};
        Bool                   bValid         {True};

    public:
        ~FMaterial() = default;
        // Move-only: RHI resource handles (ShaderIDs, TextureIDs, DescriptorSetID) have
        // unique ownership; copying would create aliased GPU resources.
        FMaterial(const FMaterial&) = delete;
        FMaterial& operator=(const FMaterial&) = delete;
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

        static ERHISamplerType
        ParseSamplerType(const FString& I_Str)
        {
            if (I_Str == "Nearest") { return ERHISamplerType::Nearest; }
            return ERHISamplerType::Linear;
        }

        static ERHICullMode
        ParseCullMode(const FString& I_Str)
        {
            if (I_Str == "None")       { return ERHICullMode::None; }
            if (I_Str == "Front")      { return ERHICullMode::Front; }
            if (I_Str == "Back")       { return ERHICullMode::Back; }
            if (I_Str == "TwoSided")   { return ERHICullMode::TwoSided; }
            return ERHICullMode::Back;
        }

        static ERHICompareOp
        ParseCompareOp(const FString& I_Str)
        {
            if (I_Str == "Never")          { return ERHICompareOp::Never; }
            if (I_Str == "Less")           { return ERHICompareOp::Less; }
            if (I_Str == "Equal")          { return ERHICompareOp::Equal; }
            if (I_Str == "LessOrEqual")    { return ERHICompareOp::LessOrEqual; }
            if (I_Str == "Greater")        { return ERHICompareOp::Greater; }
            if (I_Str == "NotEqual")       { return ERHICompareOp::NotEqual; }
            if (I_Str == "GreaterOrEqual") { return ERHICompareOp::GreaterOrEqual; }
            if (I_Str == "Always")         { return ERHICompareOp::Always; }
            return ERHICompareOp::LessOrEqual;
        }

        static ERHISamplerAddressMode
        ParseAddressMode(const FString& I_Str)
        {
            if (I_Str == "Repeat")            { return ERHISamplerAddressMode::Repeat; }
            if (I_Str == "MirroredRepeat")    { return ERHISamplerAddressMode::MirroredRepeat; }
            if (I_Str == "Clamp" || I_Str == "ClampToEdge") { return ERHISamplerAddressMode::ClampToEdge; }
            if (I_Str == "ClampToBorder")     { return ERHISamplerAddressMode::ClampToBorder; }
            if (I_Str == "MirrorClampToEdge") { return ERHISamplerAddressMode::MirrorClampToEdge; }
            return ERHISamplerAddressMode::Repeat;
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

        /** Layout type sizes (Visera naming; sizes match Slang semantics). Local constexpr, no Core dependency. */
        static constexpr UInt32 kLayoutSizeFloat     = 4u;
        static constexpr UInt32 kLayoutSizeVector2F  = 8u;
        static constexpr UInt32 kLayoutSizeVector3F  = 12u;
        static constexpr UInt32 kLayoutSizeVector4F  = 16u;
        static constexpr UInt32 kLayoutSizeMatrix3x3F = 36u;
        static constexpr UInt32 kLayoutSizeMatrix4x4F = 64u;

        /** Return byte size for Visera layout type name, or 0 if unknown. */
        static UInt32 GetLayoutTypeSize(FStringView I_TypeName)
        {
            if (I_TypeName == "Float")      return kLayoutSizeFloat;
            if (I_TypeName == "Vector2F")   return kLayoutSizeVector2F;
            if (I_TypeName == "Vector3F")   return kLayoutSizeVector3F;
            if (I_TypeName == "Vector4F")   return kLayoutSizeVector4F;
            if (I_TypeName == "Matrix3x3F") return kLayoutSizeMatrix3x3F;
            if (I_TypeName == "Matrix4x4F") return kLayoutSizeMatrix4x4F;
            return 0u;
        }

        /** Compute total byte size of a Layout array (array of { Name, Type }). Returns NullOpt if any type unknown. */
        static TOptional<UInt64> ComputeLayoutSizeBytes(const FJSON& I_Description, FStringView I_LayoutKey)
        {
            auto ArrOpt = I_Description.TryGetArray<FJSON>(FJSONRoute(I_LayoutKey));
            if (!ArrOpt.HasValue()) { return NullOpt; }
            UInt64 Total = 0;
            for (UInt32 i = 0; i < ArrOpt.GetValue().GetSize(); ++i)
            {
                const FJSON& Elem = ArrOpt.GetValue()[i];
                auto TypeOpt = Elem.TryGetString("Type");
                if (!TypeOpt.HasValue()) { return NullOpt; }
                const UInt32 Sz = GetLayoutTypeSize(TypeOpt.GetValue());
                if (Sz == 0u) { return NullOpt; }
                Total += Sz;
            }
            return TOptional<UInt64>(Total);
        }

        /** Same for a JSON array value (e.g. from Resources[i] already parsed). */
        static TOptional<UInt64> ComputeLayoutSizeBytesFromArray(const TArray<FJSON>& I_LayoutArray)
        {
            UInt64 Total = 0;
            for (const FJSON& Elem : I_LayoutArray)
            {
                auto TypeOpt = Elem.TryGetString("Type");
                if (!TypeOpt.HasValue()) { return NullOpt; }
                const UInt32 Sz = GetLayoutTypeSize(TypeOpt.GetValue());
                if (Sz == 0u) { return NullOpt; }
                Total += Sz;
            }
            return TOptional<UInt64>(Total);
        }

        /** Read JSON number array as float array (JSON has double; UBO/shader define the type). */
        static TArray<Float>
        TryGetFloatArray(const FJSON& I_Description, FStringView I_RouteKey)
        {
            auto Opt = I_Description.TryGetArray<double>(FJSONRoute(I_RouteKey));
            if (!Opt.HasValue()) { return {}; }
            TArray<Float> Out;
            for (double v : Opt.GetValue()) { Out.PushBack(static_cast<Float>(v)); }
            return Out;
        }
    };

    inline FPath ResolveRelativeToMaterialDir(const FPath& I_MaterialFile, const FString& I_RelativePath)
    {
        auto Base = I_MaterialFile.GetParent();
        if (!Base.HasValue()) { return FPath{I_RelativePath}; }
        return FPath::Normalized(FPath::Merge(Base.GetValue(), FPath{I_RelativePath}));
    }

    struct FBufferLayoutInfo
    {
        UInt64         SizeBytes = 0;
        TArray<Float>  Data;
    };

    struct FTextureResourceInfo
    {
        FString ImagePath;
        FString Type; // "Texture2D", "TextureCube", "Texture3D", "Texture1D" (Slang-aligned)
    };

    inline Bool IsTextureType(FStringView I_Type)
    {
        return I_Type == "Texture2D" || I_Type == "TextureCube" || I_Type == "Texture3D" || I_Type == "Texture1D";
    }

    TSharedPtr<FMaterial> FMaterial::
    Create(const FJSON& I_Description, FAssetHub* I_AssetHub, FRHI* I_RHI, const FPath& I_MaterialFile)
    {
        if (!I_AssetHub || !I_RHI)
        { LOG_ERROR("FMaterial::Create: AssetHub or RHI is null."); return nullptr; }

        const auto VersionOpt = I_Description.TryGetNumber<UInt32>("Version");
        if (VersionOpt.HasValue() && VersionOpt.GetValue() != 1u)
        { LOG_ERROR("FMaterial::Create: unsupported material Version (expected 1)."); return nullptr; }

        const FString VertPath = I_Description.GetString(TJSONRoute<"Shader.Vert">());
        const FString FragPath = I_Description.GetString(TJSONRoute<"Shader.Frag">());
        if (VertPath.IsEmpty() || FragPath.IsEmpty())
        { LOG_ERROR("FMaterial::Create: missing Shader.Vert or Shader.Frag."); return nullptr; }

        // --- Parse State (Surface, CullMode, ZWrite) ---
        FString SurfStr = "Opaque";
        if (auto opt = I_Description.TryGetString(TJSONRoute<"State.Surface">()); opt.HasValue())
            SurfStr = std::move(opt.GetValue());
        else if (auto opt = I_Description.TryGetString(TJSONRoute<"Surface.Type">()); opt.HasValue())
            SurfStr = std::move(opt.GetValue());
        else if (auto opt = I_Description.TryGetString("Surface"); opt.HasValue())
            SurfStr = std::move(opt.GetValue());
        FString CullStr = "Back";
        if (auto opt = I_Description.TryGetString(TJSONRoute<"State.CullMode">()); opt.HasValue())
            CullStr = std::move(opt.GetValue());
        Bool ZWrite = True;
        if (auto opt = I_Description.TryGetBool(TJSONRoute<"State.ZWrite">()); opt.HasValue())
            ZWrite = opt.GetValue();
        Bool DepthTest = False;
        if (auto opt = I_Description.TryGetBool(TJSONRoute<"State.DepthTest">()); opt.HasValue())
            DepthTest = opt.GetValue();
        // DepthWrite defaults to ZWrite for backward compatibility
        if (auto opt = I_Description.TryGetBool(TJSONRoute<"State.DepthWrite">()); opt.HasValue())
            ZWrite = opt.GetValue();
        FString DepthCompareStr = "LessOrEqual";
        if (auto opt = I_Description.TryGetString(TJSONRoute<"State.DepthCompare">()); opt.HasValue())
            DepthCompareStr = std::move(opt.GetValue());

        // --- Parse Textures: name -> image path (reflection defines which names exist) ---
        TMap<FString, FTextureResourceInfo> TextureNameToInfo;
        if (auto ObjOpt = I_Description.TryGetObject("Textures"); ObjOpt.HasValue())
        {
            const FJSON& TexturesObj = ObjOpt.GetValue();
            for (const auto& Item : TexturesObj.Items())
            {
                FStringView Key{ Item.key };
                FString Path = TexturesObj.GetString(Key);
                if (!Path.IsEmpty())
                    TextureNameToInfo.InsertOrAssign(FString(Key), FTextureResourceInfo{ std::move(Path), "Texture2D" });
            }
        }

        // --- Parse Samplers: name -> { Filter, AddressMode } ---
        TMap<FString, ERHISamplerType>        SamplerNameToFilter;
        TMap<FString, ERHISamplerAddressMode> SamplerNameToAddressMode;
        if (auto ObjOpt = I_Description.TryGetObject("Samplers"); ObjOpt.HasValue())
        {
            const FJSON& SamplersObj = ObjOpt.GetValue();
            for (const auto& Item : SamplersObj.Items())
            {
                FString SamplerName{ FStringView(Item.key) };
                FJSON SamplerVal = SamplersObj.GetObject(FStringView(Item.key));
                FString FilterStr = SamplerVal.GetString("Filter", "Linear");
                FString AddrStr = SamplerVal.GetString("AddressMode", "Repeat");
                SamplerNameToFilter.InsertOrAssign(SamplerName, ParseSamplerType(FilterStr));
                SamplerNameToAddressMode.InsertOrAssign(SamplerName, ParseAddressMode(AddrStr));
            }
        }

        // Parameters (optional default values by name) — parsed but not used for binding; reflection is source of truth for layout.
        (void)I_Description.TryGetObject("Parameters");
        TMap<FString, FBufferLayoutInfo> BufferNameToLayout; // UniformBuffer not in new schema; keep empty for now

        const FPath VertResolved  = ResolveRelativeToMaterialDir(I_MaterialFile, VertPath);
        const FPath FragResolved  = ResolveRelativeToMaterialDir(I_MaterialFile, FragPath);

        auto VertAsset = I_AssetHub->LoadShader(VertResolved);
        auto FragAsset = I_AssetHub->LoadShader(FragResolved);
        if (!VertAsset || !FragAsset)
        { LOG_ERROR("FMaterial::Create: failed to load shaders ({}, {}).", VertResolved, FragResolved); return nullptr; }

        const auto& VertRefl = VertAsset->GetReflection();
        const auto& FragRefl = FragAsset->GetReflection();
        // Only resources in the material descriptor set (Set 1) are included.
        // Other sets (FrameData=0, Lights=2, Instance=3) are engine-managed.
        static constexpr UInt32 kMaterialSet = 1;
        TMap<UInt64, FRHIShaderLayout::FResource> MergedResources;
        auto AddResource = [&MergedResources](const FRHIShaderLayout::FResource& Res)
        {
            if (Res.Set != kMaterialSet) { return; }
            const UInt64 Key = (static_cast<UInt64>(Res.Set) << 32) | Res.Binding;
            auto It = MergedResources.Find(Key);
            if (It != MergedResources.end())
            { It->second.Stages = static_cast<ERHIShaderStage>(static_cast<UInt32>(It->second.Stages) | static_cast<UInt32>(Res.Stages)); }
            else
            { MergedResources.Insert(Key, Res); }
        };
        for (const auto& Res : VertRefl.Resources) { AddResource(Res); }
        for (const auto& Res : FragRefl.Resources) { AddResource(Res); }

        // Require every SampledImage/CombinedImageSampler to have a matching texture resource by name (Type in Resources: Texture2D, TextureCube, etc., aligned with Slang)
        for (const auto& [_, Res] : MergedResources)
        {
            if (Res.Type == ERHIDescriptorType::SampledImage || Res.Type == ERHIDescriptorType::CombinedImageSampler)
            {
                if (!TextureNameToInfo.Contains(Res.Name))
                { LOG_ERROR("FMaterial::Create: shader texture '{}' has no entry in Textures.", Res.Name); return nullptr; }
            }
        }
        // Require every Sampler binding to have a matching Sampler resource by name
        for (const auto& [_, Res] : MergedResources)
        {
            if (Res.Type == ERHIDescriptorType::Sampler)
            {
                if (!SamplerNameToFilter.Contains(Res.Name))
                { LOG_ERROR("FMaterial::Create: shader Sampler '{}' has no entry in Samplers.", Res.Name); return nullptr; }
            }
        }
        // For CombinedImageSampler: texture by name; sampler by same name or by name + "Sampler"
        for (const auto& [_, Res] : MergedResources)
        {
            if (Res.Type == ERHIDescriptorType::CombinedImageSampler)
            {
                if (!TextureNameToInfo.Contains(Res.Name))
                    continue; // already validated above
                const FString SamplerKey = FString(Res.Name).Append("Sampler");
                if (!SamplerNameToFilter.Contains(Res.Name) && !SamplerNameToFilter.Contains(SamplerKey))
                { LOG_ERROR("FMaterial::Create: CombinedImageSampler '{}' needs an entry in Samplers named '{}' or '{}'.", Res.Name, Res.Name, SamplerKey); return nullptr; }
            }
        }
        // Require every UniformBuffer to have a UniformBuffer resource by name
        for (const auto& [_, Res] : MergedResources)
        {
            if (Res.Type == ERHIDescriptorType::UniformBuffer)
            {
                if (!BufferNameToLayout.Contains(Res.Name))
                { LOG_ERROR("FMaterial::Create: shader UniformBuffer '{}' has no matching entry (not supported in current schema).", Res.Name); return nullptr; }
            }
        }

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

        // Load images and create textures for each texture resource (only Texture2D supported for Image binding)
        TMap<FString, FRHITextureID> NameToTexture;
        for (const auto& [Name, TexInfo] : TextureNameToInfo)
        {
            if (TexInfo.Type != "Texture2D")
            { LOG_ERROR("FMaterial::Create: texture '{}' has Type '{}'; only Texture2D is supported for Image binding.", Name, TexInfo.Type); return nullptr; }
            const FPath Resolved = ResolveRelativeToMaterialDir(I_MaterialFile, TexInfo.ImagePath);
            auto ImageAsset = I_AssetHub->LoadImage(Resolved);
            if (!ImageAsset)
            { LOG_ERROR("FMaterial::Create: failed to load texture '{}' at {}.", Name, Resolved); return nullptr; }
            const auto& Img = ImageAsset->GetImage();
            ERHIFormat TexFormat = PixelFormatToRHIFormat(Img.GetPixelFormat());
            FRHITextureID Tex = I_RHI->CreateTexture(FRHITextureCreateInfo{
                .Width    = Img.GetWidth(),
                .Height   = Img.GetHeight(),
                .Depth    = 1,
                .Format   = TexFormat,
                .Type     = ERHIImageType::Image2D,
                .Usages   = ERHIImageUsage::ShaderResource | ERHIImageUsage::TransferDst,
                .ViewType = ERHIImageViewType::Image2D,
            });
            I_RHI->UploadTexture(Tex, Img.GetData(), Img.GetSizeInBytes());
            I_RHI->TransitionTexture(Tex, ERHIImageLayout::ShaderReadOnly, ERHIImageLayout::ShaderReadOnly);
            NameToTexture.InsertOrAssign(Name, std::move(Tex));
        }

        // Create samplers from Samplers (Filter + AddressMode)
        TMap<FString, FRHISamplerID> NameToSampler;
        for (const auto& [Name, SamplerType] : SamplerNameToFilter)
        {
            auto ItAddr = SamplerNameToAddressMode.Find(Name);
            ERHISamplerAddressMode Addr = ItAddr != SamplerNameToAddressMode.end() ? ItAddr->second : ERHISamplerAddressMode::Repeat;
            NameToSampler.InsertOrAssign(Name, I_RHI->CreateSampler(FRHISamplerCreateInfo{
                .Type        = SamplerType,
                .AddressMode = Addr,
            }));
        }

        // Create UniformBuffers from Resources (Layout defines size; optional Data for initial upload)
        TMap<FString, FRHIBufferID> NameToBuffer;
        for (const auto& [_, Res] : MergedResources)
        {
            if (Res.Type != ERHIDescriptorType::UniformBuffer) continue;
            auto It = BufferNameToLayout.Find(Res.Name);
            if (It == BufferNameToLayout.end()) continue; // already validated above
            const FBufferLayoutInfo& Info = It->second;
            FRHIBufferID Buf = I_RHI->CreateBuffer(FRHIBufferCreateInfo{
                .Size   = Info.SizeBytes,
                .Usages = ERHIBufferUsage::UniformBuffer | ERHIBufferUsage::TransferDst,
            });
            if (!Info.Data.IsEmpty())
                I_RHI->UploadBuffer(Buf, reinterpret_cast<const FByte*>(Info.Data.Data()), Info.SizeBytes, 0);
            NameToBuffer.InsertOrAssign(Res.Name, std::move(Buf));
        }

        FRHIDescriptorSetID DescSet = I_RHI->CreateDescriptorSet(FRHIDescriptorSetCreateInfo{
            .Bindings = std::move(DSBindings),
        });

        for (const auto& [_, Res] : MergedResources)
        {
            const UInt32 Binding = Res.Binding;
            if (Res.Type == ERHIDescriptorType::SampledImage && NameToTexture.Contains(Res.Name))
            { I_RHI->WriteDescriptorSampledImage(DescSet, Binding, NameToTexture[Res.Name]); }
            else if (Res.Type == ERHIDescriptorType::Sampler && NameToSampler.Contains(Res.Name))
            { I_RHI->WriteDescriptorSampler(DescSet, Binding, NameToSampler[Res.Name]); }
            else if (Res.Type == ERHIDescriptorType::CombinedImageSampler && NameToTexture.Contains(Res.Name))
            {
                // Sampler: use resource name "BaseColorSampler" if present, else "BaseColor"
                FString SamplerKey = FString(Res.Name).Append("Sampler");
                FRHISamplerID SamplerID = NameToSampler.Contains(SamplerKey) ? NameToSampler[SamplerKey] : NameToSampler[Res.Name];
                I_RHI->WriteDescriptorCombinedImageSampler(DescSet, Binding, NameToTexture[Res.Name], SamplerID);
            }
            else if (Res.Type == ERHIDescriptorType::UniformBuffer && NameToBuffer.Contains(Res.Name))
            { I_RHI->WriteDescriptorUniformBuffer(DescSet, Binding, NameToBuffer[Res.Name]); }
        }

        TArray<FRHISamplerID> OwnedSamplers;
        for (auto& [_, Smp] : NameToSampler)
            OwnedSamplers.PushBack(std::move(Smp));
        TArray<FRHITextureID> OwnedTextures;
        for (auto& [_, Tex] : NameToTexture)
            OwnedTextures.PushBack(std::move(Tex));
        TArray<FRHIBufferID> OwnedBuffers;
        for (auto& [_, Buf] : NameToBuffer)
            OwnedBuffers.PushBack(std::move(Buf));

        FRHIShaderID VertShader = I_RHI->CreateShader(FRHIShaderCreateInfo{
            .SPIRV      = VertAsset->GetSPIRV(),
            .Reflection = VertAsset->GetReflection(),
        });
        FRHIShaderID FragShader = I_RHI->CreateShader(FRHIShaderCreateInfo{
            .SPIRV      = FragAsset->GetSPIRV(),
            .Reflection = FragAsset->GetReflection(),
        });

        return MakeShared<FMaterial>(
            std::move(VertShader),
            std::move(FragShader),
            std::move(DescSet),
            std::move(OwnedSamplers),
            std::move(OwnedTextures),
            std::move(OwnedBuffers),
            ParseSurfaceType(SurfStr),
            ParseCullMode(CullStr),
            ZWrite,
            DepthTest,
            ParseCompareOp(DepthCompareStr));
    }
}
