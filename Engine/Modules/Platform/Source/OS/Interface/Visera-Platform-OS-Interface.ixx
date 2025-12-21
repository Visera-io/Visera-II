module;
#include <Visera-Platform.hpp>
export module Visera.Platform.OS.Interface;
#define VISERA_MODULE_NAME "Platform.OS"
export import Visera.Platform.OS.Interface.Library;
export import Visera.Core.Types.Path;

namespace Visera
{
    export enum class EPlatform
    {
        Unknown,

        Windows,
        MacOS,
    };

    export class VISERA_PLATFORM_API IOS
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
        [[nodiscard]] virtual UInt128
        GenerateUUID() const = 0;
        [[nodiscard]] inline EPlatform
        GetType() const { return Type; }

    public:
        explicit IOS() = delete;
        explicit IOS(EPlatform I_Type) : Type(I_Type) {};
        virtual ~IOS() {}

    private:
        const EPlatform Type;
    };
}