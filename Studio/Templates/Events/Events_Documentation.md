# Visera UE5-Style Event System

## 📋 Overview

The Visera event system implements a **UE5-inspired delegate and event system** using modern C++23 features. It provides type-safe, high-performance event handling perfect for game engines and real-time applications.

## 🏗️ Architecture

### **Core Components**

1. **TDelegate<Args...>** - Single-cast delegate (like UE5's `DECLARE_DELEGATE`)
2. **TMulticastDelegate<Args...>** - Multi-cast delegate (like UE5's `DECLARE_MULTICAST_DELEGATE`)
3. **FEventSystem** - Global event dispatcher for named events
4. **FEventContext** - Global access point for the event system

### **Design Principles**

- ✅ **Type Safety**: Compile-time type checking
- ✅ **Performance**: Direct function calls, minimal overhead
- ✅ **Memory Safety**: Automatic cleanup with weak_ptr for object references
- ✅ **Modern C++**: Uses concepts, templates, and RAII
- ✅ **Engine-Friendly**: Patterns familiar to UE5 developers

## 🚀 Usage Examples

### **1. Basic Delegate Usage**

```cpp
// Declare a delegate type
DECLARE_DELEGATE_TwoParams(FOnHealthChanged, Float, Float);

class FPlayer
{
public:
    FOnHealthChanged OnHealthChanged;
    
    void TakeDamage(Float Damage)
    {
        Health -= Damage;
        OnHealthChanged.ExecuteIfBound(Health, MaxHealth);
    }
    
private:
    Float Health = 100.0f;
    Float MaxHealth = 100.0f;
};

// Bind and use
FPlayer Player;
Player.OnHealthChanged.BindLambda([](Float Health, Float MaxHealth)
{
    LOG_INFO("Health: {}/{}", Health, MaxHealth);
});
```

### **2. Multicast Delegates**

```cpp
// Declare multicast delegate
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPositionChanged, const FVector3&);

class FGameObject
{
public:
    FOnPositionChanged OnPositionChanged;
    
    void SetPosition(const FVector3& NewPos)
    {
        Position = NewPos;
        OnPositionChanged.Broadcast(Position); // Notify all subscribers
    }
};

// Multiple subscribers
FGameObject Object;

// UI System subscribes
Object.OnPositionChanged.AddLambda([](const FVector3& Pos)
{
    UpdateUI(Pos);
});

// Audio system subscribes
Object.OnPositionChanged.AddLambda([](const FVector3& Pos)
{
    UpdateSpatialAudio(Pos);
});
```

### **3. Global Event System**

```cpp
// Subscribe to events
FEventContext::SubscribeToEvent("Player.Died", [](const FEvent& Event)
{
    LOG_INFO("Game Over!");
});

// Publish events
FEventContext::PublishEvent("Player.Died", PlayerData{PlayerID, Cause});
```

### **4. Object-Safe Binding**

```cpp
class FUIManager
{
public:
    void BindToPlayer(TSharedPtr<FPlayer> Player)
    {
        // Automatically cleaned up when Player is destroyed
        Player->OnHealthChanged.AddUObject<FUIManager, &FUIManager::UpdateHealthBar>(
            shared_from_this());
    }
    
    void UpdateHealthBar(Float Health, Float MaxHealth)
    {
        // Update UI safely
    }
};
```

## 🔧 Binding Types

### **1. Static Functions**

```cpp
void GlobalHealthCallback(Float Health, Float MaxHealth)
{
    LOG_INFO("Global health update: {}/{}", Health, MaxHealth);
}

// Bind static function
OnHealthChanged.BindStatic<&GlobalHealthCallback>();
```

### **2. Member Functions**

```cpp
class FHealthDisplay
{
public:
    void UpdateDisplay(Float Health, Float MaxHealth) { /* ... */ }
};

auto Display = MakeShared<FHealthDisplay>();
OnHealthChanged.BindUObject<FHealthDisplay, &FHealthDisplay::UpdateDisplay>(Display);
```

### **3. Lambda Functions**

```cpp
OnHealthChanged.BindLambda([this](Float Health, Float MaxHealth)
{
    if (Health <= 0.0f)
    {
        TriggerGameOver();
    }
});
```

## 🎯 Advanced Features

### **1. Event Filtering**

```cpp
// Subscribe with filtering
FEventContext::SubscribeToEvent("Input.KeyPressed", [](const FEvent& Event)
{
    if (auto KeyEvent = Event.As<TEvent<FKeyEventData>>())
    {
        if (KeyEvent->GetData().KeyCode == KEY_ESCAPE)
        {
            QuitGame();
        }
    }
});
```

### **2. Event Queuing**

```cpp
// Queue event for next frame
FEventContext::PublishEvent("DelayedAction", ActionData);

// Process immediately
auto EventSystem = FEventContext::GetEventSubsystem();
EventSystem->PublishImmediate(MakeShared<TEvent<ActionData>>("ImmediateAction", Data));
```

### **3. Cleanup and Management**

```cpp
// Remove specific binding
FDelegateHandle Handle = OnHealthChanged.AddLambda([](Float, Float){});
OnHealthChanged.Remove(Handle);

// Remove all bindings from object
OnHealthChanged.RemoveAll(SharedObjectPtr);

// Clear all bindings
OnHealthChanged.Clear();
```

## 🔄 Integration with Visera Architecture

### **Engine Integration**

```cpp
// In Engine layer
class FGameEngine
{
public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnFrameBegin, Float); // DeltaTime
    FOnFrameBegin OnFrameBegin;
    
    void Update(Float DeltaTime)
    {
        OnFrameBegin.Broadcast(DeltaTime);
        // ... engine update logic
    }
};
```

### **Studio Integration**

```cpp
// In Studio layer
class FStudioEditor
{
public:
    void Initialize()
    {
        // Subscribe to engine events through Runtime
        FEventContext::SubscribeToEvent(EventTypes::FrameBegin, 
            [this](const FEvent& Event) { UpdateEditor(Event); });
    }
    
private:
    void UpdateEditor(const FEvent& Event) { /* Update UI */ }
};
```

### **Runtime Mediation**

```cpp
// Engine and Studio communicate through Runtime events
// Engine publishes:
FEventContext::PublishEvent("Scene.ObjectSelected", ObjectID);

// Studio receives:
FEventContext::SubscribeToEvent("Scene.ObjectSelected", [](const FEvent& Event)
{
    // Update property panel
});
```

## 📊 Performance Characteristics

- **Direct Function Calls**: No virtual dispatch in hot paths
- **Memory Efficient**: Minimal allocation overhead
- **Type Safe**: Zero runtime type errors
- **Cache Friendly**: Contiguous storage for bindings
- **Thread Safe**: Proper synchronization for event queues

## 🆚 Comparison with Other Patterns

| Pattern | Performance | Type Safety | Ease of Use | Memory Safety |
|---------|-------------|-------------|-------------|---------------|
| **UE5 Delegates** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| Qt Signals/Slots | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| std::function | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| Observer Pattern | ⭐⭐ | ⭐⭐ | ⭐⭐ | ⭐⭐ |

## 🚀 Best Practices

### **1. Naming Conventions**
- Events: `EventTypes::ObjectDestroyed`
- Delegates: `FOnHealthChanged`
- Handlers: `HandleHealthChanged`

### **2. Memory Management**
```cpp
// ✅ Good - Use weak_ptr for object references
Object->OnDestroyed.AddLambda([WeakSelf = std::weak_ptr(this)](auto* Obj)
{
    if (auto Self = WeakSelf.lock()) { Self->HandleDestruction(); }
});

// ❌ Bad - Raw this pointer can dangle
Object->OnDestroyed.AddLambda([this](auto* Obj) { HandleDestruction(); });
```

### **3. Event Design**
```cpp
// ✅ Good - Specific, typed events
struct FHealthChangeData { Float OldHealth; Float NewHealth; };
FEventContext::PublishEvent("Player.HealthChanged", FHealthChangeData{100.0f, 75.0f});

// ❌ Bad - Generic, untyped events
FEventContext::PublishEvent("GenericUpdate", "health 75");
```

## 🔮 Future Enhancements

1. **Event Recording/Replay** for debugging
2. **Performance Profiling** integration
3. **Cross-Thread Events** with proper synchronization
4. **Event Validation** in debug builds
5. **Hot-Reload** support for delegate bindings

The Visera event system provides a solid foundation for complex game engine communication while maintaining the performance and safety requirements of modern C++ development! 🎯