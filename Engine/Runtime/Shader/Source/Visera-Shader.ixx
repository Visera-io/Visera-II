module;
#include <Visera-Shader.hpp>
export module Visera.Shader;
#define VISERA_MODULE_NAME "Shader"
import Visera.Core.Types.Array;
import Visera.Core.Types.Path;
import Visera.Global.Service;
import Visera.Global.Log;
import Visera.Shader.Slang;

export namespace Visera
{
    class VISERA_SHADER_API FShader : public IGlobalService<FShader>
    {
    public:
        enum class ELanguage
        {
            Slang,
        };

        [[nodiscard]] inline TArray<FByte>
        Compile(const FPath& I_Path, FStringView I_EntryPoint)
        {
            LOG_TRACE("Compiling shader \"{}\" (entry_point:{}).",
                      I_Path, I_EntryPoint);
            return Compiler->Compile(I_Path, I_EntryPoint);
        }

    private:
        TUniquePtr<FSlangCompiler> Compiler;

    public:
        FShader() : IGlobalService(FName{"Shader"}) {}
        /*void
        Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Shader");

            Compiler = MakeUnique<FSlangCompiler>();

            Status = EStatus::Bootstrapped;
        }
        void
        Terminate() override
        {
            LOG_TRACE("Terminating Shader");

            Compiler.reset();

            Status = EStatus::Terminated;
        }*/
    };

    export inline VISERA_SHADER_API TUniquePtr<FShader>
    GShader = MakeUnique<FShader>();
}

VISERA_MAKE_FORMATTER(Visera::FShader::ELanguage,
    const char* LanguageName = "Unknown";
    switch (I_Formatee)
    {
        case Visera::FShader::ELanguage::Slang:   LanguageName = "Slang";   break;
        default: break;
    },
    "{}", LanguageName
);