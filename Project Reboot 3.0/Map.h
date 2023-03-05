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
};

template <typename KeyType, typename ValueType> //, typename SetAllocator, typename KeyFuncs>
class TMapBase
{
public:
	typedef TPair<KeyType, ValueType> ElementType;

	typedef TSet<ElementType/*, KeyFuncs, SetAllocator */> ElementSetType;

	ElementSetType Pairs;
};

template <typename KeyType, typename ValueType> //, typename SetAllocator, typename KeyFuncs>
class TSortableMapBase : public TMapBase<KeyType, ValueType> //, SetAllocator, KeyFuncs>
{
};

template<typename KeyType, typename ValueType> //,typename SetAllocator /*= FDefaultSetAllocator*/, typename KeyFuncs /*= TDefaultMapHashableKeyFuncs<KeyType,ValueType,false>*/>
class TMap : public TSortableMapBase<KeyType, ValueType> //, SetAllocator, KeyFuncs>
{

};