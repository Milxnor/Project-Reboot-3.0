#pragma once

#include "Object.h"

class FScriptInterface
{
public:
	UObject* ObjectPointer;
	void* InterfacePointer;

	FORCEINLINE UObject*& GetObjectRef()
	{
		return ObjectPointer;
	}
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
};