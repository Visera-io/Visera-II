module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Platform.Interface.Library;
#define VISERA_MODULE_NAME "Runtime.Platform"
export import Visera.Core.Types.Path;

namespace Visera
{
    export class VISERA_RUNTIME_API ILibrary
    {
    public:
        [[nodiscard]] virtual void*
        LoadFunction(const char* I_Name) const = 0;

        [[nodiscard]] inline const FPath&
        GetPath() const { return Path; }

        [[nodiscard]] inline Bool
        IsLoaded() const { return Handle != nullptr; }

        ILibrary() = delete;
        ILibrary(const FPath& I_Path) : Path(I_Path) { }
        virtual ~ILibrary() = default;

    protected:
        FPath Path;
        void* Handle{nullptr};
    };
}