#pragma once

#include "Object.h"

class UFortAthenaAIBotCharacterCustomization : public UObject
{
public:
	FFortAthenaLoadout* GetCustomizationLoadout()
	{
		static auto CustomizationLoadoutOffset = GetOffset("CustomizationLoadout");
		return GetPtr<FFortAthenaLoadout>(CustomizationLoadoutOffset);
	}
};