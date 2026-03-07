module;
#include <Visera-Forge.hpp>
export module Visera.Forge;
#define VISERA_MODULE_NAME "Forge"
import Visera.Core;
import Visera.Platform;
import Visera.Runtime;
import Visera;
import Visera.Forge.Utils;
import Visera.Forge.Shader;

namespace Visera::Forge
{
    // Find files matching pattern in a path (path can contain wildcards)
    [[nodiscard]] TArray<FPath>
    FindMatchingFiles(const FPath& I_PathWithPattern)
    {
        TArray<FPath> Results;

        // Extract directory and pattern from path
        FPath SearchDir = I_PathWithPattern;
        FString Pattern;

        // Check if path contains wildcard characters
        const FString PathStr = I_PathWithPattern.GetString();
        const Bool HasWildcard = Algorithm::FindIf(PathStr, [](char Ch) { return Ch == '*' || Ch == '?'; }) != PathStr.end();

        if (HasWildcard)
        {
            // Find the last directory separator before the wildcard
            const auto LastSlash = PathStr.FindLast('/');
            const auto LastBackslash = PathStr.FindLast('\\');
            const auto LastSep = (LastSlash == FString::NPos) ? LastBackslash :
                                ((LastBackslash == FString::NPos) ? LastSlash :
                                (LastSlash > LastBackslash ? LastSlash : LastBackslash));

            if (LastSep != FString::NPos)
            {
                SearchDir = FPath(PathStr.SubString(0, LastSep + 1));
                Pattern = PathStr.SubString(LastSep + 1);
            }
            else
            {
                // No directory separator, search current directory
                SearchDir = FPath(".");
                Pattern = PathStr;
            }
        }
        else
        {
            // No wildcard, treat as exact file path
            if (FPlatform::ExistsFile(I_PathWithPattern))
            {
                Results.PushBack(I_PathWithPattern);
                return Results;
            }
            // If it's a directory, search all files in it
            if (FPlatform::ExistsDirectory(I_PathWithPattern))
            {
                SearchDir = I_PathWithPattern;
                Pattern = "*";
            }
            else
            {
                return Results; // File doesn't exist
            }
        }

        SearchDir = FPath::Normalized(SearchDir);

        if (!FPlatform::ExistsDirectory(SearchDir))
        {
            return Results;
        }

        auto AllFiles = FPlatform::EnumerateFiles(SearchDir, True);
        for (const auto& FilePath : AllFiles)
        {
            if (auto FileNameOpt = FilePath.GetFileName(); FileNameOpt.HasValue() && WildcardMatch(*FileNameOpt, Pattern))
            {
                Results.PushBack(FilePath);
            }
        }

        return Results;
    }

    // Compile shader from slang file -> FShaderAsset, then save
    [[nodiscard]] Bool
    CompileShader(const FPath& I_SourcePath, FViseraEngine* I_Engine)
    {
        LOG_INFO("Compiling shader: {}", I_SourcePath);

        auto AssetHub = I_Engine->GetAssetHub();
        if (!AssetHub) { LOG_ERROR("AssetHub service is not available."); return False; }

        const TArray<FStringView> EntryPoints = {"VertMain", "FragMain"};
        const FPath ShaderDirectory = *I_SourcePath.GetParent();
        FShaderCompiler Compiler;

        UInt32 SuccessCount = 0;
        for (const auto& EntryPoint : EntryPoints)
        {
            auto SPIRV = Compiler.Compile(I_SourcePath, EntryPoint, ShaderDirectory);
            if (SPIRV.IsEmpty()) continue;
            auto ReflSlang = Compiler.ExtractReflection(I_SourcePath, EntryPoint, ShaderDirectory);
            if (ReflSlang.EntryPoints.IsEmpty()) continue;

            // Build runtime reflection (Visera::FRHIShaderLayout) for .vshader; RHI enums, not strings
            Visera::FRHIShaderLayout Refl;
            Refl.EntryPoints.Reserve(ReflSlang.EntryPoints.GetSize());
            for (const auto& EP : ReflSlang.EntryPoints)
            { Refl.EntryPoints.PushBack({ EP.Name, StageFromString(EP.Stage) }); }
            Refl.Resources.Reserve(ReflSlang.Resources.GetSize());
            for (const auto& R : ReflSlang.Resources)
            {
                ERHIShaderStage StagesMask = ERHIShaderStage::Undefined;
                for (const auto& S : R.Stages) { StagesMask |= StageFromString(S); }
                if (StagesMask == ERHIShaderStage::Undefined) { StagesMask = ERHIShaderStage::All; }
                Refl.Resources.PushBack({
                    R.Name,
                    R.Set,
                    R.Binding,
                    R.ArrayCount,
                    TypeFromString(R.Type),
                    AccessFromString(R.Access),
                    StagesMask
                });
            }
            Refl.PushConstants.Reserve(ReflSlang.PushConstants.GetSize());
            for (const auto& PC : ReflSlang.PushConstants)
            {
                ERHIShaderStage StagesMask = ERHIShaderStage::Undefined;
                for (const auto& S : PC.Stages) { StagesMask |= StageFromString(S); }
                if (StagesMask == ERHIShaderStage::Undefined) { StagesMask = ERHIShaderStage::All; }
                Refl.PushConstants.PushBack({ PC.Size, StagesMask });
            }

            FPath OutputPath = I_SourcePath;
            FString OutputName(*OutputPath.GetFileName());
            const auto DotPos = OutputName.FindLast(".");
            if (DotPos != FString::NPos)
            { OutputName = OutputName.SubString(0, DotPos); }
            // Emit one .vshader per entry point (Vulkan has stage-specific SPIR-V).
            OutputName.Append(".");
            OutputName.Append(EntryPoint);
            OutputName.Append(".vshader");
            OutputPath = *OutputPath.GetParent() / FPath(OutputName);

            if (!AssetHub->SaveShader({std::move(SPIRV), std::move(Refl)}, OutputPath))
            {
                LOG_ERROR("Failed to save shader: {}", OutputPath);
                return False;
            }
            LOG_INFO("Successfully compiled and saved: {}", OutputPath);
            ++SuccessCount;
        }

        if (SuccessCount == 0)
        {
            LOG_ERROR("Failed to compile shader: {} (no valid entry point found)", I_SourcePath);
            return False;
        }
        return True;
    }

