#pragma once

#include "reboot.h"
#include "FortAthenaAIBotCharacterCustomization.h"

class UFortAthenaAIBotCustomizationData : public UObject // UPrimaryDataAsset
{
public:
	UFortAthenaAIBotCharacterCustomization*& GetCharacterCustomization()
	{
		static auto CharacterCustomizationOffset = GetOffset("CharacterCustomization");
		return Get<UFortAthenaAIBotCharacterCustomization*>(CharacterCustomizationOffset);
	}

	/* static void ApplyOverrideCharacterCustomizationHook(UFortAthenaAIBotCustomizationData* InBotData, AFortPlayerPawn* NewBot, __int64 idk)
	{
		LOG_INFO(LogDev, "ApplyOverrideCharacterCustomizationHook!");

		auto CharacterCustomization = InBotData->GetCharacterCustomization();

		NewBot->GetCosmeticLoadout()->GetCharacter() = CharacterCustomization->GetCustomizationLoadout()->GetCharacter();

		NewBot->Get<bool>(0x1B30) = true; // idk this is like a initialize check
	} */

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotCustomizationData");
		return Class;
	}
};