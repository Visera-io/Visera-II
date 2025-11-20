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
        using FCallback = TFunction<void(T_Args...)>;
        
        void inline
        Bind(FCallback I_Callback) { Callback = std::move(I_Callback); }
        void inline
        Unbind() { Bind(nullptr); }
        void inline
        Invoke(T_Args... I_Args)   const { if (Callback) { Callback(I_Args...); } }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };
}