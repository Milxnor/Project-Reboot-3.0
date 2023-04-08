#pragma once

#include "Array.h"
#include "BitArray.h"
#include "log.h"

#define INDEX_NONE -1

template <typename ElementType>
union TSparseArrayElementOrListLink
{
    TSparseArrayElementOrListLink(ElementType& InElement)
        : ElementData(InElement)
    {
    }
    TSparseArrayElementOrListLink(ElementType&& InElement)
        : ElementData(InElement)
    {
    }

    TSparseArrayElementOrListLink(int32 InPrevFree, int32 InNextFree)
        : PrevFreeIndex(InPrevFree)
        , NextFreeIndex(InNextFree)
    {
    }

    TSparseArrayElementOrListLink<ElementType> operator=(const TSparseArrayElementOrListLink<ElementType>& Other)
    {
        return TSparseArrayElementOrListLink(Other.NextFreeIndex, Other.PrevFreeIndex);
    }

    /** If the element is allocated, its value is stored here. */
    ElementType ElementData;

    struct
    {
        /** If the element isn't allocated, this is a link to the previous element in the array's free list. */
        int PrevFreeIndex;

        /** If the element isn't allocated, this is a link to the next element in the array's free list. */
        int NextFreeIndex;
    };
};

template <typename ArrayType>
class TSparseArray
{
public:
    typedef TSparseArrayElementOrListLink<ArrayType> FSparseArrayElement;

    TArray<FSparseArrayElement> Data;
    TBitArray AllocationFlags;
    int32 FirstFreeIndex;
    int32 NumFreeIndices;

    FORCEINLINE int32 Num() const
    {
        return Data.Num() - NumFreeIndices;
    }

    class FBaseIterator
    {
    private:
        TSparseArray<ArrayType>& IteratedArray;
        TBitArray::FSetBitIterator BitArrayIt;

    public:
        FORCEINLINE FBaseIterator(const TSparseArray<ArrayType>& Array, const TBitArray::FSetBitIterator BitIterator)
            : IteratedArray(const_cast<TSparseArray<ArrayType>&>(Array))
            , BitArrayIt(const_cast<TBitArray::FSetBitIterator&>(BitIterator))
        {
        }

        FORCEINLINE explicit operator bool() const
        {
            return (bool)BitArrayIt;
        }
        FORCEINLINE TSparseArray<ArrayType>::FBaseIterator& operator++()
        {
            ++BitArrayIt;
            return *this;
        }
        FORCEINLINE ArrayType& operator*()
        {
            return IteratedArray[BitArrayIt.GetIndex()].ElementData;
        }
        FORCEINLINE const ArrayType& operator*() const
        {
            return IteratedArray[BitArrayIt.GetIndex()].ElementData;
        }
        FORCEINLINE ArrayType* operator->()
        {
            return &IteratedArray[BitArrayIt.GetIndex()].ElementData;
        }
        FORCEINLINE const ArrayType* operator->() const
        {
            return &IteratedArray[BitArrayIt.GetIndex()].ElementData;
        }
        FORCEINLINE bool operator==(const TSparseArray<ArrayType>::FBaseIterator& Other) const
        {
            return BitArrayIt == Other.BitArrayIt;
        }
        FORCEINLINE bool operator!=(const TSparseArray<ArrayType>::FBaseIterator& Other) const
        {
            return BitArrayIt != Other.BitArrayIt;
        }

