module;
#include <Visera-Shader.hpp>
export module Visera.Shader;
#define VISERA_MODULE_NAME "Shader"
import Visera.Core.Types.Array;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Shader.Slang;
import Visera.Global;

export namespace Visera
{
    class VISERA_SHADER_API FShader : public IGlobalService
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
        FShader() : IGlobalService(EName::Shader)
        {
            Dependencies =
            {
                EName::Platform,
            };

            if (!OnBootstrap.TryBind([this]
            {
                Compiler = MakeUnique<FSlangCompiler>();
                return Compiler != nullptr;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Compiler.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
}