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
        Bind(FCallback I_Callback)       { Callback = std::move(I_Callback); }
        void inline
        Unbind() { Callback = nullptr; }
        void inline
        Invoke(T_Args... I_Args)   const { if (Callback) { Callback(I_Args...); } }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };

    // Can bind once and Can NOT unbind.
    export template<typename... T_Args>
    class VISERA_CORE_API TExclusiveDelegate
    {
    public:
        using FCallback = TFunction<void(T_Args...)>;

        [[nodiscard]] inline Bool
        TryBind(FCallback I_Callback) { if (IsBind()) { return False; }  Callback = std::move(I_Callback); return True; }
        void inline
        Invoke(T_Args... I_Args)   const { if (Callback) { Callback(I_Args...); } }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };
}