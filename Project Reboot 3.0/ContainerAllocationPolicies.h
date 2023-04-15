#pragma once

#include "NumericLimits.h"

template <int NumElements>
class TInlineAllocator
{
private:
    template <int Size, int Alignment>
    struct alignas(Alignment) TAlignedBytes
    {
        unsigned char Pad[Size];
    };

    template <typename ElementType>
    struct TTypeCompatibleBytes : public TAlignedBytes<sizeof(ElementType), alignof(ElementType)>
    {
    };

public:
    template <typename ElementType>
    class ForElementType
    {
        friend class TBitArray;

    private:
        TTypeCompatibleBytes<ElementType> InlineData[NumElements];

        ElementType* SecondaryData;

    public:

        FORCEINLINE int32 NumInlineBytes() const
        {
            return sizeof(ElementType) * NumElements;
        }
        FORCEINLINE int32 NumInlineBits() const
        {
            return NumInlineBytes() * 8;
        }

        FORCEINLINE ElementType& operator[](int32 Index)
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE const ElementType& operator[](int32 Index) const
        {
            return *(ElementType*)(&InlineData[Index]);
        }

        FORCEINLINE void operator=(void* InElements)
        {
            SecondaryData = InElements;
        }

        FORCEINLINE ElementType& GetInlineElement(int32 Index)
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE const ElementType& GetInlineElement(int32 Index) const
        {
            return *(ElementType*)(&InlineData[Index]);
        }
        FORCEINLINE ElementType& GetSecondaryElement(int32 Index)
        {
            return SecondaryData[Index];
        }
        FORCEINLINE const ElementType& GetSecondaryElement(int32 Index) const
        {
            return SecondaryData[Index];
        }
        ElementType* GetInlineElements() const
        {
            return (ElementType*)InlineData;
        }
        FORCEINLINE ElementType* GetAllocation() const
        {
            return IfAThenAElseB<ElementType>(SecondaryData, GetInlineElements());
        }
    };
};


FORCEINLINE /*FMEMORY_INLINE_FUNCTION_DECORATOR*/ size_t /*FMemory::*/QuantizeSize(SIZE_T Count, uint32 Alignment)
{
    return Count;
    /*
    if (!FMEMORY_INLINE_GMalloc)
    {
        return Count;
    }
    return FMEMORY_INLINE_GMalloc->QuantizeSize(Count, Alignment); */
}

enum
{
    DEFAULT_ALIGNMENT = 0
};

template <typename SizeType>
FORCEINLINE SizeType DefaultCalculateSlackReserve(SizeType NumElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
{
    SizeType Retval = NumElements;
    // checkSlow(NumElements > 0);
    if (bAllowQuantize)
    {
        auto Count = SIZE_T(Retval) * SIZE_T(BytesPerElement);
        Retval = (SizeType)(QuantizeSize(Count, Alignment) / BytesPerElement);
        // NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
        if (NumElements > Retval)
        {
            Retval = TNumericLimits<SizeType>::Max();
        }
    }

    return Retval;
}