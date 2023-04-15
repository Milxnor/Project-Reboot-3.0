#pragma once

#include "SparseArray.h"

template <typename ElementType>
class TSetElement
{
public:
    ElementType Value;
    mutable int32 HashNextId;
    mutable int32 HashIndex;

    TSetElement(ElementType InValue, int32 InHashNextId, int32 InHashIndex)
        : Value(InValue)
        , HashNextId(InHashNextId)
        , HashIndex(InHashIndex)
    {
    }

    FORCEINLINE TSetElement<ElementType>& operator=(const TSetElement<ElementType>& Other)
    {
        Value = Other.Value;
    }

    FORCEINLINE bool operator==(const TSetElement& Other) const
    {
        return Value == Other.Value;
    }
    FORCEINLINE bool operator!=(const TSetElement& Other) const
    {
        return Value != Other.Value;
    }
};

template <typename SetType>
class TSet
{
private:
    friend TSparseArray;

public:
    typedef TSetElement<SetType> ElementType;
    typedef TSparseArrayElementOrListLink<ElementType> ArrayElementType;

public:
    TSparseArray<ElementType> Elements;

    mutable TInlineAllocator<1>::ForElementType<int> Hash;
    mutable int32 HashSize;

public:
    class FBaseIterator
    {
    private:
        TSet<SetType>& IteratedSet;
        TSparseArray<ElementType>::FBaseIterator ElementIt;

    public:
        FORCEINLINE FBaseIterator(const TSet<SetType>& InSet, TSparseArray<TSetElement<SetType>>::FBaseIterator InElementIt)
            : IteratedSet(const_cast<TSet<SetType>&>(InSet))
            , ElementIt(InElementIt)
        {
        }

        FORCEINLINE explicit operator bool() const
        {
            return (bool)ElementIt;
        }
        FORCEINLINE TSet<SetType>::FBaseIterator& operator++()
        {
            ++ElementIt;
            return *this;
        }
        FORCEINLINE bool operator==(const TSet<SetType>::FBaseIterator& OtherIt) const
        {
            return ElementIt == OtherIt.ElementIt;
        }
        FORCEINLINE bool operator!=(const TSet<SetType>::FBaseIterator& OtherIt) const
        {
            return ElementIt != OtherIt.ElementIt;
        }
        FORCEINLINE TSet<SetType>::FBaseIterator& operator=(TSet<SetType>::FBaseIterator& OtherIt)
        {
            return ElementIt = OtherIt.ElementIt;
        }
        FORCEINLINE SetType& operator*()
        {
            return (*ElementIt).Value;
        }
        FORCEINLINE const SetType& operator*() const
        {
            return &((*ElementIt).Value);
        }
        FORCEINLINE SetType* operator->()
        {
            return &((*ElementIt).Value);
        }
        FORCEINLINE const SetType* operator->() const
        {
            return &(*ElementIt).Value;
        }
        FORCEINLINE const int32 GetIndex() const
        {
            return ElementIt.GetIndex();
        }
        FORCEINLINE ElementType& GetSetElement()
        {
            return *ElementIt;
        }
        FORCEINLINE const ElementType& GetSetElement() const
        {
            return *ElementIt;
        }
        FORCEINLINE bool IsElementValid() const
        {
            return ElementIt.IsElementValid();
        }
    };

public:
    FORCEINLINE TSet<SetType>::FBaseIterator begin()
    {
        return TSet<SetType>::FBaseIterator(*this, Elements.begin());
    }
    FORCEINLINE const TSet<SetType>::FBaseIterator begin() const
    {
        return TSet<SetType>::FBaseIterator(*this, Elements.begin());
    }
    FORCEINLINE TSet<SetType>::FBaseIterator end()
    {
        return TSet<SetType>::FBaseIterator(*this, Elements.end());
    }
    FORCEINLINE const TSet<SetType>::FBaseIterator end() const
    {
        return TSet<SetType>::FBaseIterator(*this, Elements.end());
    }

    FORCEINLINE SetType& operator[](int Index)
    {
        return Elements[Index].ElementData.Value;
    }

    FORCEINLINE int32 Num() const
    {
        return Elements.Num();
    }
    FORCEINLINE bool IsValid() const
    {
        return Elements.Data.Data != nullptr && Elements.AllocationFlags.MaxBits > 0;
    }
    FORCEINLINE TSparseArray<ElementType>& GetElements()
    {
        return Elements;
    }
    FORCEINLINE const TSparseArray<ElementType>& GetElements() const
    {
        return Elements;
    }
    FORCEINLINE const TBitArray& GetAllocationFlags() const
    {
        return Elements.GetAllocationFlags();
    }
    FORCEINLINE bool IsIndexValid(int32 IndexToCheck) const
    {
        return Elements.IsIndexValid(IndexToCheck);
    }
    FORCEINLINE const bool Contains(const SetType& ElementToLookFor) const
    {
        if (Num() <= 0)
            return false;

        for (SetType Element : *this)
        {
            if (Element == ElementToLookFor)
                return true;
        }
        return false;
    }
    FORCEINLINE const int32 Find(const SetType& ElementToLookFor) const
    {
        for (auto It = this->begin(); It != this->end(); ++It)
        {
            if (*It == ElementToLookFor)
            {
                return It.GetIndex();
            }
        }

        return -1;
    }
    FORCEINLINE bool Remove(const SetType& ElementToRemove)
    {
        auto Idx = Find(ElementToRemove);

        if (Idx == -1)
            return false;

        Elements.RemoveAt(Idx);
        return true;
    }
    FORCEINLINE bool Remove(int Index)
    {
        Elements.RemoveAt(Index);
        return true;
    }
};


/* template<typename InElementType> //, typename KeyFuncs, typename Allocator>
class TSet
{
public:
    typedef TSetElement<InElementType> ElementType;
    typedef TSparseArrayElementOrListLink<ElementType> ArrayElementType;

    TSparseArray<ElementType> Elements;

    mutable TInlineAllocator<1>::ForElementType<int> Hash;
    mutable int32 HashSize;
}; */