#pragma once

#include "NumericLimits.h"
#include "UnrealTemplate.h"
#include "TypeCompatibleBytes.h"

struct FMemory // so real place
{
    static inline void* (*Realloc)(void* Original, SIZE_T Count, uint32_t Alignment /* = DEFAULT_ALIGNMENT */);

    static void Free(void* Data)
    {
        // We could use actual free..

        Realloc(Data, 0, 0);
    }

    static size_t QuantizeSize(size_t Count, uint32 Alignment) // T(R)
    {
        return Count;
    }

    static void* Memmove(void* Dest, const void* Src, SIZE_T Count)
    {
        memmove(Dest, Src, Count);
    }

    static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
    {
        // return FPlatformMemory::Memzero(Dest, Count);
        return RtlSecureZeroMemory(Dest, Count);
    }

    template< class T >
    static FORCEINLINE void Memzero(T& Src)
    {
        static_assert(!TIsPointer<T>::Value, "For pointers use the two parameters function");
        Memzero(&Src, sizeof(T));
    }
};

template <typename T = __int64>
static T* AllocUnreal(size_t Size)
{
    return (T*)FMemory::Realloc(0, Size, 0);
}

enum
{
    DEFAULT_ALIGNMENT = 0
};

// 4.19

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE void RelocateConstructItems(void* Dest, const SourceElementType* Source, SizeType Count)
{
    FMemory::Memmove(Dest, Source, sizeof(SourceElementType) * Count);
    /*

    if constexpr (UE::Core::Private::MemoryOps::TCanBitwiseRelocate<DestinationElementType, SourceElementType>::Value)
    {
        FMemory::Memmove(Dest, Source, sizeof(SourceElementType) * Count);
    }
    else
    {
        while (Count)
        {
            // We need a typedef here because VC won't compile the destructor call below if SourceElementType itself has a member called SourceElementType
            typedef SourceElementType RelocateConstructItemsElementTypeTypedef;

            new (Dest) DestinationElementType(*Source);
            ++(DestinationElementType*&)Dest;
            (Source++)->RelocateConstructItemsElementTypeTypedef::~RelocateConstructItemsElementTypeTypedef();
            --Count;
        }
    }
    */
}

class FDefaultAllocator;

template <uint32 NumInlineElements, typename SecondaryAllocator = FDefaultAllocator>
class TInlineAllocator
{
public:

    enum { NeedsElementType = true };
    enum { RequireRangeCheck = true };

    template<typename ElementType>
    class ForElementType
    {
    public:
        ForElementType()
        {
        }

        FORCEINLINE void MoveToEmpty(ForElementType& Other)
        {
            // checkSlow(this != &Other);

            if (!Other.SecondaryData.GetAllocation())
            {
                RelocateConstructItems<ElementType>((void*)InlineData, Other.GetInlineElements(), NumInlineElements);
            }

            // Move secondary storage in any case.
            // This will move secondary storage if it exists but will also handle the case where secondary storage is used in Other but not in *this.
            SecondaryData.MoveToEmpty(Other.SecondaryData);
        }

        // FContainerAllocatorInterface
        FORCEINLINE ElementType* GetAllocation() const
        {
            return IfAThenAElseB<ElementType>(SecondaryData.GetAllocation(), GetInlineElements());
        }

        void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
        {
            // Check if the new allocation will fit in the inline data area.
            if (NumElements <= NumInlineElements)
            {
                // If the old allocation wasn't in the inline data area, relocate it into the inline data area.
                if (SecondaryData.GetAllocation())
                {
                    RelocateConstructItems<ElementType>((void*)InlineData, (ElementType*)SecondaryData.GetAllocation(), PreviousNumElements);

                    // Free the old indirect allocation.
                    SecondaryData.ResizeAllocation(0, 0, NumBytesPerElement);
                }
            }
            else
            {
                if (!SecondaryData.GetAllocation())
                {
                    // Allocate new indirect memory for the data.
                    SecondaryData.ResizeAllocation(0, NumElements, NumBytesPerElement);

                    // Move the data out of the inline data area into the new allocation.
                    RelocateConstructItems<ElementType>((void*)SecondaryData.GetAllocation(), GetInlineElements(), PreviousNumElements);
                }
                else
                {
                    // Reallocate the indirect data for the new size.
                    SecondaryData.ResizeAllocation(PreviousNumElements, NumElements, NumBytesPerElement);
                }
            }
        }

        FORCEINLINE int32 CalculateSlackReserve(int32 NumElements, SIZE_T NumBytesPerElement) const
        {
            // If the elements use less space than the inline allocation, only use the inline allocation as slack.
            return NumElements <= NumInlineElements ?
                NumInlineElements :
                SecondaryData.CalculateSlackReserve(NumElements, NumBytesPerElement);
        }
        FORCEINLINE int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
        {
            // If the elements use less space than the inline allocation, only use the inline allocation as slack.
            return NumElements <= NumInlineElements ?
                NumInlineElements :
                SecondaryData.CalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement);
        }
        FORCEINLINE int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
        {
            // If the elements use less space than the inline allocation, only use the inline allocation as slack.
            return NumElements <= NumInlineElements ?
                NumInlineElements :
                SecondaryData.CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
        }

        SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return SecondaryData.GetAllocatedSize(NumAllocatedElements, NumBytesPerElement);
        }

        bool HasAllocation()
        {
            return SecondaryData.HasAllocation();
        }

    private:
        ForElementType(const ForElementType&);
        ForElementType& operator=(const ForElementType&);

        /** The data is stored in this array if less than NumInlineElements is needed. */
        TTypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

        /** The data is allocated through the indirect allocation policy if more than NumInlineElements is needed. */
        typename SecondaryAllocator::template ForElementType<ElementType> SecondaryData;

        /** @return the base of the aligned inline element data */
        ElementType* GetInlineElements() const
        {
            return (ElementType*)InlineData;
        }
    };

    typedef void ForAnyElementType;
};


struct FScriptContainerElement
{
};

FORCEINLINE int32 DefaultCalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
{
    int32 Retval;
    // checkSlow(NumElements < NumAllocatedElements);

    const uint32 CurrentSlackElements = NumAllocatedElements - NumElements;
    const SIZE_T CurrentSlackBytes = (NumAllocatedElements - NumElements) * BytesPerElement;
    const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
    const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;
    if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements)) //  hard coded 64 :-(
    {
        Retval = NumElements;
        if (Retval > 0)
        {
            if (bAllowQuantize)
            {
                Retval = FMemory::QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement;
            }
        }
    }
    else
    {
        Retval = NumAllocatedElements;
    }

    return Retval;
}

FORCEINLINE int32 DefaultCalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
{
    int32 Retval;
    // checkSlow(NumElements > NumAllocatedElements && NumElements > 0);

    SIZE_T Grow = 4;
    if (NumAllocatedElements || SIZE_T(NumElements) > Grow)
    {
        // Allocate slack for the array proportional to its size.
        Grow = SIZE_T(NumElements) + 3 * SIZE_T(NumElements) / 8 + 16;
    }
    if (bAllowQuantize)
    {
        Retval = FMemory::QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement;
    }
    else
    {
        Retval = Grow;
    }

    if (NumElements > Retval)
    {
        Retval = MAX_int32;
    }

    return Retval;
}

FORCEINLINE int32 DefaultCalculateSlackReserve(int32 NumElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = DEFAULT_ALIGNMENT)
{
    int32 Retval = NumElements;
    // checkSlow(NumElements > 0);
    if (bAllowQuantize)
    {
        Retval = FMemory::QuantizeSize(SIZE_T(Retval) * SIZE_T(BytesPerElement), Alignment) / BytesPerElement;
        // NumElements and MaxElements are stored in 32 bit signed integers so we must be careful not to overflow here.
        if (NumElements > Retval)
        {
            Retval = MAX_int32;
        }
    }

    return Retval;
}

class FHeapAllocator
{
public:

    enum { NeedsElementType = false };
    enum { RequireRangeCheck = true };

    class ForAnyElementType
    {
    public:
        /** Default constructor. */
        ForAnyElementType()
            : Data(nullptr)
        {}

        FORCEINLINE void MoveToEmpty(ForAnyElementType& Other)
        {
            // checkSlow(this != &Other);

            if (Data)
            {
                FMemory::Free(Data);
            }

            Data = Other.Data;
            Other.Data = nullptr;
        }

        /** Destructor. */
        FORCEINLINE ~ForAnyElementType()
        {
            if (Data)
            {
                FMemory::Free(Data);
            }
        }

        // FContainerAllocatorInterface
        FORCEINLINE FScriptContainerElement* GetAllocation() const
        {
            return Data;
        }
        FORCEINLINE void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
        {
            // Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
            if (Data || NumElements)
            {
                //checkSlow(((uint64)NumElements*(uint64)ElementTypeInfo.GetSize() < (uint64)INT_MAX));
                Data = (FScriptContainerElement*)FMemory::Realloc(Data, NumElements * NumBytesPerElement, DEFAULT_ALIGNMENT);
            }
        }
        FORCEINLINE int32 CalculateSlackReserve(int32 NumElements, int32 NumBytesPerElement) const
        {
            return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true);
        }
        FORCEINLINE int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
        {
            return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }
        FORCEINLINE int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, int32 NumBytesPerElement) const
        {
            return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }

        SIZE_T GetAllocatedSize(int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return NumAllocatedElements * NumBytesPerElement;
        }

        bool HasAllocation()
        {
            return !!Data;
        }

    private:
        ForAnyElementType(const ForAnyElementType&);
        ForAnyElementType& operator=(const ForAnyElementType&);

        /** A pointer to the container's elements. */
        FScriptContainerElement* Data;
    };

    template<typename ElementType>
    class ForElementType : public ForAnyElementType
    {
    public:

        /** Default constructor. */
        ForElementType()
        {}

        FORCEINLINE ElementType* GetAllocation() const
        {
            return (ElementType*)ForAnyElementType::GetAllocation();
        }
    };
};


