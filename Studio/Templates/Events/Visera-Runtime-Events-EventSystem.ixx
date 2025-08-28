module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Events.EventSystem;
#define VISERA_MODULE_NAME "Runtime.Events.EventSystem"
import Visera.Runtime.Events.Delegate;
import Visera.Runtime.Events.MulticastDelegate;
import Visera.Runtime.Core.Subsystem;
import Visera.Core.Log;
import Visera.Core.Types.Name;

namespace Visera
{
    /**
     * @brief Base event class for typed events
     */
    export class VISERA_RUNTIME_API FEvent
    {
    public:
        explicit FEvent(const FString& I_EventType) 
            : EventType(I_EventType), EventName(I_EventType), Timestamp(std::chrono::high_resolution_clock::now()) {}
            
        explicit FEvent(const FName& I_EventName)
            : EventType(I_EventName.GetName()), EventName(I_EventName), Timestamp(std::chrono::high_resolution_clock::now()) {}
        
        virtual ~FEvent() = default;

        const FString& GetEventType() const { return EventType; }
        const FName& GetEventName() const { return EventName; }
        auto GetTimestamp() const { return Timestamp; }

        template<typename T>
        const T* As() const 
        {
            return dynamic_cast<const T*>(this);
        }

    protected:
        FString EventType;  ///< String representation for compatibility
        FName EventName;    ///< High-performance FName for fast comparisons
        std::chrono::high_resolution_clock::time_point Timestamp;
    };

    /**
     * @brief Templated event class for specific event types
     */
    export template<typename TEventData>
    class TEvent : public FEvent
    {
    public:
        explicit TEvent(const FString& I_EventType, TEventData I_Data)
            : FEvent(I_EventType), Data(std::move(I_Data)) {}

        const TEventData& GetData() const { return Data; }

    private:
        TEventData Data;
    };

    /**
     * @brief Event system subsystem interface
     */
    export class VISERA_RUNTIME_API IEventSubsystem : public ISubsystem
    {
    public:
        IEventSubsystem() : ISubsystem("EventSubsystem", ESubsystemType::Events) {}

        /**
         * @brief Subscribe to events with a callback (string-based)
         */
        virtual FDelegateHandle Subscribe(const FString& EventType, 
            std::function<void(const FEvent&)> Callback) = 0;
            
        /**
         * @brief Subscribe to events with a callback (FName-based, high-performance)
         */
        virtual FDelegateHandle Subscribe(const FName& EventName, 
            std::function<void(const FEvent&)> Callback) = 0;

        /**
         * @brief Unsubscribe from events (string-based)
         */
        virtual Bool Unsubscribe(const FString& EventType, FDelegateHandle Handle) = 0;
        
        /**
         * @brief Unsubscribe from events (FName-based, high-performance)
         */
        virtual Bool Unsubscribe(const FName& EventName, FDelegateHandle Handle) = 0;

        /**
         * @brief Publish an event
         */
        virtual void Publish(TSharedPtr<FEvent> Event) = 0;

        /**
         * @brief Publish an event immediately (synchronous)
         */
        virtual void PublishImmediate(TSharedPtr<FEvent> Event) = 0;

        /**
         * @brief Process queued events
         */
        virtual void ProcessEvents() = 0;

        /**
         * @brief Clear all event subscriptions
         */
        virtual void ClearAllSubscriptions() = 0;
    };

    /**
     * @brief Concrete event system implementation
     */
    export class VISERA_RUNTIME_API FEventSystem : public IEventSubsystem
    {
    public:
        FEventSystem() = default;
        ~FEventSystem() override = default;

        // ISubsystem interface
        Bool Initialize() override;
        void Update(Float I_DeltaTime) override;
        void Shutdown() override;
        void Bootstrap() override;
        void Terminate() override;

        // IEventSubsystem interface
        FDelegateHandle Subscribe(const FString& EventType, 
            std::function<void(const FEvent&)> Callback) override;
        FDelegateHandle Subscribe(const FName& EventName, 
            std::function<void(const FEvent&)> Callback) override;
        Bool Unsubscribe(const FString& EventType, FDelegateHandle Handle) override;
        Bool Unsubscribe(const FName& EventName, FDelegateHandle Handle) override;
        void Publish(TSharedPtr<FEvent> Event) override;
        void PublishImmediate(TSharedPtr<FEvent> Event) override;
        void ProcessEvents() override;
        void ClearAllSubscriptions() override;

        /**
         * @brief Get event statistics
         */
        struct FEventStats
        {
            UInt64 TotalEventsPublished = 0;
            UInt64 TotalEventsProcessed = 0;
            UInt32 QueuedEventCount = 0;
            UInt32 SubscriberCount = 0;
        };

        FEventStats GetStats() const { return Stats; }

    private:
        using EventCallback = std::function<void(const FEvent&)>;
        using CallbacksMap = TMap<FString, TMulticastDelegate<const FEvent&>>;
        using FastCallbacksMap = TMap<FName, TMulticastDelegate<const FEvent&>>; // High-performance FName-based lookup
        using EventQueue = TArray<TSharedPtr<FEvent>>;

        CallbacksMap EventCallbacks;        ///< String-based callbacks for compatibility
        FastCallbacksMap FastEventCallbacks; ///< FName-based callbacks for performance
        EventQueue QueuedEvents;
        mutable std::shared_mutex EventMutex;
        FEventStats Stats;

        void ProcessEventInternal(const FEvent& Event);
    };

