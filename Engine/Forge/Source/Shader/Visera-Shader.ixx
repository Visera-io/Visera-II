module;
#include <Visera-Forge.hpp>
export module Visera.Shader;
#define VISERA_MODULE_NAME "Shader"
import Visera.Core.Types.Array;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Types.JSON;
import Visera.Core.Types.Optional;
import Visera.Core.OS.FileSystem;
import Visera.Shader.Slang;
import Visera.Global.Log;
import Visera.Forge.Utils.Escaping;

export namespace Visera::Forge
{

    class VISERA_FORGE_API FShader
    {
    public:
        enum class ELanguage
        {
            Slang,
        };

        [[nodiscard]] static TOptional<FShader>
        Compile(const FPath& I_SourcePath, FStringView I_EntryPoint)
        {
            FShader Shader;
            if (!Shader.Compiler) { return NullOpt; }

            auto SPIRV = Shader.Compiler->Compile(I_SourcePath, I_EntryPoint);
            if (SPIRV.IsEmpty()) { return NullOpt; }

            auto Reflection = Shader.Compiler->ExtractReflection(I_SourcePath, I_EntryPoint);
            if (Reflection.EntryPoints.IsEmpty())
            { return NullOpt; }

            Shader.SPIRV = std::move(SPIRV);
            Shader.Reflection = std::move(Reflection);
            Shader.SourcePath = I_SourcePath;
            Shader.EntryPoint = FString(I_EntryPoint);

            return TOptional<FShader>(std::move(Shader));
        }

        [[nodiscard]] static TOptional<FShader>
        Load(const FPath& I_ShaderPath)
        {
            auto JSON = FJSON::Load(I_ShaderPath);
            if (!JSON.HasValue())
            { return NullOpt; }

            FShader Shader;
            auto& Root = JSON.GetValue();

            Shader.Version = static_cast<UInt32>(Root.GetNumber("Version", 1));
            Shader.Name = Root.GetString("Name", "");

            // Load SPIR-V
            auto SPIRVBase64 = Root.GetString("SPIRV", "");
            if (SPIRVBase64.IsEmpty())
            { return NullOpt; }

            auto Decoded = Base64Decode(SPIRVBase64);
            if (!Decoded.HasValue())
            { return NullOpt; }

            Shader.SPIRV = std::move(Decoded.GetValue());

            // Load Reflection
            auto ReflectionObj = Root.GetObject("Reflection");
            if (ReflectionObj.IsNull())
            { return NullOpt; }

            // Load EntryPoints
            auto EntryPointsArray = ReflectionObj.GetArray<FJSON>("EntryPoints");
            for (const auto& EP : EntryPointsArray)
            {
                FShaderReflection::FEntryPoint EntryPoint;
                EntryPoint.Name = EP.GetString("Name", "");
                EntryPoint.Stage = EP.GetString("Stage", "");
                Shader.Reflection.EntryPoints.PushBack(std::move(EntryPoint));
            }

            // Load Resources
            auto ResourcesArray = ReflectionObj.GetArray<FJSON>("Resources");
            for (const auto& Res : ResourcesArray)
            {
                FShaderReflection::FResource Resource;
                Resource.Name = Res.GetString("Name", "");
                Resource.Type = Res.GetString("Type", "");
                Resource.Binding = static_cast<UInt32>(Res.GetNumber("Binding", 0));
                Resource.Set = static_cast<UInt32>(Res.GetNumber("Set", 0));
                Resource.Access = Res.GetString("Access", "Read");
                Resource.Stage = Res.GetString("Stage", "All");
                Shader.Reflection.Resources.PushBack(std::move(Resource));
            }

            return TOptional<FShader>(std::move(Shader));
        }

        [[nodiscard]] Bool
        Save(const FPath& I_OutputPath) const
        {
            FJSON JSON;

            JSON.Set("Version", static_cast<Double>(Version));
            if (!Name.IsEmpty())
            { JSON.Set("Name", Name); }

            // Encode SPIR-V to base64
            JSON.Set("SPIRV", Base64Encode(SPIRV));

            // Build Reflection
            FJSON ReflectionObj;
            {
                // EntryPoints
                TArray<FJSON> EntryPointsArray;
                EntryPointsArray.Reserve(Reflection.EntryPoints.GetSize());
                for (const auto& EP : Reflection.EntryPoints)
                {
                    FJSON EPObj;
                    EPObj.Set("Name", EP.Name);
                    EPObj.Set("Stage", EP.Stage);
                    EntryPointsArray.PushBack(std::move(EPObj));
                }
                ReflectionObj.Set("EntryPoints", EntryPointsArray);

                // Resources
                TArray<FJSON> ResourcesArray;
                ResourcesArray.Reserve(Reflection.Resources.GetSize());
                for (const auto& Res : Reflection.Resources)
                {
                    FJSON ResObj;
                    ResObj.Set("Name", Res.Name);
                    ResObj.Set("Type", Res.Type);
                    ResObj.Set("Binding", static_cast<Double>(Res.Binding));
                    if (Res.Set != 0)
                    { ResObj.Set("Set", static_cast<Double>(Res.Set)); }
                    if (Res.Access != "Read")
                    { ResObj.Set("Access", Res.Access); }
                    if (Res.Stage != "All")
                    { ResObj.Set("Stage", Res.Stage); }
                    ResourcesArray.PushBack(std::move(ResObj));
                }
                ReflectionObj.Set("Resources", ResourcesArray);
            }
            JSON.Set("Reflection", ReflectionObj);

            // Save to file
            if (auto Stream = FFileSystem::OpenOStream(I_OutputPath); Stream)
            {
                *Stream << JSON.Dump(True).GetNative();
                return True;
            }
            return False;
        }

        [[nodiscard]] const TArray<FByte>&
        GetSPIRV() const { return SPIRV; }

        [[nodiscard]] const FShaderReflection&
        GetReflection() const { return Reflection; }

        [[nodiscard]] const FPath&
        GetSourcePath() const { return SourcePath; }

        [[nodiscard]] const FString&
        GetEntryPoint() const { return EntryPoint; }

    private:
        FSlangCompiler* Compiler;
        UInt32 Version = 1;
        FString Name;
        FPath SourcePath;
        FString EntryPoint;
        TArray<FByte> SPIRV;
        FShaderReflection Reflection;

    public:
        FShader()
        {
            Compiler = new FSlangCompiler();
            if(!Compiler)
            { LOG_FATAL("Failed to initalize Slang compiler!"); }
        }
        ~FShader()
        {
            delete Compiler;
        }
    };
}
