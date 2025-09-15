module;
#include <Visera-Core.hpp>
export module Visera.Core.Attribute.NonMovable;
#define VISERA_MODULE_NAME "Core.Attribute"

namespace Visera::Attribute
{
    export template<typename T>
    class VISERA_CORE_API NonMovable
    {
    protected:
        NonMovable()  = default;
        ~NonMovable() = default;

    public:
        NonMovable(NonMovable&&)            = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };
}