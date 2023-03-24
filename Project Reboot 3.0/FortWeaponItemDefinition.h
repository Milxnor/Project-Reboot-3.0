#pragma once

#include "FortWorldItemDefinition.h"

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
	int GetClipSize();
	UFortWorldItemDefinition* GetAmmoData();

	static UClass* StaticClass();
};