module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Library;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Core.Types.Path;
export import Visera.Core.Types.UUID;
export import Visera.Core.Types.String;
export import Visera.Core.Types.Pointer.Shared;

namespace Visera
{
    export class VISERA_PLATFORM_API IPlatformLibrary
    {
    public:
        [[nodiscard]] virtual void*
        LoadFunction(const char* I_Name) const = 0;

        [[nodiscard]] inline const FPath&
        GetPath() const { return Path; }

        [[nodiscard]] inline Bool
        IsLoaded() const { return Handle != nullptr; }

        IPlatformLibrary() = delete;
        IPlatformLibrary(const FPath& I_Path) : Path(I_Path) { }
        virtual ~IPlatformLibrary()
        {
            Handle = nullptr; // Always set Handle to nullptr to prevent double-free
        }

    protected:
        FPath Path;
        void* Handle{nullptr};
    };
}