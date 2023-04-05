#pragma once

#include "WeakObjectPtr.h"
#include "Object.h"

template<class T = UObject, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr;

template<class T, class TWeakObjectPtrBase>
struct TWeakObjectPtr : public TWeakObjectPtrBase
{
	T* Get()
	{
		return (T*)TWeakObjectPtrBase::Get();
	}

	bool operator==(const TWeakObjectPtr& other)
	{
		return TWeakObjectPtrBase::operator==(other);
	}
};