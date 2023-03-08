#pragma once

#include "FortItemDefinition.h"
#include "FortVolume.h"

using UPlaysetLevelStreamComponent = UObject;

extern inline __int64 (*LoadPlaysetOriginal)(UPlaysetLevelStreamComponent* a1) = nullptr;

class UFortPlaysetItemDefinition : public UFortItemDefinition // UFortAccountItemDefinition
{
public:
	static void ShowPlayset(UFortPlaysetItemDefinition* PlaysetItemDef, AFortVolume* Volume);
};