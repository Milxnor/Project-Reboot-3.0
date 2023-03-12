#pragma once

#include "FortWorldItemDefinition.h"

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
	int GetClipSize();

	static UClass* StaticClass();
};