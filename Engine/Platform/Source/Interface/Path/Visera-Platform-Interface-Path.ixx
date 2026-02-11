module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Path;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    /** Platform-native path; concrete types (FWindowsPath, FMacOSPath) convert to native view. */
    struct VISERA_PLATFORM_API IPlatformPath
    {
        /** True when path is empty. */
        [[nodiscard]] virtual Bool IsEmpty() const
        {
            VISERA_ASSERT(False);
            return True;
        }
        /** Parent directory; nullptr if no parent. */
        [[nodiscard]] virtual TUniquePtr<IPlatformPath> GetParent() const
        {
            VISERA_ASSERT(False);
            return nullptr;
        }
        /** Last path component (filename); clone of this if no separator. */
        [[nodiscard]] virtual TUniquePtr<IPlatformPath> GetFileName() const
        {
            VISERA_ASSERT(False);
            return nullptr;
        }
        /** Convert to normalized FPath (UTF-8, forward slashes). */
        [[nodiscard]] virtual FPath ToPath() const
        {
            VISERA_ASSERT(False);
            return FPath(FString());
        }
        /** Clone this path for storage. */
        [[nodiscard]] virtual TUniquePtr<IPlatformPath> Clone() const
        {
            VISERA_ASSERT(False);
            return nullptr;
        }
        /** Concatenate: this / I_Other (with separator). */
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        Concat(const IPlatformPath& I_Other) const
        {
            VISERA_ASSERT(False);
            return nullptr;
        }
        /** Append suffix to path string (no separator). E.g. path.Append("XXXXXX"). */
        [[nodiscard]] virtual TUniquePtr<IPlatformPath>
        Append(FStringView I_Suffix) const
        {
            VISERA_ASSERT(False);
            return nullptr;
        }
        virtual ~IPlatformPath() = default;
    };
}
