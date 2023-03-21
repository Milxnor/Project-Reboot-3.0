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

template <typename KeyType, typename ValueType> //, typename SetAllocator, typename KeyFuncs>
class TMapBase
{
public:
	typedef TPair<KeyType, ValueType> ElementType;

	typedef TSet<ElementType/*, KeyFuncs, SetAllocator */> ElementSetType;

	ElementSetType Pairs;

    FORCEINLINE ValueType& Find(const KeyType& Key)
    {
        for (int j = 0; j < this->Pairs.Elements.Num(); j++)
        {
            ElementType& Pair = this->Pairs.Elements.operator[](j).ElementData.Value;
            
            if (Key == Pair.Key())
                return Pair.Value();
        }
    } 
};

template <typename KeyType, typename ValueType> //, typename SetAllocator, typename KeyFuncs>
class TSortableMapBase : public TMapBase<KeyType, ValueType> //, SetAllocator, KeyFuncs>
{
};

template<typename KeyType, typename ValueType> //,typename SetAllocator /*= FDefaultSetAllocator*/, typename KeyFuncs /*= TDefaultMapHashableKeyFuncs<KeyType,ValueType,false>*/>
class TMap : public TSortableMapBase<KeyType, ValueType> //, SetAllocator, KeyFuncs>
{

};