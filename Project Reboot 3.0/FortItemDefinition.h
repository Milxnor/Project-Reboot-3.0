#pragma once

#include "FortItem.h"
#include "Object.h"
#include "Class.h"

#include "reboot.h"

class UFortItemDefinition : public UObject
{
public:
	UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level = 1);
};