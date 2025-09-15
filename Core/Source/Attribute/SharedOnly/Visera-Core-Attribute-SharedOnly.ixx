module;
#include <Visera-Core.hpp>
export module Visera.Core.Attribute.SharedOnly;
#define VISERA_MODULE_NAME "Core.Attribute"
import Visera.Core.Attribute.NonCopyable;
import Visera.Core.Attribute.NonMovable;

namespace Visera::Attribute
{
    export template<typename T>
    class VISERA_CORE_API SharedOnly
        : public std::enable_shared_from_this<T>,
          public Attribute::NonCopyable<T>,
          public Attribute::NonMovable<T>
    {
    public:
        template<typename... Args>
        static auto
        Create(Args&&... I_Args) -> TSharedPtr<T>
        {
            struct FEnableMakeShared : public T
            {
                FEnableMakeShared(Args&&... I_Args)
                : T(std::forward<Args>(I_Args)...) {}
            };
            return MakeShared<FEnableMakeShared>(std::forward<Args>(I_Args)...);
        }

        inline TSharedPtr<T>
        SharedFromThis()       { return this->shared_from_this();  }
        inline TSharedPtr<const T>
        SharedFromThis() const { return this->shared_from_this();  }

    protected:
        SharedOnly() = default; // Disable Stack Allocation

    public:
        ~SharedOnly() = default;
    };
}