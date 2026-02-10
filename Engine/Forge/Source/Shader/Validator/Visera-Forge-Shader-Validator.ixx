module;
#include <Visera-Forge.hpp>
export module Visera.Forge.Shader.Validator;
#define VISERA_MODULE_NAME "Forge.Shader"
import Visera.Core.Types.Path;
import Visera.Core.Containers.Array;
import Visera.Core.Types.String;
import Visera.Core.OS.FileSystem;
import Visera.Runtime.AssetHub.Shader;
import Visera.Runtime.RHI.Common;
import Visera.Core.Log;

export namespace Visera::Forge
{
    /** Shader enum <-> string conversion (Forge only, avoids cross-DLL). */
    [[nodiscard]] inline const char* ShaderStageToString(ERHIShaderStage E)
    {
        switch (E)
        {
        case ERHIShaderStage::Vertex: return "Vertex";
        case ERHIShaderStage::Fragment: return "Fragment";
        case ERHIShaderStage::Compute: return "Compute";
        case ERHIShaderStage::All: return "All";
        default: return "Undefined";
        }
    }
    [[nodiscard]] inline const char* ShaderTypeToString(ERHIDescriptorType E)
    {
        switch (E)
        {
        case ERHIDescriptorType::CombinedImageSampler: return "CombinedImageSampler";
        case ERHIDescriptorType::SampledImage:         return "SampledImage";
        case ERHIDescriptorType::Sampler:              return "Sampler";
        case ERHIDescriptorType::StorageImage:         return "StorageImage";
        case ERHIDescriptorType::UniformBuffer:        return "UniformBuffer";
        case ERHIDescriptorType::StorageBuffer:        return "StorageBuffer";
        default: return "Undefined";
        }
    }
    [[nodiscard]] inline const char* ShaderAccessToString(ERHIResourceAccess E)
    { switch (E) { case ERHIResourceAccess::Write: return "Write"; case ERHIResourceAccess::ReadWrite: return "ReadWrite"; default: return "Read"; } }
    [[nodiscard]] inline const char* ShaderResourceStageToString(ERHIShaderStage E)
    { return ShaderStageToString(E); }
    [[nodiscard]] inline ERHIShaderStage StageFromString(FStringView S)
    {
        if (S == "Vertex") return ERHIShaderStage::Vertex;
        if (S == "Fragment") return ERHIShaderStage::Fragment;
        if (S == "Compute") return ERHIShaderStage::Compute;
        if (S == "All") return ERHIShaderStage::All;
        return ERHIShaderStage::Undefined;
    }
    [[nodiscard]] inline ERHIDescriptorType TypeFromString(FStringView S)
    {
        if (S == "Texture2D" || S == "TextureCube" || S == "Texture3D" || S == "Texture1D") return ERHIDescriptorType::SampledImage;
        if (S == "SamplerState") return ERHIDescriptorType::Sampler;
        if (S == "ConstantBuffer" || S == "TextureBuffer") return ERHIDescriptorType::UniformBuffer;
        if (S == "StructuredBuffer" || S == "ByteAddressBuffer") return ERHIDescriptorType::StorageBuffer;
        if (S == "StorageImage" || S == "RWTexture2D" || S == "RWTexture3D" || S == "RWTextureCube") return ERHIDescriptorType::StorageImage;
        if (S == "CombinedImageSampler") return ERHIDescriptorType::CombinedImageSampler;
        return ERHIDescriptorType::SampledImage;
    }
    [[nodiscard]] inline ERHIResourceAccess AccessFromString(FStringView S)
    { if (S == "Write") return ERHIResourceAccess::Write; if (S == "ReadWrite") return ERHIResourceAccess::ReadWrite; return ERHIResourceAccess::Read; }
    [[nodiscard]] inline UInt8 ResourceStageFromString(FStringView S)
    { if (S == "Vertex") return 1; if (S == "Fragment") return 2; if (S == "Compute") return 3; return 0; }
    [[nodiscard]] inline ERHIShaderStage ResourceStageFromU8(UInt8 E)
    { if (E == 1) return ERHIShaderStage::Vertex; if (E == 2) return ERHIShaderStage::Fragment; if (E == 3) return ERHIShaderStage::Compute; return ERHIShaderStage::All; }
    [[nodiscard]] inline UInt8 ResourceStageToU8(ERHIShaderStage E)
    { switch (E) { case ERHIShaderStage::Vertex: return 1; case ERHIShaderStage::Fragment: return 2; case ERHIShaderStage::Compute: return 3; default: return 0; } }

