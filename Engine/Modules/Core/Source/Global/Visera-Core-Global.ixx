module;
#include <Visera-Core.hpp>
export module Visera.Core.Global;
#define VISERA_MODULE_NAME "Core.Global"

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API IGlobalSingleton
    {
    public:
        enum class EStatus { Disabled, Bootstrapped, Terminated };

        virtual void
        Bootstrap() = 0;
        virtual void
        Terminate() = 0;

        [[nodiscard]] Bool
        IsBootstrapped() const { return Status == EStatus::Bootstrapped; }
        [[nodiscard]] Bool
        IsTerminated() const   { return Status == EStatus::Terminated; }

        [[nodiscard]] FStringView
        GetDebugName() const { return Name; }

        virtual ~IGlobalSingleton()
        {
            if (IsBootstrapped())
            { printf(Format("{} NOT terminated properly!", GetDebugName()).data()); }
        }

    protected:
        const char* Name;
        mutable EStatus Status = EStatus::Disabled;

        IGlobalSingleton() = delete;
        explicit IGlobalSingleton(const char* I_Name) : Name(I_Name) {}

        IGlobalSingleton(const IGlobalSingleton&)			 = delete;
        IGlobalSingleton& operator=(const IGlobalSingleton&) = delete;
        IGlobalSingleton(IGlobalSingleton&&)				 = delete;
        IGlobalSingleton& operator=(IGlobalSingleton&&)      = delete;
    };
}