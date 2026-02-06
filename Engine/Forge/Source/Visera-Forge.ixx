module;
#include <Visera-Forge.hpp>
export module Visera.Forge;
#define VISERA_MODULE_NAME "Forge"
import Visera.Global;
import Visera.Tasks;
import Visera.AssetHub;
import Visera.Platform;
import Visera.AssetHub.Shader;
import Visera.RHI.Common;
import Visera.Forge.Shader.Compiler;
import Visera.Forge.Shader.Validator;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Core.OS.FileSystem;
import Visera.Forge.Utils.Wildcard;

namespace Visera::Forge
{
    namespace
    {
        // Find files matching pattern recursively
        [[nodiscard]] TArray<FPath>
        FindMatchingFiles(const FPath& I_RootDir, FStringView I_Pattern)
        {
            TArray<FPath> Results;

            auto AllFiles = FFileSystem::EnumerateFiles(I_RootDir, True);
            for (const auto& FilePath : AllFiles)
            {
                const FString FileName = FilePath.GetFileName().GetUTF8Path();
                if (WildcardMatch(FileName, I_Pattern))
                {
                    Results.PushBack(FilePath);
                }
            }

            return Results;
        }

        // Compile shader from slang file -> FShader, then save
        [[nodiscard]] Bool
        CompileShader(const FPath& I_SourcePath)
        {
            LOG_INFO("Compiling shader: {}", I_SourcePath);

            const TArray<FStringView> EntryPoints = {"VertMain", "FragMain"};
            const FPath ShaderDirectory = I_SourcePath.GetParent();
            FShaderCompiler Compiler;

            UInt32 SuccessCount = 0;
            for (const auto& EntryPoint : EntryPoints)
            {
                auto SPIRV = Compiler.Compile(I_SourcePath, EntryPoint, ShaderDirectory);
                if (SPIRV.IsEmpty()) continue;
                auto Refl = Compiler.ExtractReflection(I_SourcePath, EntryPoint, ShaderDirectory);
                if (Refl.EntryPoints.IsEmpty()) continue;

                FShader Shader;
                Shader.SPIRV = std::move(SPIRV);
                Shader.Reflection.EntryPoints.Reserve(Refl.EntryPoints.GetSize());
                for (const auto& EP : Refl.EntryPoints)
                { Shader.Reflection.EntryPoints.PushBack({ EP.Name, StageFromString(EP.Stage) }); }
                Shader.Reflection.Resources.Reserve(Refl.Resources.GetSize());
                for (const auto& R : Refl.Resources)
                {
                    ERHIShaderStages StagesMask = ERHIShaderStages::Undefined;
                    for (const auto& S : R.Stages) { StagesMask |= StageFromString(S); }
                    if (StagesMask == ERHIShaderStages::Undefined) { StagesMask = ERHIShaderStages::All; }

                    Shader.Reflection.Resources.PushBack({
                        R.Name,
                        R.Set,
                        R.Binding,
                        R.ArrayCount,
                        TypeFromString(R.Type),
                        AccessFromString(R.Access),
                        StagesMask
                    });
                }
                Shader.Reflection.PushConstants.Reserve(Refl.PushConstants.GetSize());
                for (const auto& PC : Refl.PushConstants)
                {
                    ERHIShaderStages StagesMask = ERHIShaderStages::Undefined;
                    for (const auto& S : PC.Stages) { StagesMask |= StageFromString(S); }
                    if (StagesMask == ERHIShaderStages::Undefined) { StagesMask = ERHIShaderStages::All; }

                    Shader.Reflection.PushConstants.PushBack({
                        PC.Size,
                        StagesMask
                    });
                }

                FPath OutputPath = I_SourcePath;
                FString OutputName = OutputPath.GetFileName().GetUTF8Path();
                const auto DotPos = OutputName.FindLast(".");
                if (DotPos != FString::NPos)
                { OutputName = OutputName.SubString(0, DotPos); }
                // Emit one .vshader per entry point (Vulkan has stage-specific SPIR-V).
                OutputName.Append(".");
                OutputName.Append(EntryPoint);
                OutputName.Append(".vshader");
                OutputPath = OutputPath.GetParent() / FPath(OutputName);

                if (Save(Shader, OutputPath))
                {
                    LOG_INFO("Successfully compiled and saved: {}", OutputPath);
                    ++SuccessCount;
                    continue;
                }
                LOG_ERROR("Failed to save shader: {}", OutputPath);
                return False;
            }

            if (SuccessCount == 0)
            {
                LOG_ERROR("Failed to compile shader: {} (no valid entry point found)", I_SourcePath);
                return False;
            }
            return True;
        }
    }

