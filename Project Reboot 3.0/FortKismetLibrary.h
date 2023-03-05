#pragma once

#include "Object.h"

#include "FortResourceItemDefinition.h"
#include "BuildingSMActor.h"

class UFortKismetLibrary : public UObject
{
public:
	static UFortResourceItemDefinition* K2_GetResourceItemDefinition(EFortResourceType ResourceType);

	static UClass* StaticClass();
};