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

	static void ApplyOverrideCharacterCustomizationHook(UFortAthenaAIBotCustomizationData* InBotData, AFortPlayerPawn* NewBot, __int64 idk)
	{
		LOG_INFO(LogDev, "ApplyOverrideCharacterCustomizationHook!");

		auto CharacterCustomization = InBotData->GetCharacterCustomization();

		auto Controller = NewBot->GetController();

		LOG_INFO(LogDev, "Controller: {}", Controller->IsValidLowLevel() ? Controller->GetPathName() : "BadRead");

		static auto CosmeticLoadoutBCOffset = Controller->GetOffset("CosmeticLoadoutBC");
		Controller->GetPtr<FFortAthenaLoadout>(CosmeticLoadoutBCOffset)->GetCharacter() = CharacterCustomization->GetCustomizationLoadout()->GetCharacter();

		auto PlayerStateAsFort = Cast<AFortPlayerState>(Controller->GetPlayerState());

		static auto UpdatePlayerCustomCharacterPartsVisualizationFn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization");
		PlayerStateAsFort->ProcessEvent(UpdatePlayerCustomCharacterPartsVisualizationFn, &PlayerStateAsFort);

		PlayerStateAsFort->ForceNetUpdate();
		NewBot->ForceNetUpdate();
		Controller->ForceNetUpdate();

		// NewBot->GetCosmeticLoadout()->GetCharacter() = CharacterCustomization->GetCustomizationLoadout()->GetCharacter();
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotCustomizationData");
		return Class;
	}
};