    int Execute(int I_Argc, char* I_Argv[])
    {
        if (I_Argc < 2)
        {
            LOG_INFO("Visera-Forge - Asset baking and compilation tools");
            LOG_INFO("  Font \"<font_path>\" <width> <height> [size] [range] - Bake MSDF font atlas");
            LOG_INFO("  Shader \"<path_with_pattern>\"  - Compile .slang to .vshader + .spv");
            LOG_INFO("  Validate \"<path_with_pattern>\" [no-meta] - Validate .vshader binary");
            LOG_INFO("Examples:");
            LOG_INFO("  Visera-Forge Font \"./Fonts/Roboto.ttf\" 1024 1024");
            LOG_INFO("  Visera-Forge Shader \"./Engine/Shaders/*.slang\"");
            LOG_INFO("  Visera-Forge Validate \"./Engine/Shaders/*.vshader\"");
            return 0;
        }

        // Create Engine with Forge mode (Tasks, AssetHub, RHI only)
        auto Engine = Visera::CreateEngine(EEngineMode::Forge);
        if (!Engine)
        {
            LOG_FATAL("Failed to create Engine!");
            return 1;
        }

        FStringView Command = I_Argv[1];
        if (Command == "Shader")
        {
            if (I_Argc < 3)
            {
                LOG_ERROR("Shader command requires path with optional wildcard pattern");
                LOG_INFO("Usage: Visera-Forge Shader \"<path_with_pattern>\"");
                LOG_INFO("Example: Visera-Forge Shader \"./Engine/Shaders/*.slang\"");
                return 1;
            }

            const FPath PathWithPattern = FPath{I_Argv[2]};
            LOG_INFO("Searching for shader files matching: {}", PathWithPattern);

            TArray<FPath> MatchedFiles = FindMatchingFiles(PathWithPattern);

            if (MatchedFiles.IsEmpty())
            {
                LOG_WARN("No shader files found matching: {}", PathWithPattern);
                return 1;
            }

            LOG_INFO("Found {} shader file(s) to compile", MatchedFiles.GetSize());

            UInt32 SuccessCount = 0;
            UInt32 FailCount = 0;

            for (const auto& FilePath : MatchedFiles)
            {
                if (CompileShader(FilePath, Engine.Get()))
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
            if (I_Argc < 3)
            {
                LOG_ERROR("Validate requires path with optional wildcard pattern");
                LOG_INFO("Usage: Visera-Forge Validate \"<path_with_pattern>\" [no-meta]");
                LOG_INFO("Example: Visera-Forge Validate \"./Engine/Shaders/*.vshader\"");
                LOG_INFO("  no-meta: skip writing .vshader.meta (reflection JSON); default is to write.");
                return 1;
            }
            const FPath PathWithPattern = FPath{I_Argv[2]};
            const Bool OutputMeta = (I_Argc < 4 || (FStringView{I_Argv[3]} != "no-meta" && FStringView{I_Argv[3]} != "0"));
            LOG_INFO("Validating shader files matching: {} (output .meta: {})", PathWithPattern, OutputMeta);
            TArray<FPath> MatchedFiles = FindMatchingFiles(PathWithPattern);
            if (MatchedFiles.IsEmpty())
            {
                LOG_WARN("No files found matching: {}", PathWithPattern);
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