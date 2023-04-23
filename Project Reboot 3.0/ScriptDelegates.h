#pragma once
#include "WeakObjectPtr.h"

template <typename TWeakPtr = FWeakObjectPtr>
class TScriptDelegate
{
public:
	TWeakPtr Object;
	FName FunctionName;
};