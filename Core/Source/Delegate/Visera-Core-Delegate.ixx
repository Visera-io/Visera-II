module;
#include <Visera-Core.hpp>
export module Visera.Core.Delegate;
#define VISERA_MODULE_NAME "Core.Delegate"
export import Visera.Core.Delegate.Multicast;

namespace Visera
{
    // Example: using OnDamage = TDelegate<Float>
    export template<typename... T_Args>
    class VISERA_CORE_API TDelegate
    {
    public:
        void inline
        Bind(TFunction<void(T_Args...)> I_Function) { Function = std::move(I_Function); }
        void inline
        Unbind() { Bind(nullptr); }
        void inline
        Invoke(T_Args... I_Args) const { if (Function) { Function(I_Args...); } }

        [[nodiscard]] inline Bool
        IsBind() const { return Function != nullptr; }

    private:
        TFunction<void(T_Args...)> Function;
    };
}