// Copied from 4.19

#pragma once

#include "SparseArray.h"
#include "ChooseClass.h"
#include "UnrealTypeTraits.h"

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

/** Either NULL or an identifier for an element of a set. */
class FSetElementId
{
public:

    template<typename, typename, typename>
    friend class TSet;

    friend class FScriptSet;

    /** Default constructor. */
    FORCEINLINE FSetElementId() :
        Index(INDEX_NONE)
    {}

    /** @return a boolean value representing whether the id is NULL. */
    FORCEINLINE bool IsValidId() const
    {
        return Index != INDEX_NONE;
    }

    /** Comparison operator. */
    FORCEINLINE friend bool operator==(const FSetElementId& A, const FSetElementId& B)
    {
        return A.Index == B.Index;
    }

    FORCEINLINE int32 AsInteger() const
    {
        return Index;
    }

    FORCEINLINE static FSetElementId FromInteger(int32 Integer)
    {
        return FSetElementId(Integer);
    }

private:

    /** The index of the element in the set's element array. */
    int32 Index;

    /** Initialization constructor. */
    FORCEINLINE FSetElementId(int32 InIndex) :
        Index(InIndex)
    {}

    /** Implicit conversion to the element index. */
    FORCEINLINE operator int32() const
    {
        return Index;
    }
};

template<typename ElementType, typename InKeyType, bool bInAllowDuplicateKeys = false>
struct BaseKeyFuncs
{
    typedef InKeyType KeyType;
    typedef typename TCallTraits<InKeyType>::ParamType KeyInitType;
    typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

    enum { bAllowDuplicateKeys = bInAllowDuplicateKeys };
};

/**
 * A default implementation of the KeyFuncs used by TSet which uses the element as a key.
 */
template<typename ElementType, bool bInAllowDuplicateKeys = false>
struct DefaultKeyFuncs : BaseKeyFuncs<ElementType, ElementType, bInAllowDuplicateKeys>
{
    typedef typename TCallTraits<ElementType>::ParamType KeyInitType;
    typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

    /**
     * @return The key used to index the given element.
     */
    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element;
    }

    /**
     * @return True if the keys match.
     */
    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A == B;
    }

    /** Calculates a hash index for a key. */
    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(Key);
    }
};

template<
    typename InElementType,
    typename KeyFuncs = DefaultKeyFuncs<InElementType>,
    typename Allocator = FDefaultSetAllocator
>
class TSet;

template<
    typename InElementType,
    typename KeyFuncs /*= DefaultKeyFuncs<ElementType>*/,
    typename Allocator /*= FDefaultSetAllocator*/
>
class TSet
{
private:
    friend TSparseArray;

public:
    typedef typename KeyFuncs::KeyInitType     KeyInitType;
    typedef typename KeyFuncs::ElementInitType ElementInitType;

    typedef TSetElement<InElementType> SetElementType;

    typedef InElementType ElementType;
public:
    typedef TSparseArray<SetElementType, typename Allocator::SparseArrayAllocator>     ElementArrayType;
    typedef typename Allocator::HashAllocator::template ForElementType<FSetElementId> HashType;

    ElementArrayType Elements;

    mutable HashType Hash;
    mutable int32    HashSize;

    FORCEINLINE FSetElementId& GetTypedHash(int32 HashIndex) const
    {
        return ((FSetElementId*)Hash.GetAllocation())[HashIndex & (HashSize - 1)];
    }

public:
    FORCEINLINE ElementType* Find(KeyInitType Key)
    {
        FSetElementId ElementId = FindId(Key);
        if (ElementId.IsValidId())
        {
            return &Elements[ElementId].Value;
        }
        else
        {
            return NULL;
        }
    }

    /**
     * Finds an element with the given key in the set.
     * @param Key - The key to search for.
     * @return A const pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
     */
    FORCEINLINE const ElementType* Find(KeyInitType Key) const
    {
        FSetElementId ElementId = FindId(Key);
        if (ElementId.IsValidId())
        {
            return &Elements[ElementId].Value;
        }
        else
        {
            return NULL;
        }
    }


