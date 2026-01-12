module;
#include <Visera-Core.hpp>
#include <vector>
export module Visera.Core.Types.Array;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.OS.Memory;

export namespace Visera
{
    template<typename T>
    class VISERA_CORE_API TArray
    {
    public:
        using ValueType = T;
        using ArrayType = std::vector<T>;
        using Iterator = typename ArrayType::iterator;
        using ConstIterator = typename ArrayType::const_iterator;
        using ReverseIterator = typename ArrayType::reverse_iterator;
        using ConstReverseIterator = typename ArrayType::const_reverse_iterator;
        using Reference = typename ArrayType::reference;
        using ConstReference = typename ArrayType::const_reference;
        using SizeType = typename ArrayType::size_type;

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
            : Array(I_Count, I_Value)
        {
        }

        template<typename InputIt>
        TArray(InputIt I_First, InputIt I_Last)
            : Array(I_First, I_Last)
        {
        }

        TArray(std::initializer_list<T> I_Init)
            : Array(I_Init)
        {
        }

        // Copy constructor
        TArray(const TArray& I_Other) = default;
        
        // Move constructor
        TArray(TArray&& I_Other) noexcept = default;

        // Copy assignment
        TArray& operator=(const TArray& I_Other) = default;

        // Move assignment
        TArray& operator=(TArray&& I_Other) noexcept = default;

        TArray& operator=(std::initializer_list<T> I_Init)
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
        {
            return Array.insert(I_Pos, I_Value);
        }

        Iterator Insert(ConstIterator I_Pos, T&& I_Value)
        {
            return Array.insert(I_Pos, std::move(I_Value));
        }

        Iterator Insert(ConstIterator I_Pos, SizeType I_Count, const T& I_Value)
        {
            return Array.insert(I_Pos, I_Count, I_Value);
        }

        template<typename InputIt>
        Iterator Insert(ConstIterator I_Pos, InputIt I_First, InputIt I_Last)
        {
            return Array.insert(I_Pos, I_First, I_Last);
        }

        Iterator Insert(ConstIterator I_Pos, std::initializer_list<T> I_Init)
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

        template<typename... Args>
        T& Emplace(ConstIterator I_Pos, Args&&... I_Args)
        {
            return *Array.emplace(I_Pos, std::forward<Args>(I_Args)...);
        }
    };

    template<typename T>
    using TPMRArray = std::pmr::vector<T>;
}