module;
#include <Visera-Core.hpp>
#include <queue>
export module Visera.Core.Types.Queue;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Memory;

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TQueue
    {
    public:
        using ValueType = T;
        using QueueType = std::queue<T>;

    private:
        QueueType Queue;

    public:
        // Constructors and Destructor
        TQueue() = default;
        ~TQueue() = default;

        // Copy constructor
        TQueue(const TQueue& I_Other) = default;
        
        // Move constructor
        TQueue(TQueue&& I_Other) noexcept = default;

        // Copy assignment
        TQueue& operator=(const TQueue& I_Other) = default;

        // Move assignment
        TQueue& operator=(TQueue&& I_Other) noexcept = default;

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return Queue.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return static_cast<UInt64>(Queue.size());
        }

        // Element access
        [[nodiscard]] T& Front()
        {
            return Queue.front();
        }

        [[nodiscard]] const T& Front() const
        {
            return Queue.front();
        }

        [[nodiscard]] T& Back()
        {
            return Queue.back();
        }

        [[nodiscard]] const T& Back() const
        {
            return Queue.back();
        }

        // Modifiers
        void Push(const T& I_Value)
        {
            Queue.push(I_Value);
        }

        void Push(T&& I_Value)
        {
            Queue.push(std::move(I_Value));
        }

        template<typename... Args>
        void Emplace(Args&&... I_Args)
        {
            Queue.emplace(std::forward<Args>(I_Args)...);
        }

        void Pop()
        {
            Queue.pop();
        }

        void Swap(TQueue& I_Other)
        {
            Queue.swap(I_Other.Queue);
        }

        void Clear()
        {
            while (!Queue.empty())
            {
                Queue.pop();
            }
        }
    };
}