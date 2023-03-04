#pragma once

#include "Object.h"

class FScriptInterface
{
private:
	UObject* ObjectPointer;
	void* InterfacePointer;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
};