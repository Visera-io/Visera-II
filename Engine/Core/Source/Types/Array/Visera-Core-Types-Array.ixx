module;
#include <Visera-Core.hpp>
#include <vector>
#include <memory_resource>
export module Visera.Core.Types.Array;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Memory;

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TArray
    {
    public:
        using ValueType             = T;
        using ArrayType             = std::vector<T>;
        using Iterator              = ArrayType::iterator;
        using ConstIterator         = ArrayType::const_iterator;
        using ReverseIterator       = ArrayType::reverse_iterator;
        using ConstReverseIterator  = ArrayType::const_reverse_iterator;
        using Reference             = ArrayType::reference;
        using ConstReference        = ArrayType::const_reference;
        using SizeType              = ArrayType::size_type;

    private:
        ArrayType Array;

    public:
        // Constructors and Destructor
        TArray() = default;
        ~TArray() = default;

        explicit TArray(SizeType I_Count)
            : Array(I_Count)
        {
        }

        TArray(SizeType I_Count, const T& I_Value)
            requires std::copy_constructible<T>
            : Array(I_Count, I_Value)
        {
        }

        template<typename InputIt>
        TArray(InputIt I_First, InputIt I_Last)
            : Array(I_First, I_Last)
        {
        }

        TArray(std::initializer_list<T> I_Init) requires std::copy_constructible<T>
            : Array(I_Init)
        {
        }

        // Copy constructor: only if T is copy constructible
        TArray(const TArray& I_Other) 
            requires std::copy_constructible<T>
            : Array(I_Other.Array)
        {
        }
        TArray(const TArray&) 
            requires (!std::copy_constructible<T>)
            = delete;
        
        // Copy assignment: only if T is copyable (both copy constructible and copy assignable)
        TArray& operator=(const TArray& I_Other) 
            requires (std::copy_constructible<T> && std::is_copy_assignable_v<T>)
        {
            if (this != &I_Other)
            {
                Array = I_Other.Array;
            }
            return *this;
        }
        TArray& operator=(const TArray&) 
            requires (!(std::copy_constructible<T> && std::is_copy_assignable_v<T>))
            = delete;
        
        // Move constructor: always available
        TArray(TArray&& I_Other) noexcept = default;
        
        // Move assignment: always available  
        TArray& operator=(TArray&& I_Other) noexcept = default;

        TArray& operator=(std::initializer_list<T> I_Init)
            requires std::is_copy_constructible_v<T>
        {
            Array = I_Init;
            return *this;
        }

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return Array.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return static_cast<UInt64>(Array.size());
        }

        [[nodiscard]] UInt64 GetCapacity() const
        {
            return static_cast<UInt64>(Array.capacity());
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return static_cast<UInt64>(Array.max_size());
        }

        void Reserve(SizeType I_NewCapacity)
        {
            Array.reserve(I_NewCapacity);
        }

        void Resize(SizeType I_NewSize)
        {
            Array.resize(I_NewSize);
        }

        void Resize(SizeType I_NewSize, const T& I_Value)
            requires std::copy_constructible<T>
        {
            Array.resize(I_NewSize, I_Value);
        }

        void ShrinkToFit()
        {
            Array.shrink_to_fit();
        }

        // Element access
        [[nodiscard]] T& operator[](SizeType I_Index)
        {
            return Array[I_Index];
        }

        [[nodiscard]] const T& operator[](SizeType I_Index) const
        {
            return Array[I_Index];
        }

        [[nodiscard]] T& At(SizeType I_Index)
        {
            return Array.at(I_Index);
        }

        [[nodiscard]] const T& At(SizeType I_Index) const
        {
            return Array.at(I_Index);
        }

        [[nodiscard]] T& Front()
        {
            return Array.front();
        }

        [[nodiscard]] const T& Front() const
        {
            return Array.front();
        }

        [[nodiscard]] T& Back()
        {
            return Array.back();
        }

        [[nodiscard]] const T& Back() const
        {
            return Array.back();
        }

        [[nodiscard]] T* Data()
        {
            return Array.data();
        }

        [[nodiscard]] const T* Data() const
        {
            return Array.data();
        }

        // Iterators
        [[nodiscard]] Iterator begin()
        {
            return Array.begin();
        }

        [[nodiscard]] ConstIterator begin() const
        {
            return Array.begin();
        }

        [[nodiscard]] ConstIterator cbegin() const
        {
            return Array.cbegin();
        }

        [[nodiscard]] Iterator end()
        {
            return Array.end();
        }

        [[nodiscard]] ConstIterator end() const
        {
            return Array.end();
        }

        [[nodiscard]] ConstIterator cend() const
        {
            return Array.cend();
        }

        [[nodiscard]] ReverseIterator rbegin()
        {
            return Array.rbegin();
        }

        [[nodiscard]] ConstReverseIterator rbegin() const
        {
            return Array.rbegin();
        }

        [[nodiscard]] ConstReverseIterator crbegin() const
        {
            return Array.crbegin();
        }

        [[nodiscard]] ReverseIterator rend()
        {
            return Array.rend();
        }

        [[nodiscard]] ConstReverseIterator rend() const
        {
            return Array.rend();
        }

        [[nodiscard]] ConstReverseIterator crend() const
        {
            return Array.crend();
        }

        // Modifiers
        void Clear()
        {
            Array.clear();
        }

        void PushBack(const T& I_Value)
            requires std::copy_constructible<T>
        {
            Array.push_back(I_Value);
        }

        void PushBack(T&& I_Value)
        {
            Array.push_back(std::move(I_Value));
        }

        template<typename... Args>
        T& EmplaceBack(Args&&... I_Args)
        {
            return Array.emplace_back(std::forward<Args>(I_Args)...);
        }

        void PopBack()
        {
            Array.pop_back();
        }

        Iterator Insert(ConstIterator I_Pos, const T& I_Value)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Value);
        }

        Iterator Insert(ConstIterator I_Pos, T&& I_Value)
        {
            return Array.insert(I_Pos, std::move(I_Value));
        }

        Iterator Insert(ConstIterator I_Pos, SizeType I_Count, const T& I_Value)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Count, I_Value);
        }

        template<typename InputIt>
        Iterator Insert(ConstIterator I_Pos, InputIt I_First, InputIt I_Last)
        {
            return Array.insert(I_Pos, I_First, I_Last);
        }

        Iterator Insert(ConstIterator I_Pos, std::initializer_list<T> I_Init)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Init);
        }

        Iterator Erase(ConstIterator I_Pos)
        {
            return Array.erase(I_Pos);
        }

        Iterator Erase(ConstIterator I_First, ConstIterator I_Last)
        {
            return Array.erase(I_First, I_Last);
        }

        void Swap(TArray& I_Other)
        {
            Array.swap(I_Other.Array);
        }

        Iterator RemoveAtSwap(Iterator I_Iterator) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            if (I_Iterator == Array.end()) return Array.end();
            auto Last = std::prev(Array.end());
            const Bool bRemovedLast = (I_Iterator == Last);
            if (!bRemovedLast)
            { *I_Iterator = std::move(*Last); }
            PopBack();
            return bRemovedLast ? Array.end() : I_Iterator;
        }

        void RemoveAtSwap(SizeType I_Index) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            const SizeType LastIndex = Array.size() - 1;
            if (I_Index != LastIndex)
            { Array[I_Index] = std::move(Array.back()); }

            PopBack();
        }

        template<typename... Args>
        T& Emplace(ConstIterator I_Pos, Args&&... I_Args)
        {
            return Array.emplace(I_Pos, std::forward<Args>(I_Args)...);
        }
    };

    template<typename T>
    class VISERA_CORE_API TPMRArray
    {
    public:
        using ValueType             = T;
        using ArrayType             = std::pmr::vector<T>;
        using Iterator              = ArrayType::iterator;
        using ConstIterator         = ArrayType::const_iterator;
        using ReverseIterator       = ArrayType::reverse_iterator;
        using ConstReverseIterator  = ArrayType::const_reverse_iterator;
        using Reference             = ArrayType::reference;
        using ConstReference        = ArrayType::const_reference;
        using SizeType              = ArrayType::size_type;

    private:
        ArrayType Array;

    public:
        // Constructors and Destructor
        explicit TPMRArray(std::pmr::memory_resource* I_Resource = std::pmr::get_default_resource())
            : Array(I_Resource)
        {
        }

        ~TPMRArray() = default;

        explicit TPMRArray(SizeType I_Count, std::pmr::memory_resource* I_Resource = std::pmr::get_default_resource())
            : Array(I_Count, I_Resource)
        {
        }

        TPMRArray(SizeType I_Count, const T& I_Value, std::pmr::memory_resource* I_Resource = std::pmr::get_default_resource())
            requires std::copy_constructible<T>
            : Array(I_Count, I_Value, I_Resource)
        {
        }

        template<typename InputIt>
        TPMRArray(InputIt I_First, InputIt I_Last, std::pmr::memory_resource* I_Resource = std::pmr::get_default_resource())
            : Array(I_First, I_Last, I_Resource)
        {
        }

        TPMRArray(std::initializer_list<T> I_Init, std::pmr::memory_resource* I_Resource = std::pmr::get_default_resource())
            requires std::copy_constructible<T>
            : Array(I_Init, I_Resource)
        {
        }

        // Copy constructor: only if T is copy constructible
        TPMRArray(const TPMRArray& I_Other)
            requires std::copy_constructible<T>
            : Array(I_Other.Array)
        {
        }
        TPMRArray(const TPMRArray&)
            requires (!std::copy_constructible<T>)
            = delete;
        
        // Copy assignment: only if T is copyable (both copy constructible and copy assignable)
        TPMRArray& operator=(const TPMRArray& I_Other)
            requires (std::copy_constructible<T> && std::is_copy_assignable_v<T>)
        {
            if (this != &I_Other)
            {
                Array = I_Other.Array;
            }
            return *this;
        }
        TPMRArray& operator=(const TPMRArray&)
            requires (!(std::copy_constructible<T> && std::is_copy_assignable_v<T>))
            = delete;
        
        // Move constructor: always available
        TPMRArray(TPMRArray&& I_Other) noexcept = default;
        
        // Move assignment: always available  
        TPMRArray& operator=(TPMRArray&& I_Other) noexcept = default;

        TPMRArray& operator=(std::initializer_list<T> I_Init)
            requires std::is_copy_constructible_v<T>
        {
            Array = I_Init;
            return *this;
        }

        // Capacity
        [[nodiscard]] Bool IsEmpty() const
        {
            return Array.empty();
        }

        [[nodiscard]] UInt64 GetSize() const
        {
            return static_cast<UInt64>(Array.size());
        }

        [[nodiscard]] UInt64 GetCapacity() const
        {
            return static_cast<UInt64>(Array.capacity());
        }

        [[nodiscard]] UInt64 GetMaxSize() const
        {
            return static_cast<UInt64>(Array.max_size());
        }

        void Reserve(SizeType I_NewCapacity)
        {
            Array.reserve(I_NewCapacity);
        }

        void Resize(SizeType I_NewSize)
        {
            Array.resize(I_NewSize);
        }

        void Resize(SizeType I_NewSize, const T& I_Value)
            requires std::copy_constructible<T>
        {
            Array.resize(I_NewSize, I_Value);
        }

        void ShrinkToFit()
        {
            Array.shrink_to_fit();
        }

        // Element access
        [[nodiscard]] T& operator[](SizeType I_Index)
        {
            return Array[I_Index];
        }

        [[nodiscard]] const T& operator[](SizeType I_Index) const
        {
            return Array[I_Index];
        }

        [[nodiscard]] T& At(SizeType I_Index)
        {
            return Array.at(I_Index);
        }

        [[nodiscard]] const T& At(SizeType I_Index) const
        {
            return Array.at(I_Index);
        }

        [[nodiscard]] T& Front()
        {
            return Array.front();
        }

        [[nodiscard]] const T& Front() const
        {
            return Array.front();
        }

        [[nodiscard]] T& Back()
        {
            return Array.back();
        }

        [[nodiscard]] const T& Back() const
        {
            return Array.back();
        }

        [[nodiscard]] T* Data()
        {
            return Array.data();
        }

        [[nodiscard]] const T* Data() const
        {
            return Array.data();
        }

        // Iterators
        [[nodiscard]] Iterator begin()
        {
            return Array.begin();
        }

        [[nodiscard]] ConstIterator begin() const
        {
            return Array.begin();
        }

        [[nodiscard]] ConstIterator cbegin() const
        {
            return Array.cbegin();
        }

        [[nodiscard]] Iterator end()
        {
            return Array.end();
        }

        [[nodiscard]] ConstIterator end() const
        {
            return Array.end();
        }

        [[nodiscard]] ConstIterator cend() const
        {
            return Array.cend();
        }

        [[nodiscard]] ReverseIterator rbegin()
        {
            return Array.rbegin();
        }

        [[nodiscard]] ConstReverseIterator rbegin() const
        {
            return Array.rbegin();
        }

        [[nodiscard]] ConstReverseIterator crbegin() const
        {
            return Array.crbegin();
        }

        [[nodiscard]] ReverseIterator rend()
        {
            return Array.rend();
        }

        [[nodiscard]] ConstReverseIterator rend() const
        {
            return Array.rend();
        }

        [[nodiscard]] ConstReverseIterator crend() const
        {
            return Array.crend();
        }

        // Modifiers
        void Clear()
        {
            Array.clear();
        }

        void PushBack(const T& I_Value)
            requires std::copy_constructible<T>
        {
            Array.push_back(I_Value);
        }

        void PushBack(T&& I_Value)
        {
            Array.push_back(std::move(I_Value));
        }

        template<typename... Args>
        T& EmplaceBack(Args&&... I_Args)
        {
            return Array.emplace_back(std::forward<Args>(I_Args)...);
        }

        void PopBack()
        {
            Array.pop_back();
        }

        Iterator Insert(ConstIterator I_Pos, const T& I_Value)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Value);
        }

        Iterator Insert(ConstIterator I_Pos, T&& I_Value)
        {
            return Array.insert(I_Pos, std::move(I_Value));
        }

        Iterator Insert(ConstIterator I_Pos, SizeType I_Count, const T& I_Value)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Count, I_Value);
        }

        template<typename InputIt>
        Iterator Insert(ConstIterator I_Pos, InputIt I_First, InputIt I_Last)
        {
            return Array.insert(I_Pos, I_First, I_Last);
        }

        Iterator Insert(ConstIterator I_Pos, std::initializer_list<T> I_Init)
            requires std::copy_constructible<T>
        {
            return Array.insert(I_Pos, I_Init);
        }

        Iterator Erase(ConstIterator I_Pos)
        {
            return Array.erase(I_Pos);
        }

        Iterator Erase(ConstIterator I_First, ConstIterator I_Last)
        {
            return Array.erase(I_First, I_Last);
        }

        void Swap(TPMRArray& I_Other)
        {
            Array.swap(I_Other.Array);
        }

        Iterator RemoveAtSwap(Iterator I_Iterator) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            if (I_Iterator == Array.end()) return Array.end();
            auto Last = std::prev(Array.end());
            const Bool bRemovedLast = (I_Iterator == Last);
            if (!bRemovedLast)
            { *I_Iterator = std::move(*Last); }
            PopBack();
            return bRemovedLast ? Array.end() : I_Iterator;
        }

        void RemoveAtSwap(SizeType I_Index) requires (std::movable<T> && std::assignable_from<T&, T>)
        {
            const SizeType LastIndex = Array.size() - 1;
            if (I_Index != LastIndex)
            { Array[I_Index] = std::move(Array.back()); }

            PopBack();
        }

        template<typename... Args>
        T& Emplace(ConstIterator I_Pos, Args&&... I_Args)
        {
            return Array.emplace(I_Pos, std::forward<Args>(I_Args)...);
        }
    };
}