    int Execute(int I_Argc, char* I_Argv[])
    {
        if (I_Argc < 2)
        {
            LOG_INFO("Visera-Forge - Shader compilation and validation");
            LOG_INFO("  Shader \"<directory>\" \"<pattern>\"  - Compile .slang to .vshader + .spv");
            LOG_INFO("  Validate \"<directory>\" \"<pattern>\" - Validate .vshader binary (PascalCase names)");
            LOG_INFO("Example: Visera-Forge Shader \".\\Engine\\Shaders\" \"*.slang\"");
            LOG_INFO("Example: Visera-Forge Validate \".\\Engine\\Shaders\" \"*.vshader\"");
            return 0;
        }

        const FStringView Command = I_Argv[1];

        if (Command == "Font")
        {
            LOG_INFO("Baking Font (WIP)");
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);
            return 0;
        }

        if (Command == "Shader")
        {
            if (I_Argc < 4)
            {
                LOG_ERROR("Shader command requires directory and pattern (both provided by caller)");
                LOG_INFO("Usage: Visera-Forge Shader \"<directory>\" \"<pattern>\"");
                LOG_INFO("Example: Visera-Forge Shader \".\\Engine\\Shaders\" \"*.slang\"");
                return 1;
            }
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);

            const FPath SearchDir = FPath{I_Argv[2]};
            const FStringView Pattern = I_Argv[3];
            LOG_INFO("Searching for shader files in {} matching pattern: {}", SearchDir, Pattern);

            if (!FFileSystem::Exists(SearchDir) || !FFileSystem::IsDirectory(SearchDir))
            {
                LOG_ERROR("Directory does not exist or is not a directory: {}", SearchDir);
                return 1;
            }

            TArray<FPath> MatchedFiles = FindMatchingFiles(SearchDir, Pattern);

            if (MatchedFiles.IsEmpty())
            {
                LOG_WARN("No shader files found matching pattern: {}", Pattern);
                return 1;
            }

            LOG_INFO("Found {} shader file(s) to compile", MatchedFiles.GetSize());

            UInt32 SuccessCount = 0;
            UInt32 FailCount = 0;

            for (const auto& FilePath : MatchedFiles)
            {
                if (CompileShader(FilePath))
                {
                    ++SuccessCount;
                }
                else
                {
                    ++FailCount;
                }
            }

            LOG_INFO("Compilation complete: {} succeeded, {} failed", SuccessCount, FailCount);
            return FailCount > 0 ? 1 : 0;
        }

        if (Command == "Validate")
        {
            if (I_Argc < 4)
            {
                LOG_ERROR("Validate requires directory and pattern (provided by caller)");
                LOG_INFO("Usage: Visera-Forge Validate \"<directory>\" \"<pattern>\" [no-meta]");
                LOG_INFO("Example: Visera-Forge Validate \".\\Engine\\Shaders\" \"*.vshader\"");
                LOG_INFO("  no-meta: skip writing .vshader.meta (reflection JSON); default is to write.");
                return 1;
            }
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);
            const FPath SearchDir = FPath{I_Argv[2]};
            const FStringView Pattern = I_Argv[3];
            const Bool OutputMeta = (I_Argc < 5 || (FStringView{I_Argv[4]} != "no-meta" && FStringView{I_Argv[4]} != "0"));
            LOG_INFO("Validating shader files in {} matching pattern: {} (output .meta: {})", SearchDir, Pattern, OutputMeta);
            if (!FFileSystem::Exists(SearchDir) || !FFileSystem::IsDirectory(SearchDir))
            {
                LOG_ERROR("Directory does not exist or is not a directory: {}", SearchDir);
                return 1;
            }
            TArray<FPath> MatchedFiles = FindMatchingFiles(SearchDir, Pattern);
            if (MatchedFiles.IsEmpty())
            {
                LOG_WARN("No files found matching pattern: {}", Pattern);
                return 1;
            }
            UInt32 PassCount = 0;
            UInt32 FailCount = 0;
            for (const auto& FilePath : MatchedFiles)
            {
                auto Result = Validate(FilePath, OutputMeta);
                if (Result.Ok)
                {
                    LOG_INFO("OK: {}", FilePath);
                    ++PassCount;
                }
                else
                {
                    LOG_WARN("FAIL: {}", FilePath);
                    for (const auto& Err : Result.Errors)
                    { LOG_WARN("  - {}", Err); }
                    ++FailCount;
                }
            }
            LOG_INFO("Validation complete: {} passed, {} failed", PassCount, FailCount);
            return FailCount > 0 ? 1 : 0;
        }

        LOG_WARN("Unknown command: {}", Command);
        return 1;
    }
}

export int main(int I_Argc, char* I_Argv[])
{
    return Visera::Forge::Execute(I_Argc, I_Argv);
}