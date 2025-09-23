module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Event.Delegate;
#define VISERA_MODULE_NAME "Runtime.Events.Delegate"
import Visera.Core.Log;

namespace Visera
{
    /**
     * @brief Unique identifier for delegate bindings
     */
    export using FDelegateHandle = UInt64;

    /**
     * @brief Concepts for delegate function types
     */
    export namespace DelegateConcepts
    {
        template<typename T>
        concept Callable = std::is_invocable_v<T>;

        template<typename T, typename... Args>
        concept CallableWith = std::is_invocable_v<T, Args...>;

        template<typename T, typename R, typename... Args>
        concept CallableWithReturn = std::is_invocable_r_v<R, T, Args...>;
    }

    /**
     * @brief Base delegate binding interface
     */
    template<typename... Args>
    class IDelegateBinding
    {
    public:
        virtual ~IDelegateBinding() = default;
        virtual void Execute(Args... args) = 0;
        virtual Bool IsValid() const = 0;
        virtual FDelegateHandle GetHandle() const = 0;
    };

    /**
     * @brief Binding for free functions and static methods
     */
    template<typename... Args>
    class FFunctionBinding : public IDelegateBinding<Args...>
    {
    public:
        using FunctionType = void(*)(Args...);

        explicit FFunctionBinding(FunctionType I_Function, FDelegateHandle I_Handle)
            : Function(I_Function), Handle(I_Handle) {}

        void Execute(Args... args) override
        {
            if (Function)
            {
                Function(args...);
            }
        }

        Bool IsValid() const override { return Function != nullptr; }
        FDelegateHandle GetHandle() const override { return Handle; }

    private:
        FunctionType Function;
        FDelegateHandle Handle;
    };

    /**
     * @brief Binding for member functions with weak_ptr safety
     */
    template<typename TClass, typename... Args>
    class FMemberBinding : public IDelegateBinding<Args...>
    {
    public:
        using MemberFunctionType = void(TClass::*)(Args...);

        FMemberBinding(TWeakPtr<TClass> I_Object, MemberFunctionType I_Function, FDelegateHandle I_Handle)
            : ObjectWeak(I_Object), Function(I_Function), Handle(I_Handle) {}

        void Execute(Args... args) override
        {
            if (auto Object = ObjectWeak.lock())
            {
                (Object.get()->*Function)(args...);
            }
        }

        Bool IsValid() const override 
        { 
            return !ObjectWeak.expired() && Function != nullptr; 
        }

        FDelegateHandle GetHandle() const override { return Handle; }

    private:
        TWeakPtr<TClass> ObjectWeak;
        MemberFunctionType Function;
        FDelegateHandle Handle;
    };

    /**
     * @brief Binding for lambda functions and functors
     */
    template<typename TLambda, typename... Args>
    class FLambdaBinding : public IDelegateBinding<Args...>
    {
    public:
        explicit FLambdaBinding(TLambda&& I_Lambda, FDelegateHandle I_Handle)
            : Lambda(std::forward<TLambda>(I_Lambda)), Handle(I_Handle) {}

        void Execute(Args... args) override
        {
            Lambda(args...);
        }

        Bool IsValid() const override { return true; }
        FDelegateHandle GetHandle() const override { return Handle; }

    private:
        TLambda Lambda;
        FDelegateHandle Handle;
    };

    /**
     * @brief Single-cast delegate (UE5 DECLARE_DELEGATE equivalent)
     */
    export template<typename... Args>
    class TDelegate
    {
    public:
        TDelegate() = default;
        ~TDelegate() = default;

        // Move semantics
        TDelegate(TDelegate&& Other) noexcept : Binding(std::move(Other.Binding)) {}
        TDelegate& operator=(TDelegate&& Other) noexcept
        {
            if (this != &Other)
            {
                Binding = std::move(Other.Binding);
            }
            return *this;
        }

        // No copy semantics for performance
        TDelegate(const TDelegate&) = delete;
        TDelegate& operator=(const TDelegate&) = delete;

        /**
         * @brief Bind a free function or static method
         */
        template<void(*Function)(Args...)>
        void BindStatic()
        {
            Binding = MakeUnique<FFunctionBinding<Args...>>(Function, GenerateHandle());
        }

        /**
         * @brief Bind a member function with shared_ptr safety
         */
        template<typename TClass, void(TClass::*Function)(Args...)>
        void BindUObject(TSharedPtr<TClass> Object)
        {
            Binding = MakeUnique<FMemberBinding<TClass, Args...>>(Object, Function, GenerateHandle());
        }

        /**
         * @brief Bind a lambda or functor
         */
        template<typename TLambda>
        void BindLambda(TLambda&& Lambda)
        requires DelegateConcepts::CallableWith<TLambda, Args...>
        {
            Binding = MakeUnique<FLambdaBinding<std::decay_t<TLambda>, Args...>>(
                std::forward<TLambda>(Lambda), GenerateHandle());
        }

        /**
         * @brief Execute the delegate
         */
        void ExecuteIfBound(Args... args)
        {
            if (IsBound())
            {
                Binding->Execute(args...);
            }
        }

        /**
         * @brief Check if delegate is bound and valid
         */
        Bool IsBound() const
        {
            return Binding && Binding->IsValid();
        }

        /**
         * @brief Clear the binding
         */
        void Unbind()
        {
            Binding.reset();
        }

        /**
         * @brief Get delegate handle
         */
        FDelegateHandle GetHandle() const
        {
            return Binding ? Binding->GetHandle() : 0;
        }

    private:
        TUniquePtr<IDelegateBinding<Args...>> Binding;

        static FDelegateHandle GenerateHandle()
        {
            static std::atomic<FDelegateHandle> HandleCounter{1};
            return HandleCounter.fetch_add(1);
        }
    };

}