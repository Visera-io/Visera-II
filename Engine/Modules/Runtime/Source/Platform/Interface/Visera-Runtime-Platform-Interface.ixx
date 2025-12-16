module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.Interface;
#define VISERA_MODULE_NAME "Runtime.Platform"
export import Visera.Runtime.Platform.Interface.Library;
export import Visera.Core.Types.Path;

namespace Visera
{
    export enum class EPlatform
    {
        Unknown,

        Windows,
        MacOS,
    };

    export class VISERA_RUNTIME_API IPlatform
    {
    public:
        [[nodiscard]] virtual TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const = 0;
        [[nodiscard]] virtual const FPath&
        GetExecutableDirectory() const = 0;
        [[nodiscard]] virtual const FPath&
        GetResourceDirectory() const = 0;
        [[nodiscard]] virtual const FPath&
        GetFrameworkDirectory() const = 0;
        [[nodiscard]] virtual const FPath&
        GetCacheDirectory() const = 0;
        [[nodiscard]] virtual Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const = 0;

        [[nodiscard]] inline EPlatform
        GetType() const { return Type; }

    public:
        explicit IPlatform() = delete;
        explicit IPlatform(EPlatform I_Type) : Type(I_Type) {};
        virtual ~IPlatform() {}

    private:
        const EPlatform Type;
    };
}