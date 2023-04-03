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
};