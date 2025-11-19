module;
#include <Visera-Core.hpp>
export module Visera.Core.Delegate.Multicast;
#define VISERA_MODULE_NAME "Core.Delegate"

namespace Visera
{
    export template<typename... T_Args>
    class VISERA_CORE_API TMulticastDelegate
    {
    public:
        using FFunctionID = UInt64;
        inline FFunctionID
        Subscribe(TFunction<void(T_Args...)> I_Function) { Functions.push_back(std::move(I_Function)); return Functions.size() - 1; }
        void inline
        Unsubscribe(FFunctionID I_FunctionID) { VISERA_ASSERT(I_FunctionID < Functions.size()); Functions[I_FunctionID] = nullptr; }
        void inline
        Broadcast(T_Args... I_Args) const { for (const auto& Function : Functions) { if (Function) { Function(I_Args...); } } }

    private:
        TArray<TFunction<void(T_Args...)>> Functions;
    };
}