    struct FShaderValidationResult
    {
        Bool Ok = True;
        TArray<FString> Errors;

        void AddError(FStringView I_Message) { Ok = False; Errors.PushBack(FString(I_Message)); }
    };

    /** Returns True if I_Str is non-empty and first character is A–Z (PascalCase). */
    [[nodiscard]] inline Bool IsPascalCase(FStringView I_Str) noexcept
    {
        if (I_Str.IsEmpty()) { return False; }
        const char C = I_Str[0];
        return C >= 'A' && C <= 'Z';
    }

    /**
     * Validate a binary .vshader asset: header/chunks, PascalCase for all Names in Reflection chunk.
     * Path must be to the .vshader file (single binary with Header + ChunkTable + SPIRV + Reflection).
     * When I_OutputMeta is true (default), writes .vshader.meta with reflection as JSON for development inspection.
     */
    [[nodiscard]] inline FShaderValidationResult Validate(const FPath& I_VshaderPath, Bool I_OutputMeta = True)
    {
        FShaderValidationResult Result;

        if (!FFileSystem::Exists(I_VshaderPath))
        { Result.AddError("File does not exist."); return Result; }
        if (FFileSystem::IsDirectory(I_VshaderPath))
        { Result.AddError("Path is a directory, expected .vshader file."); return Result; }

        TArray<FByte> SPIRVChunk, ReflectionChunk;
        UInt32 Version = 0;
        if (!ReadShaderChunks(I_VshaderPath, Version, SPIRVChunk, ReflectionChunk))
        { Result.AddError("Invalid .vshader binary (bad header or chunk table)."); return Result; }
        if (SPIRVChunk.IsEmpty())
        { Result.AddError("SPIR-V chunk is missing or empty."); return Result; }

        FRHIShaderLayout Refl;
        if (ReflectionChunk.IsEmpty() || !DeserializeShaderReflection(Version, FStringView(reinterpret_cast<const char*>(ReflectionChunk.Data()), ReflectionChunk.GetSize()), Refl))
        { Result.AddError("Reflection chunk missing or failed to deserialize."); return Result; }

        for (const auto& EP : Refl.EntryPoints)
        {
            if (EP.Name.IsEmpty())
            { Result.AddError("EntryPoints entry has empty Name."); }
            else if (!IsPascalCase(EP.Name))
            { Result.AddError("EntryPoints[\"" + EP.Name + "\"] must be PascalCase."); }
        }
        for (const auto& R : Refl.Resources)
        {
            if (R.Name.IsEmpty())
            { Result.AddError("Resources entry has empty Name."); }
            else if (!IsPascalCase(R.Name))
            { Result.AddError("Resources[\"" + R.Name + "\"] must be PascalCase."); }
        }

        if (I_OutputMeta)
        {
            // NOTE: Visera::FJSON is backed by nlohmann::json (std::map), which sorts object keys.
            // For human inspection, we want stable custom key order, so we emit JSON manually.
            auto EscapeJSONString = [](FStringView S) -> FString
            {
                FString Out;
                for (UInt64 i = 0; i < S.GetSize(); ++i)
                {
                    const char C = S[static_cast<FStringView::SizeType>(i)];
                    switch (C)
                    {
                    case '\\\\': Out.Append("\\\\"); Out.Append("\\\\"); break;
                    case '\"':  Out.Append("\\\\"); Out.Append("\""); break;
                    case '\n':  Out.Append("\\\\n"); break;
                    case '\r':  Out.Append("\\\\r"); break;
                    case '\t':  Out.Append("\\\\t"); break;
                    default:    Out.Append(C); break;
                    }
                }
                return Out;
            };

            auto Indent = [](Int32 N) -> FString { return FString(static_cast<UInt64>(N), ' '); };

            auto AppendStagesArray = [&](FString& IO, Int32 IndentSpaces, ERHIShaderStage Mask)
            {
                IO.Append(Indent(IndentSpaces)).Append("\"Stages\": [");
                Bool First = True;
                auto Add = [&](const char* S)
                {
                    if (!First) IO.Append(", ");
                    IO.Append("\"").Append(S).Append("\"");
                    First = False;
                };
                const UInt32 V = static_cast<UInt32>(Mask);
                if (V & static_cast<UInt32>(ERHIShaderStage::Vertex)) Add("Vertex");
                if (V & static_cast<UInt32>(ERHIShaderStage::Fragment)) Add("Fragment");
                if (V & static_cast<UInt32>(ERHIShaderStage::Compute)) Add("Compute");
                if (First) Add("All");
                IO.Append("]");
            };

            FString Meta;
            Meta.Append("{\n");
            Meta.Append(Indent(4)).Append("\"Reflection\": {\n");

            // EntryPoints
            Meta.Append(Indent(8)).Append("\"EntryPoints\": [\n");
            for (UInt64 i = 0; i < Refl.EntryPoints.GetSize(); ++i)
            {
                const auto& EP = Refl.EntryPoints[i];
                Meta.Append(Indent(12)).Append("{\n");
                Meta.Append(Indent(16)).Append("\"Name\": \"").Append(EscapeJSONString(EP.Name)).Append("\",\n");
                Meta.Append(Indent(16)).Append("\"Stage\": \"").Append(ShaderStageToString(EP.Stage)).Append("\"\n");
                Meta.Append(Indent(12)).Append("}");
                Meta.Append(i + 1 < Refl.EntryPoints.GetSize() ? ",\n" : "\n");
            }
            Meta.Append(Indent(8)).Append("],\n");

            // PushConstants
            Meta.Append(Indent(8)).Append("\"PushConstants\": [\n");
            for (UInt64 i = 0; i < Refl.PushConstants.GetSize(); ++i)
            {
                const auto& PC = Refl.PushConstants[i];
                Meta.Append(Indent(12)).Append("{\n");
                Meta.Append(Indent(16)).Append(FString::Format("\"Size\": {}", static_cast<UInt64>(PC.Size))).Append(",\n");
                AppendStagesArray(Meta, 16, PC.Stages);
                Meta.Append("\n");
                Meta.Append(Indent(12)).Append("}");
                Meta.Append(i + 1 < Refl.PushConstants.GetSize() ? ",\n" : "\n");
            }
            Meta.Append(Indent(8)).Append("],\n");

            // Resources
            Meta.Append(Indent(8)).Append("\"Resources\": [\n");
            for (UInt64 i = 0; i < Refl.Resources.GetSize(); ++i)
            {
                const auto& R = Refl.Resources[i];
                Meta.Append(Indent(12)).Append("{\n");
                Meta.Append(Indent(16)).Append(FString::Format("\"Set\": {},\n", static_cast<UInt64>(R.Set)));
                Meta.Append(Indent(16)).Append(FString::Format("\"Binding\": {},\n", static_cast<UInt64>(R.Binding)));
                Meta.Append(Indent(16)).Append(FString::Format("\"ArrayCount\": {},\n", static_cast<UInt64>(R.ArrayCount)));
                Meta.Append(Indent(16)).Append("\"Name\": \"").Append(EscapeJSONString(R.Name)).Append("\",\n");
                Meta.Append(Indent(16)).Append("\"Type\": \"").Append(ShaderTypeToString(R.Type)).Append("\",\n");
                AppendStagesArray(Meta, 16, R.Stages);
                if (R.Access != ERHIResourceAccess::Read)
                {
                    Meta.Append(",\n");
                    Meta.Append(Indent(16)).Append("\"Access\": \"").Append(ShaderAccessToString(R.Access)).Append("\"");
                }
                Meta.Append("\n");
                Meta.Append(Indent(12)).Append("}");
                Meta.Append(i + 1 < Refl.Resources.GetSize() ? ",\n" : "\n");
            }
            Meta.Append(Indent(8)).Append("]\n");

            Meta.Append(Indent(4)).Append("}\n");
            Meta.Append("}\n");

            FString MetaFileName(*I_VshaderPath.GetFileName());
            MetaFileName += ".meta";
            const FPath MetaPath = *I_VshaderPath.GetParent() / FPath(MetaFileName);
            if (auto Stream = FFileSystem::OpenOStream(MetaPath); Stream)
            { *Stream << Meta.GetNative(); }
        }

        return Result;
    }
}
