module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Events.MulticastDelegate;
#define VISERA_MODULE_NAME "Runtime.Events.MulticastDelegate"
import Visera.Runtime.Events.Delegate;
import Visera.Core.Log;

namespace Visera
{
    /**
     * @brief Multicast delegate (UE5 DECLARE_MULTICAST_DELEGATE equivalent)
     * 
     * Allows multiple functions to be bound and executed in sequence.
     * Perfect for event broadcasting in game engines.
     */
    export template<typename... Args>
    class TMulticastDelegate
    {
    public:
        TMulticastDelegate() = default;
        ~TMulticastDelegate() = default;

        // Move semantics
        TMulticastDelegate(TMulticastDelegate&& Other) noexcept 
            : Bindings(std::move(Other.Bindings)) {}

        TMulticastDelegate& operator=(TMulticastDelegate&& Other) noexcept
        {
            if (this != &Other)
            {
                Bindings = std::move(Other.Bindings);
            }
            return *this;
        }

        // No copy semantics for performance
        TMulticastDelegate(const TMulticastDelegate&) = delete;
        TMulticastDelegate& operator=(const TMulticastDelegate&) = delete;

        /**
         * @brief Add a free function binding
         */
        template<void(*Function)(Args...)>
        FDelegateHandle AddStatic()
        {
            auto Handle = GenerateHandle();
            auto Binding = MakeUnique<FFunctionBinding<Args...>>(Function, Handle);
            Bindings.emplace_back(std::move(Binding));
            return Handle;
        }

        /**
         * @brief Add a member function binding with shared_ptr safety
         */
        template<typename TClass, void(TClass::*Function)(Args...)>
        FDelegateHandle AddUObject(TSharedPtr<TClass> Object)
        {
            auto Handle = GenerateHandle();
            auto Binding = MakeUnique<FMemberBinding<TClass, Args...>>(Object, Function, Handle);
            Bindings.emplace_back(std::move(Binding));
            return Handle;
        }

        /**
         * @brief Add a lambda or functor binding
         */
        template<typename TLambda>
        FDelegateHandle AddLambda(TLambda&& Lambda)
        requires DelegateConcepts::CallableWith<TLambda, Args...>
        {
            auto Handle = GenerateHandle();
            auto Binding = MakeUnique<FLambdaBinding<std::decay_t<TLambda>, Args...>>(
                std::forward<TLambda>(Lambda), Handle);
            Bindings.emplace_back(std::move(Binding));
            return Handle;
        }

        /**
         * @brief Remove a specific binding by handle
         */
        Bool Remove(FDelegateHandle Handle)
        {
            auto It = std::find_if(Bindings.begin(), Bindings.end(),
                [Handle](const auto& Binding) 
                {
                    return Binding && Binding->GetHandle() == Handle;
                });

            if (It != Bindings.end())
            {
                Bindings.erase(It);
                return true;
            }
            return false;
        }

        /**
         * @brief Remove all bindings from a specific object
         */
        template<typename TClass>
        Int32 RemoveAll(TSharedPtr<TClass> Object)
        {
            Int32 RemovedCount = 0;
            Bindings.erase(
                std::remove_if(Bindings.begin(), Bindings.end(),
                    [&Object, &RemovedCount](const auto& Binding)
                    {
                        // Check if this is a member binding for the specified object
                        if (auto MemberBinding = dynamic_cast<FMemberBinding<TClass, Args...>*>(Binding.get()))
                        {
                            if (auto BindingObject = MemberBinding->ObjectWeak.lock())
                            {
                                if (BindingObject == Object)
                                {
                                    ++RemovedCount;
                                    return true;
                                }
                            }
                        }
                        return false;
                    }),
                Bindings.end());
            return RemovedCount;
        }

        /**
         * @brief Broadcast to all bound functions
         */
        void Broadcast(Args... args)
        {
            // Clean up invalid bindings first
            CleanupInvalidBindings();

            // Execute all valid bindings
            for (const auto& Binding : Bindings)
            {
                if (Binding && Binding->IsValid())
                {
                    try
                    {
                        Binding->Execute(args...);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("Exception in multicast delegate execution: {}", e.what());
                    }
                }
            }
        }

        /**
         * @brief Check if any delegates are bound
         */
        Bool IsBound() const
        {
            return !Bindings.empty() && std::any_of(Bindings.begin(), Bindings.end(),
                [](const auto& Binding) { return Binding && Binding->IsValid(); });
        }

        /**
         * @brief Get number of bound delegates
         */
        UInt32 GetBoundCount() const
        {
            return static_cast<UInt32>(std::count_if(Bindings.begin(), Bindings.end(),
                [](const auto& Binding) { return Binding && Binding->IsValid(); }));
        }

        /**
         * @brief Clear all bindings
         */
        void Clear()
        {
            Bindings.clear();
        }

        /**
         * @brief Remove invalid bindings (cleanup)
         */
        void CleanupInvalidBindings()
        {
            Bindings.erase(
                std::remove_if(Bindings.begin(), Bindings.end(),
                    [](const auto& Binding) 
                    {
                        return !Binding || !Binding->IsValid();
                    }),
                Bindings.end());
        }

    private:
        TArray<TUniquePtr<IDelegateBinding<Args...>>> Bindings;

        static FDelegateHandle GenerateHandle()
        {
            static std::atomic<FDelegateHandle> HandleCounter{1};
            return HandleCounter.fetch_add(1);
        }
    };

    /**
     * @brief Event dispatcher class for named events (similar to UE5's event system)
     */
    export template<typename... Args>
    class TEventDispatcher
    {
    public:
        using EventDelegate = TMulticastDelegate<Args...>;
        using EventMap = TMap<FString, TUniquePtr<EventDelegate>>;

        /**
         * @brief Get or create an event by name
         */
        EventDelegate& GetEvent(const FString& EventName)
        {
            auto It = Events.find(EventName);
            if (It == Events.end())
            {
                auto [InsertIt, Success] = Events.emplace(EventName, MakeUnique<EventDelegate>());
                return *InsertIt->second;
            }
            return *It->second;
        }

        /**
         * @brief Broadcast an event by name
         */
        void BroadcastEvent(const FString& EventName, Args... args)
        {
            auto It = Events.find(EventName);
            if (It != Events.end() && It->second)
            {
                It->second->Broadcast(args...);
            }
        }

        /**
         * @brief Check if an event exists and has bindings
         */
        Bool IsEventBound(const FString& EventName) const
        {
            auto It = Events.find(EventName);
            return It != Events.end() && It->second && It->second->IsBound();
        }

        /**
         * @brief Remove an event entirely
         */
        Bool RemoveEvent(const FString& EventName)
        {
            return Events.erase(EventName) > 0;
        }

        /**
         * @brief Clear all events
         */
        void ClearAllEvents()
        {
            Events.clear();
        }

        /**
         * @brief Get all event names
         */
        TArray<FString> GetEventNames() const
        {
            TArray<FString> Names;
            Names.reserve(Events.size());
            for (const auto& [Name, _] : Events)
            {
                Names.push_back(Name);
            }
            return Names;
        }

    private:
        EventMap Events;
    };

}