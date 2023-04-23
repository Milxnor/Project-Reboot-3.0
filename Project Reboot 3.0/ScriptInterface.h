#pragma once

#include "Object.h"

class FScriptInterface
{
public:
	UObject* ObjectPointer = nullptr;
	void* InterfacePointer = nullptr;

	FORCEINLINE UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
};