        FORCEINLINE int32 GetIndex() const
        {
            return BitArrayIt.GetIndex();
        }
        FORCEINLINE bool IsElementValid() const
        {
            return *BitArrayIt;
        }
    };

public:
    FORCEINLINE TSparseArray<ArrayType>::FBaseIterator begin()
    {
        return TSparseArray<ArrayType>::FBaseIterator(*this, TBitArray::FSetBitIterator(AllocationFlags, 0));
    }
    FORCEINLINE const TSparseArray<ArrayType>::FBaseIterator begin() const
    {
        return TSparseArray<ArrayType>::FBaseIterator(*this, TBitArray::FSetBitIterator(AllocationFlags, 0));
    }
    FORCEINLINE TSparseArray<ArrayType>::FBaseIterator end()
    {
        return TSparseArray<ArrayType>::FBaseIterator(*this, TBitArray::FSetBitIterator(AllocationFlags));
    }
    FORCEINLINE const TSparseArray<ArrayType>::FBaseIterator end() const
    {
        return TSparseArray<ArrayType>::FBaseIterator(*this, TBitArray::FSetBitIterator(AllocationFlags));
    }

    FORCEINLINE FSparseArrayElement& operator[](uint32 Index)
    {
        return *(FSparseArrayElement*)&Data.at(Index).ElementData;
    }
    FORCEINLINE const FSparseArrayElement& operator[](uint32 Index) const
    {
        return *(const FSparseArrayElement*)&Data.at(Index).ElementData;
    }
    FORCEINLINE int32 GetNumFreeIndices() const
    {
        return NumFreeIndices;
    }
    FORCEINLINE int32 GetFirstFreeIndex() const
    {
        return FirstFreeIndex;
    }
    FORCEINLINE const TArray<FSparseArrayElement>& GetData() const
    {
        return Data;
    }
    FSparseArrayElement& GetData(int32 Index)
    {
        return *(FSparseArrayElement*)&Data.at(Index).ElementData;
        // return ((FSparseArrayElement*)Data.Data)[Index];
    }

    /** Accessor for the element or free list data. */
    const FSparseArrayElement& GetData(int32 Index) const
    {
        return *(const FSparseArrayElement*)&Data.at(Index).ElementData;
        // return ((FSparseArrayElement*)Data.Data)[Index];
    }
    FORCEINLINE const TBitArray& GetAllocationFlags() const
    {
        return AllocationFlags;
    }
    FORCEINLINE bool IsIndexValid(int32 IndexToCheck) const
    {
        return AllocationFlags.IsSet(IndexToCheck);
    }

    void RemoveAt(int32 Index, int32 Count = 1)
    {
        /* if (!TIsTriviallyDestructible<ElementType>::Value)
        {
            for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
            {
                ((ElementType&)GetData(It).ElementData).~ElementType();
            }
        } */

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
            AllocationFlags.Set(Index, false);
            // AllocationFlags[Index] = false;

            ++Index;
        }
    }

    /*
    FORCEINLINE bool RemoveAt(const int32 IndexToRemove)
    {
        LOG_INFO(LogDev, "IndexToRemove: {}", IndexToRemove);
        LOG_INFO(LogDev, "AllocationFlags.IsSet(IndexToRemove): {}", AllocationFlags.IsSet(IndexToRemove));
        LOG_INFO(LogDev, "Data.Num(): {}", Data.Num());

        if (IndexToRemove >= 0 && IndexToRemove < Data.Num() && AllocationFlags.IsSet(IndexToRemove))
        {
            int32 PreviousFreeIndex = -1;
            int32 NextFreeIndex = -1;

            LOG_INFO(LogDev, "NumFreeIndices: {}", NumFreeIndices);

            if (NumFreeIndices == 0)
            {
                FirstFreeIndex = IndexToRemove;
                Data.at(IndexToRemove) = { -1, -1 };
            }
            else
            {
                for (auto It = AllocationFlags.begin(); It != AllocationFlags.end(); ++It)
                {
                    if (!It)
                    {
                        if (It.GetIndex() < IndexToRemove)
                        {
                            Data.at(IndexToRemove).PrevFreeIndex = It.GetIndex();
                        }
                        else if (It.GetIndex() > IndexToRemove)
                        {
                            Data.at(IndexToRemove).NextFreeIndex = It.GetIndex();
                            break;
                        }
                    }
                }
            }

            AllocationFlags.Set(IndexToRemove, false);
            NumFreeIndices++;

            return true;
        }
        return false;
    }
    */
};