    /** Adds an element to the hash. */
    FORCEINLINE void HashElement(FSetElementId ElementId, const SetElementType& Element) const
    {
        // Compute the hash bucket the element goes in.
        Element.HashIndex = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)) & (HashSize - 1);

        // Link the element into the hash bucket.
        Element.HashNextId = GetTypedHash(Element.HashIndex);
        GetTypedHash(Element.HashIndex) = ElementId;
    }

    /**
     * Checks if the hash has an appropriate number of buckets, and if not resizes it.
     * @param NumHashedElements - The number of elements to size the hash for.
     * @param bAllowShrinking - true if the hash is allowed to shrink.
     * @return true if the set was rehashed.
     */
    bool ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
    {
        // Calculate the desired hash size for the specified number of elements.
        const int32 DesiredHashSize = Allocator::GetNumberOfHashBuckets(NumHashedElements);

        // If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
        if (NumHashedElements > 0 &&
            (!HashSize ||
                HashSize < DesiredHashSize ||
                (HashSize > DesiredHashSize && bAllowShrinking)))
        {
            HashSize = DesiredHashSize;
            Rehash();
            return true;
        }
        else
        {
            return false;
        }
    }

    /** Resizes the hash. */
    void Rehash() const
    {
        // Free the old hash.
        Hash.ResizeAllocation(0, 0, sizeof(FSetElementId));

        int32 LocalHashSize = HashSize;
        if (LocalHashSize)
        {
            // Allocate the new hash.
            // checkSlow(FMath::IsPowerOfTwo(HashSize));
            Hash.ResizeAllocation(0, LocalHashSize, sizeof(FSetElementId));
            for (int32 HashIndex = 0; HashIndex < LocalHashSize; ++HashIndex)
            {
                GetTypedHash(HashIndex) = FSetElementId();
            }

            // Add the existing elements to the new hash.
            for (typename ElementArrayType::TConstIterator ElementIt(Elements); ElementIt; ++ElementIt)
            {
                HashElement(FSetElementId(ElementIt.GetIndex()), *ElementIt);
            }
        }
    }

    template<bool bConst, bool bRangedFor = false>
    class TBaseIterator
    {
    private:
        friend class TSet;

        typedef typename TChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

    public:
        typedef typename TChooseClass<
            bConst,
            typename TChooseClass<bRangedFor, typename ElementArrayType::TRangedForConstIterator, typename ElementArrayType::TConstIterator>::Result,
            typename TChooseClass<bRangedFor, typename ElementArrayType::TRangedForIterator, typename ElementArrayType::TIterator     >::Result
        >::Result ElementItType;

        FORCEINLINE TBaseIterator(const ElementItType& InElementIt)
            : ElementIt(InElementIt)
        {
        }

        /** Advances the iterator to the next element. */
        FORCEINLINE TBaseIterator& operator++()
        {
            ++ElementIt;
            return *this;
        }

        /** conversion to "bool" returning true if the iterator is valid. */
        FORCEINLINE explicit operator bool() const
        {
            return !!ElementIt;
        }
        /** inverse of the "bool" operator */
        FORCEINLINE bool operator !() const
        {
            return !(bool)*this;
        }

        // Accessors.
        FORCEINLINE FSetElementId GetId() const
        {
            return TSet::IndexToId(ElementIt.GetIndex());
        }
        FORCEINLINE ItElementType* operator->() const
        {
            return &ElementIt->Value;
        }
        FORCEINLINE ItElementType& operator*() const
        {
            return ElementIt->Value;
        }

        FORCEINLINE friend bool operator==(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt == Rhs.ElementIt; }
        FORCEINLINE friend bool operator!=(const TBaseIterator& Lhs, const TBaseIterator& Rhs) { return Lhs.ElementIt != Rhs.ElementIt; }

        ElementItType ElementIt;
    };

    /** The base type of whole set iterators. */
    template<bool bConst>
    class TBaseKeyIterator
    {
    private:
        typedef typename TChooseClass<bConst, const TSet, TSet>::Result SetType;
        typedef typename TChooseClass<bConst, const ElementType, ElementType>::Result ItElementType;

    public:
        /** Initialization constructor. */
        FORCEINLINE TBaseKeyIterator(SetType& InSet, KeyInitType InKey)
            : Set(InSet)
            , Key(InKey)
            , Id()
        {
            // The set's hash needs to be initialized to find the elements with the specified key.
            Set.ConditionalRehash(Set.Elements.Num());
            if (Set.HashSize)
            {
                NextId = Set.GetTypedHash(KeyFuncs::GetKeyHash(Key));
                ++(*this);
            }
            /* // Milxnor: This is only on newer builds?
            else
            {
                NextIndex = INDEX_NONE;
            }
            */
        }

        /** Advances the iterator to the next element. */
        FORCEINLINE TBaseKeyIterator& operator++()
        {
            Id = NextId;

            while (Id.IsValidId())
            {
                NextId = Set.GetInternalElement(Id).HashNextId;
                // checkSlow(Id != NextId);

                if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Set[Id]), Key))
                {
                    break;
                }

                Id = NextId;
            }
            return *this;
        }

        /** conversion to "bool" returning true if the iterator is valid. */
        FORCEINLINE explicit operator bool() const
        {
            return Id.IsValidId();
        }
        /** inverse of the "bool" operator */
        FORCEINLINE bool operator !() const
        {
            return !(bool)*this;
        }

        // Accessors.
        FORCEINLINE ItElementType* operator->() const
        {
            return &Set[Id];
        }
        FORCEINLINE ItElementType& operator*() const
        {
            return Set[Id];
        }

    protected:
        SetType& Set;
        typename TTypeTraits<typename KeyFuncs::KeyType>::ConstPointerType Key;
        FSetElementId Id;
        FSetElementId NextId;
    };

    FORCEINLINE const SetElementType& GetInternalElement(FSetElementId Id) const
    {
        return Elements[Id];
    }
    FORCEINLINE SetElementType& GetInternalElement(FSetElementId Id)
    {
        return Elements[Id];
    }

