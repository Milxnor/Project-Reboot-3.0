#pragma once

#include "Object.h"

class UFortInventoryInterface
{
public:
	static char RemoveInventoryItemHook(__int64 a1, FGuid a2, int Count, char bForceRemoveFromQuickBars, char bForceRemoval);
};