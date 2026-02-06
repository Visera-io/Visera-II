module;
#include <Visera-AssetHub.hpp>
export module Visera.AssetHub.Shader;
#define VISERA_MODULE_NAME "AssetHub.Shader"
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Core.OS.FileSystem;
import Visera.RHI.Common;

export namespace Visera
{
    /** Runtime reflection from a .vshader file. Uses RHI enums for efficiency (no string parsing in render path). */
    struct FShaderReflection
    {
        struct FEntryPoint
        {
            FString           Name;
            ERHIShaderStages  Stage = ERHIShaderStages::Vertex;
        };
        struct FResource
        {
            FString            Name;
            ERHIResourceType   Type   = ERHIResourceType::Texture;
            UInt32             Binding;
            UInt32             Set    = 0;
            ERHIResourceAccess Access = ERHIResourceAccess::Read;
            ERHIShaderStages   Stage  = ERHIShaderStages::All; // which stage(s) use this resource
        };
        TArray<FEntryPoint> EntryPoints;
        TArray<FResource>   Resources;
    };

    /** .vshader binary format: constants and encode/decode helpers. Single source of truth for Validator and runtime read. */
    class VISERA_ASSETHUB_API FShader
    {
    public:
        static constexpr UInt32 ShaderMagic = 0x52485356u; // "VSHR" little-endian
        static constexpr UInt32 ShaderVersion = 1u;
        static constexpr UInt32 ShaderChunkTypeSPIRV = 0u;
        static constexpr UInt32 ShaderChunkTypeReflection = 1u;
        static constexpr UInt32 HeaderSize = 4u + 4u + 4u;
        static constexpr UInt32 ChunkEntrySize = 4u + 4u + 4u;
        static constexpr UInt32 ShaderChunkCount = 2u;
        static constexpr UInt32 ChunkTableSize = ShaderChunkCount * ChunkEntrySize;
        static constexpr UInt32 ShaderFileHeaderTotal = HeaderSize + ChunkTableSize;

        static UInt32 ReadU32(const FByte*& I_Ptr)
        {
            UInt32 v = static_cast<UInt32>(I_Ptr[0]) | (static_cast<UInt32>(I_Ptr[1]) << 8) | (static_cast<UInt32>(I_Ptr[2]) << 16) | (static_cast<UInt32>(I_Ptr[3]) << 24);
            I_Ptr += 4;
            return v;
        }
        static UInt8 ReadU8(const FByte*& I_Ptr) { return static_cast<UInt8>(*I_Ptr++); }

        /** Binary layout: resource stage 0=All, 1=Vertex, 2=Fragment, 3=Compute. Used by Deserialize only. */
        static ERHIShaderStages ResourceStageFromU8(UInt8 E) { if (E == 1) return ERHIShaderStages::Vertex; if (E == 2) return ERHIShaderStages::Fragment; if (E == 3) return ERHIShaderStages::Compute; return ERHIShaderStages::All; }

        static void WriteU32(TArray<FByte>& O_Out, UInt32 V)
        {
            O_Out.PushBack(static_cast<FByte>(V & 0xff));
            O_Out.PushBack(static_cast<FByte>((V >> 8) & 0xff));
            O_Out.PushBack(static_cast<FByte>((V >> 16) & 0xff));
            O_Out.PushBack(static_cast<FByte>((V >> 24) & 0xff));
        }
        static void WriteU8(TArray<FByte>& O_Out, UInt8 V) { O_Out.PushBack(V); }
        static void WriteBytes(TArray<FByte>& O_Out, const char* I_Ptr, UInt32 I_Len)
        {
            for (UInt32 i = 0; i < I_Len; ++i) O_Out.PushBack(static_cast<FByte>(I_Ptr[i]));
        }

        /** Loaded/compiled shader: SPIR-V bytes + reflection. */
        TArray<FByte> SPIRV;
        FShaderReflection Reflection;

        [[nodiscard]] const TArray<FByte>& GetSPIRV() const { return SPIRV; }
        [[nodiscard]] const FShaderReflection& GetReflection() const { return Reflection; }
    };