public:
    class TConstIterator : public TBaseIterator<true>
    {
        friend class TSet;

    public:
        FORCEINLINE TConstIterator(const TSet& InSet)
            : TBaseIterator<true>(begin(InSet.Elements))
        {
        }
    };

    class TIterator : public TBaseIterator<false>
    {
        friend class TSet;

    public:
        FORCEINLINE TIterator(TSet& InSet)
            : TBaseIterator<false>(begin(InSet.Elements))
            , Set(InSet)
        {
        }

        /** Removes the current element from the set. */
        /* // T(R)
        FORCEINLINE void RemoveCurrent()
        {
            Set.Remove(TBaseIterator<false>::GetId());
        }
        */

    private:
        TSet& Set;
    };

    using TRangedForConstIterator = TBaseIterator<true, true>;
    using TRangedForIterator = TBaseIterator<false, true>;

    static FORCEINLINE FSetElementId IndexToId(int32 Index)
    {
        return FSetElementId(Index);
    }

    /** Creates an iterator for the contents of this set */
    FORCEINLINE TIterator CreateIterator()
    {
        return TIterator(*this);
    }

    /** Creates a const iterator for the contents of this set */
    FORCEINLINE TConstIterator CreateConstIterator() const
    {
        return TConstIterator(*this);
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
    FORCEINLINE bool IsIndexValid(int32 IndexToCheck) const
    {
        return Elements.IsIndexValid(IndexToCheck);
    }
    FSetElementId FindId(KeyInitType Key) const
    {
        if (Elements.Num())
        {
            for (FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
                ElementId.IsValidId();
                ElementId = Elements[ElementId].HashNextId)
            {
                if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements[ElementId].Value), Key))
                {
                    return ElementId;
                }
            }
        }
        return FSetElementId();
    }

    FORCEINLINE bool Contains(KeyInitType Key) const
    {
        return FindId(Key).IsValidId();
    }

private:
    /**
        * DO NOT USE DIRECTLY
        * STL-like iterators to enable range-based for loop support.
        */
    FORCEINLINE friend TRangedForIterator      begin(TSet& Set) { return TRangedForIterator(begin(Set.Elements)); }
    FORCEINLINE friend TRangedForConstIterator begin(const TSet& Set) { return TRangedForConstIterator(begin(Set.Elements)); }
    FORCEINLINE friend TRangedForIterator      end(TSet& Set) { return TRangedForIterator(end(Set.Elements)); }
    FORCEINLINE friend TRangedForConstIterator end(const TSet& Set) { return TRangedForConstIterator(end(Set.Elements)); }
};
