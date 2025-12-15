module;
#include <Visera-Game.hpp>
export module Visera.Game.AssetHub.Shader.Common;
#define VISERA_MODULE_NAME "Game.AssetHub"

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