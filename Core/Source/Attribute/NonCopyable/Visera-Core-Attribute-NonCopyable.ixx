module;
#include <Visera-Core.hpp>
export module Visera.Core.Attribute.NonCopyable;
#define VISERA_MODULE_NAME "Core.Attribute"

namespace Visera::Attribute
{
    export template<typename T>
    class VISERA_CORE_API NonCopyable
    {
    protected:
        NonCopyable()  = default;
        ~NonCopyable() = default;

    public:
        NonCopyable(const NonCopyable&)            = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };
}