    /** Read SPIR-V and Reflection chunks from a .vshader binary file. */
    [[nodiscard]] inline Bool
    ReadShaderChunks(const FPath& I_Path, TArray<FByte>& O_SPIRV, TArray<FByte>& O_ReflectionChunk)
    {
        O_SPIRV.Clear();
        O_ReflectionChunk.Clear();
        if (auto File = FFileSystem::OpenFile(I_Path, EFileMode::Read | EFileMode::Binary); File && File->IsOpen())
        {
            TArray<FByte> All = File->ReadAll();
            if (All.GetSize() < FShader::ShaderFileHeaderTotal) return False;
            const FByte* p = All.Data();
            if (FShader::ReadU32(p) != FShader::ShaderMagic) return False;
            if (FShader::ReadU32(p) != FShader::ShaderVersion) return False;
            const UInt32 ChunkCountRead = FShader::ReadU32(p);
            if (ChunkCountRead != FShader::ShaderChunkCount) return False;
            UInt32 SpirvOffset = 0, SpirvSize = 0, ReflOffset = 0, ReflSize = 0;
            for (UInt32 i = 0; i < ChunkCountRead; ++i)
            {
                const UInt32 Type = FShader::ReadU32(p);
                const UInt32 Offset = FShader::ReadU32(p);
                const UInt32 Size = FShader::ReadU32(p);
                if (Type == FShader::ShaderChunkTypeSPIRV) { SpirvOffset = Offset; SpirvSize = Size; }
                else if (Type == FShader::ShaderChunkTypeReflection) { ReflOffset = Offset; ReflSize = Size; }
            }
            if (SpirvOffset + SpirvSize > All.GetSize() || ReflOffset + ReflSize > All.GetSize()) return False;
            O_SPIRV.Resize(SpirvSize);
            for (UInt32 i = 0; i < SpirvSize; ++i) O_SPIRV[i] = All[SpirvOffset + i];
            O_ReflectionChunk.Resize(ReflSize);
            for (UInt32 i = 0; i < ReflSize; ++i) O_ReflectionChunk[i] = All[ReflOffset + i];
            return True;
        }
        return False;
    }

    /** Deserialize reflection chunk bytes into O_Reflection. */
    [[nodiscard]] inline Bool
    DeserializeShaderReflection(FStringView I_ChunkBytes, FShaderReflection& O_Reflection)
    {
        if (I_ChunkBytes.GetSize() < 4) return False;
        const FByte* p = reinterpret_cast<const FByte*>(I_ChunkBytes.Data());
        const FByte* end = p + I_ChunkBytes.GetSize();

        const UInt32 NumNames = FShader::ReadU32(p);
        TArray<FString> NameTable;
        NameTable.Reserve(NumNames);
        for (UInt32 i = 0; i < NumNames; ++i)
        {
            if (p + 4 > end) return False;
            const UInt32 Len = FShader::ReadU32(p);
            if (p + Len > end) return False;
            NameTable.PushBack(FString(FStringView(reinterpret_cast<const char*>(p), Len)));
            p += Len;
        }

        if (p + 4 > end) return False;
        const UInt32 NumEP = FShader::ReadU32(p);
        for (UInt32 i = 0; i < NumEP; ++i)
        {
            if (p + 8 > end) return False;
            const UInt32 NameIdx = FShader::ReadU32(p);
            const auto Stage = static_cast<ERHIShaderStages>(FShader::ReadU32(p));
            if (NameIdx >= NameTable.GetSize()) return False;
            FShaderReflection::FEntryPoint EP;
            EP.Name = NameTable[NameIdx];
            EP.Stage = Stage;
            O_Reflection.EntryPoints.PushBack(std::move(EP));
        }

        if (p + 4 > end) return False;
        const UInt32 NumRes = FShader::ReadU32(p);
        for (UInt32 i = 0; i < NumRes; ++i)
        {
            if (p + 15 > end) return False;
            const UInt32 NameIdx = FShader::ReadU32(p);
            const auto Type = static_cast<ERHIResourceType>(FShader::ReadU8(p));
            const UInt32 Binding = FShader::ReadU32(p);
            const UInt32 Set = FShader::ReadU32(p);
            const auto Access = static_cast<ERHIResourceAccess>(FShader::ReadU8(p));
            const ERHIShaderStages RStage = FShader::ResourceStageFromU8(FShader::ReadU8(p));
            if (NameIdx >= NameTable.GetSize()) return False;
            FShaderReflection::FResource R;
            R.Name = NameTable[NameIdx];
            R.Type = Type;
            R.Binding = Binding;
            R.Set = Set;
            R.Access = Access;
            R.Stage = RStage;
            O_Reflection.Resources.PushBack(std::move(R));
        }
        return True;
    }
}
