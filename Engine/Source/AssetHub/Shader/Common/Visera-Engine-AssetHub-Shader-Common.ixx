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
    };
}
VISERA_MAKE_FORMATTER(Visera::EShaderLanguage,
    const char* LanguageName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::EShaderLanguage::Slang:   LanguageName = "Slang";   break;
        default: break;
    },
    "{}", LanguageName
);