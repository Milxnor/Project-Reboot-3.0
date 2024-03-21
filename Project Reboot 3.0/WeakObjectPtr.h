#pragma once

#include "UObjectArray.h"

struct FWeakObjectPtr
{
public:
	int		ObjectIndex;
	int		ObjectSerialNumber;

	UObject* Get()
	{
		return ChunkedObjects ? ChunkedObjects->GetObjectByIndex(ObjectIndex) : UnchunkedObjects ? UnchunkedObjects->GetObjectByIndex(ObjectIndex) : nullptr;
	}

	FORCEINLINE bool operator==(const FWeakObjectPtr& Other) const
	{
		return
			(ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber)
			// || (!IsValid() && !Other.IsValid())
			;
	}

	friend uint32 GetTypeHash(const FWeakObjectPtr& WeakObjectPtr)
	{
		return uint32(WeakObjectPtr.ObjectIndex ^ WeakObjectPtr.ObjectSerialNumber);
	}

	bool operator==(const FWeakObjectPtr& other)
	{
		return ObjectIndex == other.ObjectIndex && ObjectSerialNumber == other.ObjectSerialNumber;
	}
};