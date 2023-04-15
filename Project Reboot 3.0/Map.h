#pragma once

#include "Set.h"

// template <typename KeyType, typename ValueType>
// using TPair = TTuple<KeyType, ValueType>;

template <typename KeyType, typename ValueType>
class TPair
{
public:
	KeyType First;
	ValueType Second;

    FORCEINLINE KeyType& Key()
    {
        return First;
    }
    FORCEINLINE const KeyType& Key() const
    {
        return First;
    }
    FORCEINLINE ValueType& Value()
    {
        return Second;
    }
    FORCEINLINE const ValueType& Value() const
    {
        return Second;
    }
};

template <typename KeyType, typename ValueType>
class TMap
{
public:
    typedef TPair<KeyType, ValueType> ElementType;

public:
    TSet<ElementType> Pairs;

public:
    class FBaseIterator
    {
    private:
        TMap<KeyType, ValueType>& IteratedMap;
        TSet<ElementType>::FBaseIterator SetIt;

    public:
        FBaseIterator(TMap<KeyType, ValueType>& InMap, TSet<ElementType>::FBaseIterator InSet)
            : IteratedMap(InMap)
            , SetIt(InSet)
        {
        }
        FORCEINLINE TMap<KeyType, ValueType>::FBaseIterator operator++()
        {
            ++SetIt;
            return *this;
        }
        FORCEINLINE TMap<KeyType, ValueType>::ElementType& operator*()
        {
            return *SetIt;
        }
        FORCEINLINE const TMap<KeyType, ValueType>::ElementType& operator*() const
        {
            return *SetIt;
        }
        FORCEINLINE bool operator==(const TMap<KeyType, ValueType>::FBaseIterator& Other) const
        {
            return SetIt == Other.SetIt;
        }
        FORCEINLINE bool operator!=(const TMap<KeyType, ValueType>::FBaseIterator& Other) const
        {
            return SetIt != Other.SetIt;
        }
        FORCEINLINE bool IsElementValid() const
        {
            return SetIt.IsElementValid();
        }
    };

    FORCEINLINE TMap<KeyType, ValueType>::FBaseIterator begin()
    {
        return TMap<KeyType, ValueType>::FBaseIterator(*this, Pairs.begin());
    }
    FORCEINLINE const TMap<KeyType, ValueType>::FBaseIterator begin() const
    {
        return TMap<KeyType, ValueType>::FBaseIterator(*this, Pairs.begin());
    }
    FORCEINLINE TMap<KeyType, ValueType>::FBaseIterator end()
    {
        return TMap<KeyType, ValueType>::FBaseIterator(*this, Pairs.end());
    }
    FORCEINLINE const TMap<KeyType, ValueType>::FBaseIterator end() const
    {
        return TMap<KeyType, ValueType>::FBaseIterator(*this, Pairs.end());
    }
    FORCEINLINE ValueType& operator[](const KeyType& Key)
    {
        return this->GetByKey(Key);
    }
    FORCEINLINE const ValueType& operator[](const KeyType& Key) const
    {
        return this->GetByKey(Key);
    }
    FORCEINLINE int32 Num() const
    {
        return Pairs.Num();
    }
    FORCEINLINE bool IsValid() const
    {
        return Pairs.IsValid();
    }
    FORCEINLINE bool IsIndexValid(int32 IndexToCheck) const
    {
        return Pairs.IsIndexValid(IndexToCheck);
    }
    FORCEINLINE bool Contains(const KeyType& ElementToLookFor) const
    {
        for (auto& Element : *this)
        {
            if (Element.Key() == ElementToLookFor)
                return true;
        }
        return false;
    }
    FORCEINLINE ValueType& GetByKey(const KeyType& Key, bool* wasSuccessful = nullptr)
    {
        for (auto& Pair : *this)
        {
            if (Pair.Key() == Key)
            {
                if (wasSuccessful)
                    *wasSuccessful = true;

                return Pair.Value();
            }
        }

        // LOG_INFO(LogDev, "Failed to find Key!!!");

        if (wasSuccessful)
            *wasSuccessful = false;
    }
    FORCEINLINE ValueType& Find(const KeyType& Key, bool* wasSuccessful = nullptr)
    {
        return GetByKey(Key, wasSuccessful);
    }
    FORCEINLINE ValueType GetByKeyNoRef(const KeyType& Key)
    {
        for (auto& Pair : *this)
        {
            if (Pair.Key() == Key)
            {
                return Pair.Value();
            }
        }
    }
};