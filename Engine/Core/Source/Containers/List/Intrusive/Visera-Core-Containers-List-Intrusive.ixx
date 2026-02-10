module;
#include <Visera-Core.hpp>
export module Visera.Core.Containers.List:Intrusive;
#define VISERA_MODULE_NAME "Core.Containers"

export namespace Visera
{
    template <class NodeType>
    class TIntrusiveDoubleLinkedListIterator;

    template <class ElementType, class ContainerType>
    class TIntrusiveDoubleLinkedList;

    /**
     * Node of an intrusive double linked list.
     * Structs/classes must inherit this to use it, e.g:
     *   struct FMyStruct : public TIntrusiveDoubleLinkedListNode<FMyStruct>
     *
     * TIntrusiveDoubleLinkedListNode can be inherited multiple times if ElementType
     * needs to be stored in several lists at once, by specifying a different
     * ContainerType template parameter to distinguish the nodes.
     */
    template <class InElementType, class ContainerType = InElementType>
    class VISERA_CORE_API TIntrusiveDoubleLinkedListNode
    {
    public:
        using NodeType     = TIntrusiveDoubleLinkedListNode<InElementType, ContainerType>;
        using ElementType  = InElementType;

        [[nodiscard]] TIntrusiveDoubleLinkedListNode()
            : Next(GetThisElement())
            , Prev(GetThisElement())
        {
        }

        VISERA_FORCEINLINE void Reset()
        {
            Next = Prev = GetThisElement();
        }

        [[nodiscard]] VISERA_FORCEINLINE Bool IsInList() const
        {
            return Next != GetThisElement();
        }

        [[nodiscard]] VISERA_FORCEINLINE ElementType* GetNext() const
        {
            return Next;
        }

        [[nodiscard]] VISERA_FORCEINLINE ElementType* GetPrev() const
        {
            return Prev;
        }

        /** Removes this element from the list in constant time. */
        void Remove()
        {
            static_cast<NodeType*>(Next)->Prev = Prev;
            static_cast<NodeType*>(Prev)->Next = Next;
            Next = Prev = GetThisElement();
        }

        /** Inserts this node after the specified node. */
        void InsertAfter(ElementType* I_NewPrev)
        {
            ElementType* NewNext = static_cast<NodeType*>(I_NewPrev)->Next;
            Next                 = NewNext;
            Prev                 = I_NewPrev;
            static_cast<NodeType*>(NewNext)->Prev = GetThisElement();
            static_cast<NodeType*>(I_NewPrev)->Next = GetThisElement();
        }

        /** Inserts this node before the specified node. */
        void InsertBefore(ElementType* I_NewNext)
        {
            ElementType* NewPrev = static_cast<NodeType*>(I_NewNext)->Prev;
            Next                 = I_NewNext;
            Prev                 = NewPrev;
            static_cast<NodeType*>(I_NewNext)->Prev = GetThisElement();
            static_cast<NodeType*>(NewPrev)->Next   = GetThisElement();
        }

    protected:
        friend class TIntrusiveDoubleLinkedListIterator<TIntrusiveDoubleLinkedListNode>;
        friend class TIntrusiveDoubleLinkedList<ElementType, ContainerType>;

        [[nodiscard]] VISERA_FORCEINLINE ElementType*       GetThisElement()       { return static_cast<ElementType*>(this); }
        [[nodiscard]] VISERA_FORCEINLINE const ElementType* GetThisElement() const { return static_cast<const ElementType*>(this); }

        ElementType* Next;
        ElementType* Prev;
    };

    /**
     * Iterator for intrusive double linked list.
     */
    template <class NodeType>
    class VISERA_CORE_API TIntrusiveDoubleLinkedListIterator
    {
    public:
        using ElementType  = typename NodeType::ElementType;
        using PointerType  = std::conditional_t<std::is_const_v<NodeType>, const ElementType*, ElementType*>;
        using ReferenceType = std::conditional_t<std::is_const_v<NodeType>, const ElementType&, ElementType&>;

        [[nodiscard]] VISERA_FORCEINLINE explicit TIntrusiveDoubleLinkedListIterator(PointerType I_Node)
            : CurrentNode(I_Node)
        {
        }

        TIntrusiveDoubleLinkedListIterator& operator++()
        {
            VISERA_ASSERT(CurrentNode);
            CurrentNode = static_cast<PointerType>(CurrentNode->NodeType::Next);
            return *this;
        }

        TIntrusiveDoubleLinkedListIterator operator++(int)
        {
            auto Tmp = *this;
            ++(*this);
            return Tmp;
        }

        TIntrusiveDoubleLinkedListIterator& operator--()
        {
            VISERA_ASSERT(CurrentNode);
            CurrentNode = static_cast<PointerType>(CurrentNode->NodeType::Prev);
            return *this;
        }

        TIntrusiveDoubleLinkedListIterator operator--(int)
        {
            auto Tmp = *this;
            --(*this);
            return Tmp;
        }

        [[nodiscard]] PointerType operator->() const
        {
            VISERA_ASSERT(CurrentNode);
            return CurrentNode;
        }

        [[nodiscard]] ReferenceType operator*() const
        {
            VISERA_ASSERT(CurrentNode);
            return *CurrentNode;
        }

        [[nodiscard]] PointerType GetNode() const
        {
            VISERA_ASSERT(CurrentNode);
            return CurrentNode;
        }

        [[nodiscard]] VISERA_FORCEINLINE Bool operator==(const TIntrusiveDoubleLinkedListIterator& I_Other) const
        {
            return CurrentNode == I_Other.CurrentNode;
        }

        [[nodiscard]] VISERA_FORCEINLINE Bool operator!=(const TIntrusiveDoubleLinkedListIterator& I_Other) const
        {
            return CurrentNode != I_Other.CurrentNode;
        }

