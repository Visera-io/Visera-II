module;
#include <Visera-Engine.hpp>
export module Visera.Engine.AssetHub.Shader.Common;
#define VISERA_MODULE_NAME "Engine.AssetHub"

export namespace Visera
{
    enum class EShaderLanguage
    {
        Unknown,
        Slang,
        //SPIRV,
    };

    enum class EShaderType
    {
        Unknown,
        Vertex,
        Fragment,
    };
}

template <>
struct fmt::formatter<Visera::EShaderType>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::EShaderType& I_ShaderStage, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* StageName = "Unknown";
        switch (I_ShaderStage)
        {
            case Visera::EShaderType::Vertex:   StageName = "Vertex";   break;
            case Visera::EShaderType::Fragment: StageName = "Fragment"; break;
            default: break;
        }
        return fmt::format_to(I_Context.out(),"{}",StageName);
    }
};

template <>
struct fmt::formatter<Visera::EShaderLanguage>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::EShaderLanguage& I_ShaderLanguage, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        const char* LanguageName = "Unknown";
        switch (I_ShaderLanguage)
        {
            case Visera::EShaderLanguage::Slang:   LanguageName = "Slang";   break;
            default: break;
        }
        return fmt::format_to(I_Context.out(),"{}", LanguageName);
    }
};