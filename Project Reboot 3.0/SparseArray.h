#pragma once

#include "Array.h"
#include "BitArray.h"
#include "ChooseClass.h"
#include "log.h"
#include "TypeCompatibleBytes.h"
#include "IsTriviallyDestructible.h"

#define INDEX_NONE -1

template<typename ElementType>
union TSparseArrayElementOrFreeListLink
{
    /** If the element is allocated, its value is stored here. */
    ElementType ElementData;

    struct
    {
        /** If the element isn't allocated, this is a link to the previous element in the array's free list. */
        int32 PrevFreeIndex;

        /** If the element isn't allocated, this is a link to the next element in the array's free list. */
        int32 NextFreeIndex;
    };
};

template<typename ElementType, typename Allocator = FDefaultSparseArrayAllocator >
class TSparseArray;

template<typename InElementType, typename Allocator>
class TSparseArray
{
    using ElementType = InElementType;

public:
    typedef TSparseArrayElementOrFreeListLink<
        TAlignedBytes<sizeof(ElementType), alignof(ElementType)>
    > FElementOrFreeListLink;

    typedef TArray<FElementOrFreeListLink/*, typename Allocator::ElementAllocator*/> DataType;
    DataType Data;

    typedef TBitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;
    AllocationBitArrayType AllocationFlags;

    /** The index of an unallocated element in the array that currently contains the head of the linked list of free elements. */
    int32 FirstFreeIndex;

    /** The number of elements in the free list. */
    int32 NumFreeIndices;

    FORCEINLINE int32 Num() const
    {
        return Data.Num() - NumFreeIndices;
    }
    void RemoveAt(int32 Index, int32 Count = 1)
    {
        if (!TIsTriviallyDestructible<ElementType>::Value)
        {
            for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
            {
                ((ElementType&)GetData(It).ElementData).~ElementType();
            }
        }

        RemoveAtUninitialized(Index, Count);
    }

    /** Removes Count elements from the array, starting from Index, without destructing them. */
    void RemoveAtUninitialized(int32 Index, int32 Count = 1)
    {
        for (; Count; --Count)
        {
            // check(AllocationFlags[Index]);

            // Mark the element as free and add it to the free element list.
            if (NumFreeIndices)
            {
                GetData(FirstFreeIndex).PrevFreeIndex = Index;
            }
            auto& IndexData = GetData(Index);
            IndexData.PrevFreeIndex = -1;
            IndexData.NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
            FirstFreeIndex = Index;
            ++NumFreeIndices;
            AllocationFlags[Index] = false;

            ++Index;
        }
    }

    template<bool bConst>
    class TBaseIterator
    {
    public:
        typedef TConstSetBitIterator<typename Allocator::BitArrayAllocator> BitArrayItType;

    private:
        typedef typename TChooseClass<bConst, const TSparseArray, TSparseArray>::Result ArrayType;
        typedef typename TChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

    public:
        explicit TBaseIterator(ArrayType& InArray, const BitArrayItType& InBitArrayIt)
            : Array(InArray)
            , BitArrayIt(InBitArrayIt)
        {
        }

        FORCEINLINE TBaseIterator& operator++()
        {
            // Iterate to the next set allocation flag.
            ++BitArrayIt;
            return *this;
        }

        FORCEINLINE int32 GetIndex() const { return BitArrayIt.GetIndex(); }

        FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.BitArrayIt == Rhs.BitArrayIt && &Lhs.Array == &Rhs.Array; }
        FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.BitArrayIt != Rhs.BitArrayIt || &Lhs.Array != &Rhs.Array; }

        /** conversion to "bool" returning true if the iterator is valid. */
        FORCEINLINE explicit operator bool() const
        {
            return !!BitArrayIt;
        }

        /** inverse of the "bool" operator */
        FORCEINLINE bool operator !() const
        {
            return !(bool)*this;
        }

        FORCEINLINE ItElementType& operator*() const { return Array[GetIndex()]; }
        FORCEINLINE ItElementType* operator->() const { return &Array[GetIndex()]; }
        FORCEINLINE const FRelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }

    protected:
        ArrayType& Array;
        BitArrayItType BitArrayIt;
    };

