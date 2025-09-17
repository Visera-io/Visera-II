module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Events.EventSystemImpl;
#define VISERA_MODULE_NAME "Runtime.Events.EventSystemImpl"
import Visera.Runtime.Events.EventSystem;
import Visera.Core.Log;

namespace Visera
{
    // Static member definition
    TSharedPtr<IEventSubsystem> FEventContext::EventSubsystem = nullptr;

    // FEventContext implementations
    void FEventContext::SetEventSubsystem(TSharedPtr<IEventSubsystem> I_Subsystem)
    {
        EventSubsystem = I_Subsystem;
        if (EventSubsystem)
        {
            LOG_DEBUG("Event subsystem set successfully.");
        }
        else
        {
            LOG_DEBUG("Event subsystem cleared.");
        }
    }

    TSharedPtr<IEventSubsystem> FEventContext::GetEventSubsystem()
    {
        return EventSubsystem;
    }

    Bool FEventContext::IsEventSystemAvailable()
    {
        return EventSubsystem != nullptr && EventSubsystem->IsBootstrapped();
    }

    // FEventSystem implementations
    Bool FEventSystem::Initialize()
    {
        LOG_DEBUG("Initializing Event System...");

        // Initialize event statistics
        Stats = FEventStats{};

        LOG_DEBUG("Event System initialized successfully.");
        return true;
    }

    void FEventSystem::Bootstrap()
    {
        LOG_TRACE("Bootstrapping Event System...");

        if (!Initialize())
        {
            LOG_ERROR("Failed to initialize Event System during bootstrap.");
            return;
        }

        Status = EStatues::Bootstrapped;
        LOG_DEBUG("Event System bootstrapped successfully.");
    }

    void FEventSystem::Update(Float I_DeltaTime)
    {
        // Process queued events each frame
        ProcessEvents();

        // Update statistics
        Stats.QueuedEventCount = static_cast<UInt32>(QueuedEvents.size());

        UInt32 totalSubscribers = 0;
        {
            std::shared_lock lock(EventMutex);
            for (const auto& [EventType, Callbacks] : EventCallbacks)
            {
                totalSubscribers += Callbacks.GetBoundCount();
            }
        }
        Stats.SubscriberCount = totalSubscribers;
    }

    void FEventSystem::Terminate()
    {
        LOG_TRACE("Terminating Event System...");

        Shutdown();
        Status = EStatues::Terminated;

        LOG_DEBUG("Event System terminated successfully.");
    }

    void FEventSystem::Shutdown()
    {
        LOG_DEBUG("Shutting down Event System...");

        // Clear all subscriptions and queued events
        ClearAllSubscriptions();

        {
            std::unique_lock lock(EventMutex);
            QueuedEvents.clear();
        }

        LOG_DEBUG("Event System shutdown complete.");
    }

    FDelegateHandle FEventSystem::Subscribe(const FString& EventType,
        std::function<void(const FEvent&)> Callback)
    {
        std::unique_lock lock(EventMutex);

        // Get or create the multicast delegate for this event type
        auto& EventDelegate = EventCallbacks[EventType];

        // Add the callback as a lambda binding
        FDelegateHandle Handle = EventDelegate.AddLambda(std::move(Callback));

        LOG_DEBUG("Subscribed to event '{}' with handle {}", EventType, Handle);
        return Handle;
    }

    Bool FEventSystem::Unsubscribe(const FString& EventType, FDelegateHandle Handle)
    {
        std::unique_lock lock(EventMutex);

        auto It = EventCallbacks.find(EventType);
        if (It != EventCallbacks.end())
        {
            Bool Removed = It->second.Remove(Handle);
            if (Removed)
            {
                LOG_DEBUG("Unsubscribed from event '{}' with handle {}", EventType, Handle);

                // Clean up empty event delegates
                if (!It->second.IsBound())
                {
                    EventCallbacks.erase(It);
                    LOG_DEBUG("Removed empty event delegate for '{}'", EventType);
                }
            }
            return Removed;
        }

        LOG_WARN("Attempted to unsubscribe from non-existent event type '{}'", EventType);
        return false;
    }

    void FEventSystem::Publish(TSharedPtr<FEvent> Event)
    {
        if (!Event)
        {
            LOG_ERROR("Attempted to publish null event");
            return;
        }

        {
            std::unique_lock lock(EventMutex);
            QueuedEvents.push_back(Event);
        }

        ++Stats.TotalEventsPublished;
        LOG_DEBUG("Queued event '{}' for processing", Event->GetEventType());
    }

    void FEventSystem::PublishImmediate(TSharedPtr<FEvent> Event)
    {
        if (!Event)
        {
            LOG_ERROR("Attempted to publish null event immediately");
            return;
        }

        ProcessEventInternal(*Event);
        ++Stats.TotalEventsPublished;
        ++Stats.TotalEventsProcessed;

        LOG_DEBUG("Published event '{}' immediately", Event->GetEventType());
    }

    void FEventSystem::ProcessEvents()
    {
        TArray<TSharedPtr<FEvent>> EventsToProcess;

        // Move queued events to local array for processing
        {
            std::unique_lock lock(EventMutex);
            EventsToProcess = std::move(QueuedEvents);
            QueuedEvents.clear();
        }

        // Process events without holding the lock
        for (const auto& Event : EventsToProcess)
        {
            if (Event)
            {
                ProcessEventInternal(*Event);
                ++Stats.TotalEventsProcessed;
            }
        }

        if (!EventsToProcess.empty())
        {
            LOG_DEBUG("Processed {} events", EventsToProcess.size());
        }
    }

    void FEventSystem::ClearAllSubscriptions()
    {
        std::unique_lock lock(EventMutex);

        UInt32 ClearedEventTypes = static_cast<UInt32>(EventCallbacks.size());
        EventCallbacks.clear();

        LOG_DEBUG("Cleared all event subscriptions ({} event types)", ClearedEventTypes);
    }

    void FEventSystem::ProcessEventInternal(const FEvent& Event)
    {
        std::shared_lock lock(EventMutex);

        auto It = EventCallbacks.find(Event.GetEventType());
        if (It != EventCallbacks.end())
        {
            // Broadcast to all subscribers
            It->second.Broadcast(Event);
        }
    }

}