    /**
     * @brief Event system context for global access
     */
    export class VISERA_RUNTIME_API FEventContext
    {
    public:
        /**
         * @brief Set the active event subsystem
         */
        static void SetEventSubsystem(TSharedPtr<IEventSubsystem> I_Subsystem);

        /**
         * @brief Get the active event subsystem
         */
        static TSharedPtr<IEventSubsystem> GetEventSubsystem();

        /**
         * @brief Check if event subsystem is available
         */
        static Bool IsEventSystemAvailable();

        /**
         * @brief Convenience methods for common operations (string-based)
         */
        template<typename TEventData>
        static void PublishEvent(const FString& EventType, TEventData Data)
        {
            if (auto EventSystem = GetEventSubsystem())
            {
                auto Event = MakeShared<TEvent<TEventData>>(EventType, std::move(Data));
                EventSystem->Publish(Event);
            }
        }

        template<typename TCallback>
        static FDelegateHandle SubscribeToEvent(const FString& EventType, TCallback&& Callback)
        {
            if (auto EventSystem = GetEventSubsystem())
            {
                return EventSystem->Subscribe(EventType, std::forward<TCallback>(Callback));
            }
            return 0;
        }
        
        /**
         * @brief High-performance convenience methods (FName-based)
         */
        template<typename TEventData>
        static void PublishEvent(const FName& EventName, TEventData Data)
        {
            if (auto EventSystem = GetEventSubsystem())
            {
                auto Event = MakeShared<TEvent<TEventData>>(EventName, std::move(Data));
                EventSystem->Publish(Event);
            }
        }

        template<typename TCallback>
        static FDelegateHandle SubscribeToEvent(const FName& EventName, TCallback&& Callback)
        {
            if (auto EventSystem = GetEventSubsystem())
            {
                return EventSystem->Subscribe(EventName, std::forward<TCallback>(Callback));
            }
            return 0;
        }

    private:
        static TSharedPtr<IEventSubsystem> EventSubsystem;
    };

    /**
     * @brief Common event types for the engine
     */
    export namespace EventTypes
    {
        // String-based events for compatibility
        inline const FString EngineStartup = "Engine.Startup";
        inline const FString EngineShutdown = "Engine.Shutdown";
        inline const FString FrameBegin = "Engine.FrameBegin";
        inline const FString FrameEnd = "Engine.FrameEnd";

        // Window events
        inline const FString WindowResize = "Window.Resize";
        inline const FString WindowClose = "Window.Close";
        inline const FString WindowFocus = "Window.Focus";

        // Input events
        inline const FString KeyPressed = "Input.KeyPressed";
        inline const FString KeyReleased = "Input.KeyReleased";
        inline const FString MouseMoved = "Input.MouseMoved";
        inline const FString MouseClicked = "Input.MouseClicked";

        // Resource events
        inline const FString ResourceLoaded = "Resource.Loaded";
        inline const FString ResourceUnloaded = "Resource.Unloaded";
        inline const FString ResourceError = "Resource.Error";

        // Studio events
        inline const FString StudioSelectionChanged = "Studio.SelectionChanged";
        inline const FString StudioPropertyChanged = "Studio.PropertyChanged";
        inline const FString StudioToolActivated = "Studio.ToolActivated";
    }
    
    /**
     * @brief High-performance FName-based event types for hot paths
     * Use these for frequently-fired events to get O(1) performance
     */
    export namespace FastEventTypes
    {
        // Engine events (high frequency)
        inline const FName EngineStartup{"Engine.Startup"};
        inline const FName EngineShutdown{"Engine.Shutdown"};
        inline const FName FrameBegin{"Engine.FrameBegin"};
        inline const FName FrameEnd{"Engine.FrameEnd"};

        // Window events (high frequency)
        inline const FName WindowResize{"Window.Resize"};
        inline const FName WindowClose{"Window.Close"};
        inline const FName WindowFocus{"Window.Focus"};

        // Input events (very high frequency)
        inline const FName KeyPressed{"Input.KeyPressed"};
        inline const FName KeyReleased{"Input.KeyReleased"};
        inline const FName MouseMoved{"Input.MouseMoved"};
        inline const FName MouseClicked{"Input.MouseClicked"};

        // Resource events
        inline const FName ResourceLoaded{"Resource.Loaded"};
        inline const FName ResourceUnloaded{"Resource.Unloaded"};
        inline const FName ResourceError{"Resource.Error"};

        // Studio events
        inline const FName StudioSelectionChanged{"Studio.SelectionChanged"};
        inline const FName StudioPropertyChanged{"Studio.PropertyChanged"};
        inline const FName StudioToolActivated{"Studio.ToolActivated"};
    }

    /**
     * @brief Event data structures for common events
     */
    export namespace EventData
    {
        struct FWindowResizeData
        {
            UInt32 Width;
            UInt32 Height;
            UInt32 PreviousWidth;
            UInt32 PreviousHeight;
        };

        struct FKeyEventData
        {
            Int32 KeyCode;
            Bool IsPressed;
            Bool IsRepeat;
            UInt32 Modifiers;
        };

        struct FMouseEventData
        {
            Float X, Y;
            Float DeltaX, DeltaY;
            Int32 Button;
            Bool IsPressed;
        };

        struct FResourceEventData
        {
            FString ResourcePath;
            FString ResourceType;
            UInt64 ResourceID;
            Bool Success;
            FString ErrorMessage;
        };
    }

}