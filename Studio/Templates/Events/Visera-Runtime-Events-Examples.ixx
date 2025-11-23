module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Event.Examples;
#define VISERA_MODULE_NAME "Runtime.Events.Examples"
import Visera.Runtime.Event.Delegate;
import Visera.Runtime.Event.MulticastDelegate;
import Visera.Runtime.Event.EventSystem;
import Visera.Core.Log;

namespace ViseraExamples
{
    /**
     * @brief Example game object class demonstrating event usage
     */
    export class VISERA_RUNTIME_API FGameObject
    {
    public:
        // UE5-style delegate declarations
        DECLARE_DELEGATE_OneParam(FOnDestroyed, FGameObject*);
        DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, Float, Float); // CurrentHealth, MaxHealth
        DECLARE_MULTICAST_DELEGATE_OneParam(FOnPositionChanged, const FVector3&);

        explicit FGameObject(const FString& I_Name) : Name(I_Name), Health(100.0f), MaxHealth(100.0f) {}

        // Delegate accessors (UE5 pattern)
        FOnDestroyed& OnDestroyed() { return DestroyedDelegate; }
        FOnHealthChanged& OnHealthChanged() { return HealthChangedDelegate; }
        FOnPositionChanged& OnPositionChanged() { return PositionChangedDelegate; }

        void TakeDamage(Float Damage)
        {
            Float OldHealth = Health;
            Health = std::max(0.0f, Health - Damage);
            
            // Broadcast health change event
            HealthChangedDelegate.Broadcast(Health, MaxHealth);
            
            LOG_INFO("{} took {} damage. Health: {}/{}", Name, Damage, Health, MaxHealth);
            
            if (Health <= 0.0f && OldHealth > 0.0f)
            {
                // Broadcast destruction event
                DestroyedDelegate.ExecuteIfBound(this);
                LOG_INFO("{} was destroyed!", Name);
            }
        }

        void SetPosition(const FVector3& NewPosition)
        {
            Position = NewPosition;
            PositionChangedDelegate.Broadcast(Position);
            LOG_DEBUG("{} moved to position ({}, {}, {})", 
                Name, Position.X, Position.Y, Position.Z);
        }

        const FString& GetName() const { return Name; }
        Float GetHealth() const { return Health; }
        const FVector3& GetPosition() const { return Position; }

    private:
        FString Name;
        Float Health;
        Float MaxHealth;
        FVector3 Position{0.0f, 0.0f, 0.0f};

