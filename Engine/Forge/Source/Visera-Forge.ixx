module;
#include <Visera-Forge.hpp>
export module Visera.Forge;
#define VISERA_MODULE_NAME "Forge"
import Visera.Global;
import Visera.Tasks;
import Visera.AssetHub;
import Visera.Platform;
import Visera.Core.Types.String;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Core.OS.FileSystem;
import Visera.Forge.Utils.Wildcard;
import Visera.Shader;

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

        // Compile shader from slang file
        [[nodiscard]] Bool
        CompileShader(const FPath& I_SourcePath)
        {
            LOG_INFO("Compiling shader: {}", I_SourcePath);

            // Try common entry points
            const TArray<FStringView> EntryPoints = {"vertMain", "fragMain", "main", "vertex", "fragment"};

            for (const auto& EntryPoint : EntryPoints)
            {
                if (auto Shader = FShader::Compile(I_SourcePath, EntryPoint); Shader.HasValue())
                {
                    // Generate output path: replace .slang with .vshader
                    FPath OutputPath = I_SourcePath;
                    FString OutputName = OutputPath.GetFileName().GetUTF8Path();
                    const auto DotPos = OutputName.FindLast(".");
                    if (DotPos != FString::NPos)
                    {
                        OutputName = OutputName.SubString(0, DotPos);
                    }
                    OutputName.Append(".vshader");
                    OutputPath = OutputPath.GetParent() / FPath(OutputName);

                    if (Shader.GetValue().Save(OutputPath))
                    {
                        LOG_INFO("Successfully compiled and saved: {}", OutputPath);
                        return True;
                    }
                    else
                    {
                        LOG_ERROR("Failed to save shader: {}", OutputPath);
                        return False;
                    }
                }
            }

            LOG_ERROR("Failed to compile shader: {} (no valid entry point found)", I_SourcePath);
            return False;
        }
    }

    int Execute(int I_Argc, char* I_Argv[])
    {
        if (I_Argc < 2)
        {
            LOG_INFO("Visera-Forge - Shader compilation tool");
            LOG_INFO("Usage: Visera-Forge Shader \"<pattern>\"");
            LOG_INFO("Example: Visera-Forge Shader \"*.slang\"");
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
            if (I_Argc < 3)
            {
                LOG_ERROR("Shader command requires a pattern argument");
                LOG_INFO("Usage: Visera-Forge Shader \"<pattern>\"");
                LOG_INFO("Example: Visera-Forge Shader \"*.slang\"");
                return 1;
            }

            const FStringView Pattern = I_Argv[2];
            LOG_INFO("Searching for shader files matching pattern: {}", Pattern);

            // Search in common shader directories
            const FPath ResourceDir = FPlatform::GetResourceDirectory();
            const TArray<FPath> SearchDirs = {
                ResourceDir / FPath{"Assets/App/Shader"},
                ResourceDir / FPath{"Assets/Engine/Shader"},
                ResourceDir / FPath{"Assets/Studio/Shader"},
            };

            TArray<FPath> MatchedFiles;
            for (const auto& SearchDir : SearchDirs)
            {
                if (FFileSystem::Exists(SearchDir) && FFileSystem::IsDirectory(SearchDir))
                {
                    auto Files = FindMatchingFiles(SearchDir, Pattern);
                    for (auto& File : Files)
                    {
                        MatchedFiles.PushBack(std::move(File));
                    }
                }
            }

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

        LOG_WARN("Unknown command: {}", Command);
        return 1;
    }
}

export int main(int I_Argc, char* I_Argv[])
{
    return Visera::Forge::Execute(I_Argc, I_Argv);
}