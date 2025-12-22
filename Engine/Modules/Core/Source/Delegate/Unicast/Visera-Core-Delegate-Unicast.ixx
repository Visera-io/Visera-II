module;
#include <Visera-Core.hpp>
export module Visera.Core.Delegate.Unicast;
#define VISERA_MODULE_NAME "Core.Delegate"
import Visera.Core.Traits.Policy;

export namespace Visera
{
    // Default policy is Exclusive
    template<typename T_Signature, typename T_Policy = Policy::Exclusive>
    class VISERA_CORE_API TUnicastDelegate;

    // Shared: can re-bind and can unbind.
    template<typename... T_Args>
    class VISERA_CORE_API TUnicastDelegate<void(T_Args...), Policy::Shared>
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

    // Exclusive: can bind once and Can NOT unbind.
    template<typename... T_Args>
    class VISERA_CORE_API TUnicastDelegate<void(T_Args...), Policy::Exclusive>
    {
    public:
        using FCallback = TFunction<void(T_Args...)>;

        [[nodiscard]] inline Bool
        TryBind(FCallback I_Callback) { if (IsBind()) { return False; }  Callback = std::move(I_Callback); return True; }
        void inline
        Invoke(T_Args... I_Args) const { if (Callback) { Callback(I_Args...); } }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };
}