module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Path;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Core.Types.Path;
import Visera.Core.Types.String;

export namespace Visera
{
    /** Empty base for EBO; concrete platform path (e.g. FWindowsPath) derives and implicitly converts to native view. */
    struct VISERA_PLATFORM_API IPlatformPath
    {
        /** Default: no conversion; override in concrete platform path. */
        [[nodiscard]] virtual FPath ToPath() const
        {
            VISERA_ASSERT(False);
            return FPath(FString());
        }
        virtual ~IPlatformPath() = default;
    };
}