public:

    /** Iterates over all allocated elements in a sparse array. */
    class TIterator : public TBaseIterator<false>
    {
    public:
        TIterator(TSparseArray& InArray)
            : TBaseIterator<false>(InArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
        {
        }

        TIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
            : TBaseIterator<false>(InArray, InBitArrayIt)
        {
        }

        /** Safely removes the current element from the array. */
        /*
        void RemoveCurrent()
        {
            this->Array.RemoveAt(this->GetIndex());
        }
        */
    };

    /** Iterates over all allocated elements in a const sparse array. */
    class TConstIterator : public TBaseIterator<true>
    {
    public:
        TConstIterator(const TSparseArray& InArray)
            : TBaseIterator<true>(InArray, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(InArray.AllocationFlags))
        {
        }

        TConstIterator(const TSparseArray& InArray, const typename TBaseIterator<true>::BitArrayItType& InBitArrayIt)
            : TBaseIterator<true>(InArray, InBitArrayIt)
        {
        }
    };

#if TSPARSEARRAY_RANGED_FOR_CHECKS // T(R) Milxnor: Check this
    class TRangedForIterator : public TIterator
    {
    public:
        TRangedForIterator(TSparseArray& InArray, const typename TBaseIterator<false>::BitArrayItType& InBitArrayIt)
            : TIterator(InArray, InBitArrayIt)
            , InitialNum(InArray.Num())
        {
        }

    private:
        int32 InitialNum;

        friend FORCEINLINE bool operator!=(const TRangedForIterator& Lhs, const TRangedForIterator& Rhs)
        {
            // We only need to do the check in this operator, because no other operator will be
            // called until after this one returns.
            //
            // Also, we should only need to check one side of this comparison - if the other iterator isn't
            // even from the same array then the compiler has generated bad code.
            ensureMsgf(Lhs.Array.Num() == Lhs.InitialNum, TEXT("Container has changed during ranged-for iteration!"));
            return *(TIterator*)&Lhs != *(TIterator*)&Rhs;
        }
    };

    class TRangedForConstIterator : public TConstIterator
    {
    public:
        TRangedForConstIterator(const TSparseArray& InArray, const typename TBaseIterator<true>::BitArrayItType& InBitArrayIt)
            : TConstIterator(InArray, InBitArrayIt)
            , InitialNum(InArray.Num())
        {
        }

    private:
        int32 InitialNum;

        friend FORCEINLINE bool operator!=(const TRangedForConstIterator& Lhs, const TRangedForConstIterator& Rhs)
        {
            // We only need to do the check in this operator, because no other operator will be
            // called until after this one returns.
            //
            // Also, we should only need to check one side of this comparison - if the other iterator isn't
            // even from the same array then the compiler has generated bad code.
            ensureMsgf(Lhs.Array.Num() == Lhs.InitialNum, TEXT("Container has changed during ranged-for iteration!"));
            return *(TIterator*)&Lhs != *(TIterator*)&Rhs;
        }
    };
#else
    using TRangedForIterator = TIterator;
    using TRangedForConstIterator = TConstIterator;
#endif

public:

    ElementType& operator[](int32 Index)
    {
        // checkSlow(Index >= 0 && Index < Data.Num() && Index < AllocationFlags.Num());
        //checkSlow(AllocationFlags[Index]); // Disabled to improve loading times -BZ
        return *(ElementType*)&GetData(Index).ElementData;
    }
    const ElementType& operator[](int32 Index) const
    {
        // checkSlow(Index >= 0 && Index < Data.Num() && Index < AllocationFlags.Num());
        //checkSlow(AllocationFlags[Index]); // Disabled to improve loading times -BZ
        return *(ElementType*)&GetData(Index).ElementData;
    }

    FElementOrFreeListLink& GetData(int32 Index)
    {
        return ((FElementOrFreeListLink*)Data.GetData())[Index];
    }

    /** Accessor for the element or free list data. */
    const FElementOrFreeListLink& GetData(int32 Index) const
    {
        return ((FElementOrFreeListLink*)Data.GetData())[Index];
    }

    FORCEINLINE friend TRangedForIterator      begin(TSparseArray& Array) { return TRangedForIterator(Array, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags)); }
    FORCEINLINE friend TRangedForConstIterator begin(const TSparseArray& Array) { return TRangedForConstIterator(Array, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags)); }
    FORCEINLINE friend TRangedForIterator      end(TSparseArray& Array) { return TRangedForIterator(Array, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags, Array.AllocationFlags.Num())); }
    FORCEINLINE friend TRangedForConstIterator end(const TSparseArray& Array) { return TRangedForConstIterator(Array, TConstSetBitIterator<typename Allocator::BitArrayAllocator>(Array.AllocationFlags, Array.AllocationFlags.Num())); }

};