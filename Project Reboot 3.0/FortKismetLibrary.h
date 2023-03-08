#pragma once

#include "Object.h"

#include "FortResourceItemDefinition.h"
#include "BuildingSMActor.h"

class UFortKismetLibrary : public UObject
{
public:
	static UFortResourceItemDefinition* K2_GetResourceItemDefinition(EFortResourceType ResourceType);
	static void ApplyCharacterCosmetics(UObject* WorldContextObject, const TArray<UObject*>& CharacterParts, UObject* PlayerState, bool* bSuccess);

	static UClass* StaticClass();
};