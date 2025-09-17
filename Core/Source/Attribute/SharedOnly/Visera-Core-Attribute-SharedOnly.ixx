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
    protected:
        SharedOnly() = default; // Protected constructor to allow inheritance

    public:
        ~SharedOnly() = default;
    };
}