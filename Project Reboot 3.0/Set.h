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

template<typename InElementType> //, typename KeyFuncs, typename Allocator>
class TSet
{
public:
    typedef TSetElement<InElementType> ElementType;
    typedef TSparseArrayElementOrListLink<ElementType> ArrayElementType;

    TSparseArray<ElementType> Elements;

    mutable TInlineAllocator<1>::ForElementType<int> Hash;
    mutable int32 HashSize;
};