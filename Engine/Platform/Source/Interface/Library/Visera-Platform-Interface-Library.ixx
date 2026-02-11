module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Library;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Core.Types.Path;
export import Visera.Core.Types.UUID;
export import Visera.Core.Types.String;
export import Visera.Core.Types.Pointer.Shared;
export import Visera.Platform.Interface.Path;
import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    class VISERA_PLATFORM_API IPlatformLibrary
    {
    public:
        [[nodiscard]] virtual void*
        LoadFunction(const char* I_Name) const = 0;

        /** Facade: returns normalized FPath for cross-platform use. */
        [[nodiscard]] inline FPath
        GetPath() const { return Path ? Path->ToPath() : FPath(FString()); }

        [[nodiscard]] inline Bool
        IsLoaded() const { return Handle != nullptr; }

        IPlatformLibrary() = delete;
        explicit IPlatformLibrary(const IPlatformPath& I_Path) : Path(I_Path.Clone()) { }
        virtual ~IPlatformLibrary()
        {
            Handle = nullptr; // Always set Handle to nullptr to prevent double-free
        }

    protected:
        TUniquePtr<IPlatformPath> Path;
        void* Handle{nullptr};
    };
}