class FDefaultAllocator;
class FDefaultBitArrayAllocator;

/** Encapsulates the allocators used by a sparse array in a single type. */
template<typename InElementAllocator = FDefaultAllocator, typename InBitArrayAllocator = FDefaultBitArrayAllocator>
class TSparseArrayAllocator
{
public:

    typedef InElementAllocator ElementAllocator;
    typedef InBitArrayAllocator BitArrayAllocator;
};

/** An inline sparse array allocator that allows sizing of the inline allocations for a set number of elements. */
template<
    uint32 NumInlineElements,
    typename SecondaryAllocator = TSparseArrayAllocator<FDefaultAllocator, FDefaultAllocator>
>
class TInlineSparseArrayAllocator
{
private:

    /** The size to allocate inline for the bit array. */
    enum { InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD };

public:

    typedef TInlineAllocator<NumInlineElements, typename SecondaryAllocator::ElementAllocator>		ElementAllocator;
    typedef TInlineAllocator<InlineBitArrayDWORDs, typename SecondaryAllocator::BitArrayAllocator>	BitArrayAllocator;
};



#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4

static FORCEINLINE uint32 CountLeadingZeros(uint32 Value)
{
    unsigned long Log2;
    if (_BitScanReverse(&Log2, Value) != 0)
    {
        return 31 - Log2;
    }

    return 32;
}

static /* constexpr */ FORCEINLINE uint32 CeilLogTwo(uint32 Arg)  // Milxnor: really in FPlatformMath
{
    Arg = Arg ? Arg : 1;
    return 32 - CountLeadingZeros(Arg - 1);
}

static /* constexpr */ FORCEINLINE uint32 RoundUpToPowerOfTwo(uint32 Arg) // Milxnor: really in FPlatformMath
{
    return 1 << CeilLogTwo(Arg);
}

template<
    typename InSparseArrayAllocator = TSparseArrayAllocator<>,
    typename InHashAllocator = TInlineAllocator<1, FDefaultAllocator>,
    uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
    uint32   BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
    uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
>
class TSetAllocator
{
public:

    /** Computes the number of hash buckets to use for a given number of elements. */
    static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
    {
        if (NumHashedElements >= MinNumberOfHashedElements)
        {
            return RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
        }

        return 1;
    }

    typedef InSparseArrayAllocator SparseArrayAllocator;
    typedef InHashAllocator        HashAllocator;
};

class FDefaultAllocator;

/** An inline set allocator that allows sizing of the inline allocations for a set number of elements. */
template<
    uint32   NumInlineElements,
    typename SecondaryAllocator = TSetAllocator<TSparseArrayAllocator<FDefaultAllocator, FDefaultAllocator>, FDefaultAllocator>,
    uint32   AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
    uint32   MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
>
class TInlineSetAllocator
{
private:

    enum { NumInlineHashBuckets = (NumInlineElements + AverageNumberOfElementsPerHashBucket - 1) / AverageNumberOfElementsPerHashBucket };

    static_assert(!(NumInlineHashBuckets& (NumInlineHashBuckets - 1)), "Number of inline buckets must be a power of two");

public:

    /** Computes the number of hash buckets to use for a given number of elements. */
    static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
    {
        const uint32 NumDesiredHashBuckets = RoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket);
        if (NumDesiredHashBuckets < NumInlineHashBuckets)
        {
            return NumInlineHashBuckets;
        }

        if (NumHashedElements < MinNumberOfHashedElements)
        {
            return NumInlineHashBuckets;
        }

        return NumDesiredHashBuckets;
    }

    typedef TInlineSparseArrayAllocator<NumInlineElements, typename SecondaryAllocator::SparseArrayAllocator> SparseArrayAllocator;
    typedef TInlineAllocator<NumInlineHashBuckets, typename SecondaryAllocator::HashAllocator>                HashAllocator;
};


/**
 * 'typedefs' for various allocator defaults.
 *
 * These should be replaced with actual typedefs when Core.h include order is sorted out, as then we won't need to
 * 'forward' these TAllocatorTraits specializations below.
 */

class FDefaultAllocator : public FHeapAllocator { public: typedef FHeapAllocator          Typedef; };
class FDefaultSetAllocator : public TSetAllocator<> { public: typedef TSetAllocator<>         Typedef; };
class FDefaultBitArrayAllocator : public TInlineAllocator<4> { public: typedef TInlineAllocator<4>     Typedef; };
class FDefaultSparseArrayAllocator : public TSparseArrayAllocator<> { public: typedef TSparseArrayAllocator<> Typedef; };
