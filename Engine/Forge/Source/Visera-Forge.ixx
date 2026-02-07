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
import Visera.Forge.Baking.Font;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Core.Types.Pointer;
import Visera.Core.OS.FileSystem;
import Visera.Core.Algorithm.Ranges;
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Image;
import Visera.Forge.Utils.Wildcard;

namespace Visera::Forge
{
    namespace
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
            const FString PathStr = I_PathWithPattern.GetUTF8Path();
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
                if (FFileSystem::Exists(I_PathWithPattern) && !FFileSystem::IsDirectory(I_PathWithPattern))
                {
                    Results.PushBack(I_PathWithPattern);
                    return Results;
                }
                // If it's a directory, search all files in it
                if (FFileSystem::IsDirectory(I_PathWithPattern))
                {
                    SearchDir = I_PathWithPattern;
                    Pattern = "*";
                }
                else
                {
                    return Results; // File doesn't exist
                }
            }

            if (!FFileSystem::Exists(SearchDir) || !FFileSystem::IsDirectory(SearchDir))
            {
                return Results;
            }

            auto AllFiles = FFileSystem::EnumerateFiles(SearchDir, True);
            for (const auto& FilePath : AllFiles)
            {
                const FString FileName = FilePath.GetFileName().GetUTF8Path();
                if (WildcardMatch(FileName, Pattern))
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

        const FStringView Command = I_Argv[1];

        if (Command == "Font")
        {
            if (I_Argc < 5)
            {
                LOG_ERROR("Font command requires font path, atlas width, and atlas height");
                LOG_INFO("Usage: Visera-Forge Font \"<font_path>\" <width> <height> [font_size] [range] [format]");
                LOG_INFO("");
                LOG_INFO("Examples:");
                LOG_INFO("  # Generate EXR format (default, float precision):");
                LOG_INFO("  Visera-Forge Font \"./Fonts/Roboto.ttf\" 1024 1024 32 2.0 exr");
                LOG_INFO("");
                LOG_INFO("  # Generate PNG format (8-bit, smaller file size):");
                LOG_INFO("  Visera-Forge Font \"./Fonts/Roboto.ttf\" 1024 1024 32 2.0 png");
                LOG_INFO("");
                LOG_INFO("Parameters:");
                LOG_INFO("  font_path  - Path to the font file (.ttf, .otf, etc.)");
                LOG_INFO("  width      - Atlas texture width in pixels");
                LOG_INFO("  height     - Atlas texture height in pixels");
                LOG_INFO("  font_size  - Font size in pixels (default: 32)");
                LOG_INFO("  range      - MSDF distance field range (default: 2.0)");
                LOG_INFO("  format     - Output format: 'exr' (default) or 'png'");
                return 1;
            }
            
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);
            
            const FPath FontPath = FPath{I_Argv[2]};
            const UInt32 AtlasWidth = static_cast<UInt32>(std::strtoul(I_Argv[3], nullptr, 10));
            const UInt32 AtlasHeight = static_cast<UInt32>(std::strtoul(I_Argv[4], nullptr, 10));
            const Float FontSize = (I_Argc > 5) ? static_cast<Float>(std::strtod(I_Argv[5], nullptr)) : 32.0f;
            const Float Range = (I_Argc > 6) ? static_cast<Float>(std::strtod(I_Argv[6], nullptr)) : 2.0f;
            const FStringView FormatStr = (I_Argc > 7) ? FStringView{I_Argv[7]} : FStringView{"exr"};
            const Bool UsePNG = (FormatStr == "png" || FormatStr == "PNG");
            
            if (!FFileSystem::Exists(FontPath))
            {
                LOG_ERROR("Font file does not exist: {}", FontPath);
                return 1;
            }
            
            LOG_INFO("Baking font atlas: {}", FontPath);
            LOG_INFO("  Atlas size: {}x{}", AtlasWidth, AtlasHeight);
            LOG_INFO("  Font size: {}", FontSize);
            LOG_INFO("  Range: {}", Range);
            
            // Load font
            auto AssetHub = IGlobalService::Get<FAssetHub>(EName::AssetHub);
            if (!AssetHub)
            {
                LOG_ERROR("Failed to get AssetHub service!");
                return 1;
            }
            
            auto Font = AssetHub->LoadFont(FontPath, 0);
            if (!Font || !Font->IsLoaded())
            {
                LOG_ERROR("Failed to load font: {}", FontPath);
                return 1;
            }
            
            // Configure atlas generation
            FMSDFAtlasConfig Config;
            Config.FontSize = FontSize;
            Config.AtlasWidth = AtlasWidth;
            Config.AtlasHeight = AtlasHeight;
            Config.Range = Range;
            Config.Scale = 1.0f;
            Config.BorderPx = 2;
            // Default to ASCII printable characters (32-126)
            Config.CharacterSet.Resize(95);
            for (UInt32 i = 0; i < 95; ++i)
            {
                Config.CharacterSet[i] = 32 + i;
            }
            
            // Bake atlas
            FFontBaker Baker;
            auto ResultOpt = Baker.BakeAtlas(Font, Config);
            if (!ResultOpt.HasValue())
            {
                LOG_ERROR("Failed to bake font atlas!");
                return 1;
            }
            
            const FMSDFAtlasResult& Result = ResultOpt.GetValue();
            LOG_INFO("Successfully baked atlas with {} glyphs", Result.GlyphEntries.GetSize());
            
            // Save atlas image
            FPath OutputPath = FontPath;
            FString OutputName = OutputPath.GetFileName().GetUTF8Path();
            const auto DotPos = OutputName.FindLast(".");
            if (DotPos != FString::NPos)
            { OutputName = OutputName.SubString(0, DotPos); }
            
            // Convert to PNG if requested (convert RGBA32_Float to RGBA8_UNorm)
            TSharedPtr<const FImage> ImageToSave = Result.AtlasImage;
            if (UsePNG)
            {
                OutputName.Append(".msdf.png");
                // Convert float image to 8-bit UNorm for PNG
                // MSDF values are typically in range [-Range, Range], we need to map to [0, 1]
                auto PNGImage = MakeShared<FImage>(FImage::FCreateInfo
                {
                    .Width = Result.AtlasImage->GetWidth(),
                    .Height = Result.AtlasImage->GetHeight(),
                    .Depth = 1,
                    .PixelFormat = EPixelFormat::RGBA8_UNorm,
                    .ColorSpace = EColorSpace::Linear,
                });
                
                const Float* SrcData = reinterpret_cast<const Float*>(Result.AtlasImage->GetData());
                UInt8* DstData = PNGImage->AccessData();
                const UInt64 PixelCount = static_cast<UInt64>(PNGImage->GetWidth()) * PNGImage->GetHeight();
                
                // Map MSDF from [-Range, Range] to [0, 1] for RGB channels, keep alpha as-is
                const Float InvRange = 1.0f / (Range * 2.0f);
                for (UInt64 i = 0; i < PixelCount; ++i)
                {
                    const Float R = SrcData[i * 4 + 0];
                    const Float G = SrcData[i * 4 + 1];
                    const Float B = SrcData[i * 4 + 2];
                    const Float A = SrcData[i * 4 + 3];
                    
                    // Clamp and map to [0, 1]
                    DstData[i * 4 + 0] = static_cast<UInt8>(Math::Clamp((R + Range) * InvRange, 0.0f, 1.0f) * 255.0f);
                    DstData[i * 4 + 1] = static_cast<UInt8>(Math::Clamp((G + Range) * InvRange, 0.0f, 1.0f) * 255.0f);
                    DstData[i * 4 + 2] = static_cast<UInt8>(Math::Clamp((B + Range) * InvRange, 0.0f, 1.0f) * 255.0f);
                    DstData[i * 4 + 3] = static_cast<UInt8>(Math::Clamp(A, 0.0f, 1.0f) * 255.0f);
                }
                
                ImageToSave = PNGImage;
            }
            else
            {
                OutputName.Append(".msdf.exr");
            }
            
            OutputPath = OutputPath.GetParent() / FPath(OutputName);
            
            LOG_INFO("Saving atlas image to: {}", OutputPath);
            LOG_INFO("  Image size: {}x{}", ImageToSave->GetWidth(), ImageToSave->GetHeight());
            LOG_INFO("  Format: {}", static_cast<Int32>(ImageToSave->GetPixelFormat()));
            
            // Save image using AssetHub
            if (!AssetHub->SaveImage(ImageToSave, OutputPath))
            {
                LOG_ERROR("Failed to save atlas image to: {}", OutputPath);
                return 1;
            }
            
            LOG_INFO("Successfully saved MSDF font atlas to: {}", OutputPath);
            
            return 0;
        }

        if (Command == "Shader")
        {
            if (I_Argc < 3)
            {
                LOG_ERROR("Shader command requires path with optional wildcard pattern");
                LOG_INFO("Usage: Visera-Forge Shader \"<path_with_pattern>\"");
                LOG_INFO("Example: Visera-Forge Shader \"./Engine/Shaders/*.slang\"");
                return 1;
            }
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);

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
            if (I_Argc < 3)
            {
                LOG_ERROR("Validate requires path with optional wildcard pattern");
                LOG_INFO("Usage: Visera-Forge Validate \"<path_with_pattern>\" [no-meta]");
                LOG_INFO("Example: Visera-Forge Validate \"./Engine/Shaders/*.vshader\"");
                LOG_INFO("  no-meta: skip writing .vshader.meta (reflection JSON); default is to write.");
                return 1;
            }
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);
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