        // Event delegates
        FOnDestroyed DestroyedDelegate;
        FOnHealthChanged HealthChangedDelegate;
        FOnPositionChanged PositionChangedDelegate;
    };

    /**
     * @brief Example UI system that responds to game events
     */
    export class VISERA_RUNTIME_API FUISystem
    {
    public:
        void Initialize()
        {
            // Subscribe to global events using the event system
            if (auto EventSystem = FEventContext::GetEventSubsystem())
            {
                EventSystem->Subscribe(EventTypes::FrameBegin, 
                    [this](const FEvent& Event) { OnFrameBegin(Event); });
                
                EventSystem->Subscribe("GameObject.Destroyed",
                    [this](const FEvent& Event) { OnGameObjectDestroyed(Event); });
            }
            
            LOG_INFO("UI System initialized and subscribed to events");
        }

        void BindToGameObject(TSharedPtr<FGameObject> GameObject)
        {
            if (!GameObject) return;

            // Bind to object-specific events using delegates
            GameObject->OnHealthChanged().AddLambda(
                [this, WeakObject = TWeakPtr<FGameObject>(GameObject)](Float Health, Float MaxHealth)
                {
                    if (auto Object = WeakObject.lock())
                    {
                        UpdateHealthBar(Object->GetName(), Health, MaxHealth);
                    }
                });

            GameObject->OnPositionChanged().AddLambda(
                [this, WeakObject = TWeakPtr<FGameObject>(GameObject)](const FVector3& Position)
                {
                    if (auto Object = WeakObject.lock())
                    {
                        UpdatePositionDisplay(Object->GetName(), Position);
                    }
                });

            GameObject->OnDestroyed().BindLambda(
                [this](FGameObject* DestroyedObject)
                {
                    RemoveObjectFromUI(DestroyedObject->GetName());
                });

            LOG_INFO("UI System bound to GameObject: {}", GameObject->GetName());
        }

    private:
        void OnFrameBegin(const FEvent& Event)
        {
            // Update UI each frame
            FrameCount++;
            if (FrameCount % 60 == 0) // Log every second at 60 FPS
            {
                LOG_DEBUG("UI Frame update #{}", FrameCount);
            }
        }

        void OnGameObjectDestroyed(const FEvent& Event)
        {
            // Handle global destruction events
            LOG_INFO("UI received global destruction notification");
        }

        void UpdateHealthBar(const FString& ObjectName, Float Health, Float MaxHealth)
        {
            Float HealthPercent = MaxHealth > 0 ? (Health / MaxHealth) * 100.0f : 0.0f;
            LOG_INFO("UI: {} health bar updated to {:.1f}%", ObjectName, HealthPercent);
        }

        void UpdatePositionDisplay(const FString& ObjectName, const FVector3& Position)
        {
            LOG_DEBUG("UI: {} position updated to ({:.2f}, {:.2f}, {:.2f})", 
                ObjectName, Position.X, Position.Y, Position.Z);
        }

        void RemoveObjectFromUI(const FString& ObjectName)
        {
            LOG_INFO("UI: Removing {} from display", ObjectName);
        }

        UInt64 FrameCount = 0;
    };

    /**
     * @brief Example demonstrating the complete event system usage
     */
    export class VISERA_RUNTIME_API FEventSystemDemo
    {
    public:
        void RunDemo()
        {
            LOG_INFO("=== Starting UE5-Style Event System Demo ===");

            // Initialize event system
            SetupEventSystem();

            // Create UI system
            UISystem = MakeShared<FUISystem>();
            UISystem->Initialize();

            // Create game objects
            auto Player = MakeShared<FGameObject>("Player");
            auto Enemy = MakeShared<FGameObject>("Enemy");

            // Bind UI to objects
            UISystem->BindToGameObject(Player);
            UISystem->BindToGameObject(Enemy);

            // Set up inter-object communication using static delegates
            Player->OnDestroyed().BindLambda([this](FGameObject* DestroyedPlayer)
            {
                LOG_INFO("Game Over! Player was destroyed.");
                PublishGameEvent("Game.Over", "Player died");
            });

            Enemy->OnDestroyed().BindLambda([this](FGameObject* DestroyedEnemy)
            {
                LOG_INFO("Victory! Enemy was destroyed.");
                PublishGameEvent("Game.Victory", "Enemy defeated");
            });

            // Simulate game events
            SimulateGameplay(Player, Enemy);

            LOG_INFO("=== Event System Demo Complete ===");
        }

    private:
        TSharedPtr<FUISystem> UISystem;
        TSharedPtr<FEventSystem> EventSystemInstance;

        void SetupEventSystem()
        {
            EventSystemInstance = MakeShared<FEventSystem>();
            EventSystemInstance->Initialize();
            EventSystemInstance->Bootstrap();
            
            // Set as global event system
            FEventContext::SetEventSubsystem(EventSystemInstance);
            
            LOG_INFO("Event system initialized and set as global instance");
        }

        void SimulateGameplay(TSharedPtr<FGameObject> Player, TSharedPtr<FGameObject> Enemy)
        {
            LOG_INFO("\n--- Simulating Gameplay ---");

            // Simulate frame updates
            for (int Frame = 0; Frame < 5; ++Frame)
            {
                PublishFrameEvent();
                EventSystemInstance->ProcessEvents();
                
                // Move objects
                Player->SetPosition(FVector3{static_cast<Float>(Frame), 0.0f, 0.0f});
                Enemy->SetPosition(FVector3{0.0f, static_cast<Float>(Frame), 0.0f});
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Combat simulation
            LOG_INFO("\n--- Combat Begins ---");
            Player->TakeDamage(30.0f);  // Player takes damage
            Enemy->TakeDamage(50.0f);   // Enemy takes damage
            Enemy->TakeDamage(60.0f);   // Enemy destroyed
            
            // Process any remaining events
            EventSystemInstance->ProcessEvents();
        }

        void PublishFrameEvent()
        {
            FEventContext::PublishEvent(EventTypes::FrameBegin, FString("Frame update"));
        }

        void PublishGameEvent(const FString& EventType, const FString& Message)
        {
            FEventContext::PublishEvent(EventType, Message);
        }
    };

    // Helper struct for vector operations
    struct FVector3
    {
        Float X, Y, Z;
        
        FVector3(Float InX = 0.0f, Float InY = 0.0f, Float InZ = 0.0f)
            : X(InX), Y(InY), Z(InZ) {}
    };

}

// UE5-style delegate macros for convenience
#define DECLARE_DELEGATE(DelegateName) \
    using DelegateName = Visera::TDelegate<>

#define DECLARE_DELEGATE_OneParam(DelegateName, Param1Type) \
    using DelegateName = Visera::TDelegate<Param1Type>

#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) \
    using DelegateName = Visera::TDelegate<Param1Type, Param2Type>

#define DECLARE_MULTICAST_DELEGATE(DelegateName) \
    using DelegateName = Visera::TMulticastDelegate<>

#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type) \
    using DelegateName = Visera::TMulticastDelegate<Param1Type>

#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) \
    using DelegateName = Visera::TMulticastDelegate<Param1Type, Param2Type>

#define DECLARE_MULTICAST_DELEGATE_ThreeParams(DelegateName, Param1Type, Param2Type, Param3Type) \
    using DelegateName = Visera::TMulticastDelegate<Param1Type, Param2Type, Param3Type>