    private:
        PointerType CurrentNode;
    };

    /**
     * Intrusive double linked list.
     */
    template <class InElementType, class ContainerType = InElementType>
    class VISERA_CORE_API TIntrusiveDoubleLinkedList
    {
    public:
        using ElementType = InElementType;
        using NodeType    = TIntrusiveDoubleLinkedListNode<ElementType, ContainerType>;

        [[nodiscard]] VISERA_FORCEINLINE TIntrusiveDoubleLinkedList() = default;

        TIntrusiveDoubleLinkedList(const TIntrusiveDoubleLinkedList&)            = delete;
        TIntrusiveDoubleLinkedList& operator=(const TIntrusiveDoubleLinkedList&) = delete;

        /**
         * Fast empty that clears this list without changing the links in any elements.
         */
        VISERA_FORCEINLINE void Reset()
        {
            Sentinel.Reset();
        }

        [[nodiscard]] VISERA_FORCEINLINE Bool IsEmpty() const
        {
            return Sentinel.Next == GetSentinel();
        }

        [[nodiscard]] VISERA_FORCEINLINE Bool IsFilled() const
        {
            return Sentinel.Next != GetSentinel();
        }

        VISERA_FORCEINLINE void AddHead(ElementType* I_Element)
        {
            static_cast<NodeType*>(I_Element)->InsertAfter(GetSentinel());
        }

        void AddHead(TIntrusiveDoubleLinkedList&& I_Other)
        {
            if (I_Other.IsFilled())
            {
                static_cast<NodeType*>(I_Other.Sentinel.Prev)->Next = Sentinel.Next;
                static_cast<NodeType*>(I_Other.Sentinel.Next)->Prev = GetSentinel();
                static_cast<NodeType*>(Sentinel.Next)->Prev         = I_Other.Sentinel.Prev;
                Sentinel.Next                                     = I_Other.Sentinel.Next;
                I_Other.Sentinel.Next = I_Other.Sentinel.Prev      = I_Other.GetSentinel();
            }
        }

        VISERA_FORCEINLINE void AddTail(ElementType* I_Element)
        {
            static_cast<NodeType*>(I_Element)->InsertBefore(GetSentinel());
        }

        void AddTail(TIntrusiveDoubleLinkedList&& I_Other)
        {
            if (I_Other.IsFilled())
            {
                static_cast<NodeType*>(I_Other.Sentinel.Next)->Prev = Sentinel.Prev;
                static_cast<NodeType*>(I_Other.Sentinel.Prev)->Next = GetSentinel();
                static_cast<NodeType*>(Sentinel.Prev)->Next        = I_Other.Sentinel.Next;
                Sentinel.Prev                                     = I_Other.Sentinel.Prev;
                I_Other.Sentinel.Next = I_Other.Sentinel.Prev     = I_Other.GetSentinel();
            }
        }

        [[nodiscard]] VISERA_FORCEINLINE ElementType* GetHead()
        {
            return IsFilled() ? Sentinel.Next : nullptr;
        }

        [[nodiscard]] VISERA_FORCEINLINE ElementType* GetTail()
        {
            return IsFilled() ? Sentinel.Prev : nullptr;
        }

        [[nodiscard]] ElementType* PopHead()
        {
            if (IsEmpty())
            {
                return nullptr;
            }

            ElementType* Head = Sentinel.Next;
            static_cast<NodeType*>(Head)->Remove();
            return Head;
        }

        [[nodiscard]] ElementType* PopTail()
        {
            if (IsEmpty())
            {
                return nullptr;
            }

            ElementType* Tail = Sentinel.Prev;
            static_cast<NodeType*>(Tail)->Remove();
            return Tail;
        }

        static VISERA_FORCEINLINE void Remove(ElementType* I_Element)
        {
            static_cast<NodeType*>(I_Element)->Remove();
        }

        /** Removes I_Element from its current position and inserts at head. O(1). */
        VISERA_FORCEINLINE void MoveToHead(ElementType* I_Element)
        {
            static_cast<NodeType*>(I_Element)->Remove();
            AddHead(I_Element);
        }

        static VISERA_FORCEINLINE void InsertAfter(ElementType* I_InsertThis, ElementType* I_AfterThis)
        {
            static_cast<NodeType*>(I_InsertThis)->InsertAfter(I_AfterThis);
        }

        static VISERA_FORCEINLINE void InsertBefore(ElementType* I_InsertThis, ElementType* I_BeforeThis)
        {
            static_cast<NodeType*>(I_InsertThis)->InsertBefore(I_BeforeThis);
        }

        using TIterator      = TIntrusiveDoubleLinkedListIterator<NodeType>;
        using TConstIterator = TIntrusiveDoubleLinkedListIterator<const NodeType>;

        [[nodiscard]] VISERA_FORCEINLINE TIterator      begin()       { return TIterator(Sentinel.Next); }
        [[nodiscard]] VISERA_FORCEINLINE TConstIterator begin() const { return TConstIterator(Sentinel.Next); }
        [[nodiscard]] VISERA_FORCEINLINE TIterator      end()         { return TIterator(GetSentinel()); }
        [[nodiscard]] VISERA_FORCEINLINE TConstIterator end() const   { return TConstIterator(GetSentinel()); }

    private:
        [[nodiscard]] VISERA_FORCEINLINE ElementType*       GetSentinel()       { return static_cast<ElementType*>(&Sentinel); }
        [[nodiscard]] VISERA_FORCEINLINE const ElementType* GetSentinel() const { return static_cast<const ElementType*>(&Sentinel); }

        NodeType Sentinel;
    };
}
