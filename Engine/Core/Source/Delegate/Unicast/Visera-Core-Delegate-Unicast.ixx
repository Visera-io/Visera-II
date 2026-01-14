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
    template<typename R, typename... T_Args>
    class VISERA_CORE_API TUnicastDelegate<R(T_Args...), Policy::Shared>
    {
    public:
        using FCallback = TFunction<R(T_Args...)>;

        void inline
        Bind(FCallback I_Callback) { Callback = std::move(I_Callback); }
        void inline
        Unbind() { Callback = nullptr; }
        std::conditional_t<std::is_void_v<R>, void, std::optional<R>>
        Invoke(T_Args... I_Args) const
        {
            if (!Callback)
            {
                if constexpr (std::is_void_v<R>) return;
                else return std::nullopt;
            }
            if constexpr (std::is_void_v<R>)
            {
                Callback(std::forward<T_Args>(I_Args)...);
                return;
            }
            else
            {
                return Callback(std::forward<T_Args>(I_Args)...);
            }
        }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };

    // Exclusive: can bind once and Can NOT unbind.
    template<typename R, typename... T_Args>
    class VISERA_CORE_API TUnicastDelegate<R(T_Args...), Policy::Exclusive>
    {
    public:
        using FCallback = TFunction<R(T_Args...)>;

        [[nodiscard]] inline Bool
        TryBind(FCallback I_Callback)
        {
            if (IsBind()) { return False; }
            Callback = std::move(I_Callback);
            return True;
        }

        std::conditional_t<std::is_void_v<R>, void, std::optional<R>>
        Invoke(T_Args... I_Args) const
        {
            if (!Callback)
            {
                if constexpr (std::is_void_v<R>) return;
                else return std::nullopt;
            }
            if constexpr (std::is_void_v<R>)
            {
                Callback(std::forward<T_Args>(I_Args)...);
                return;
            }
            else
            {
                return Callback(std::forward<T_Args>(I_Args)...);
            }
        }

        [[nodiscard]] inline Bool
        IsBind() const { return Callback != nullptr; }

    private:
        FCallback Callback;
    };
}