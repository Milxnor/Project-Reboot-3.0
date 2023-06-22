#pragma once

#include "BuildingSMActor.h"
#include "FortPawn.h"

class ABuildingContainer : public ABuildingSMActor
{
public:
	bool ShouldDestroyOnSearch()
	{
		static auto bDestroyContainerOnSearchOffset = GetOffset("bDestroyContainerOnSearch");
		static auto bDestroyContainerOnSearchFieldMask = GetFieldMask(GetProperty("bDestroyContainerOnSearch"));
		return this->ReadBitfieldValue(bDestroyContainerOnSearchOffset, bDestroyContainerOnSearchFieldMask);
	}

	FName& GetSearchLootTierGroup()
	{
		static auto SearchLootTierGroupOffset = this->GetOffset("SearchLootTierGroup");
		return Get<FName>(SearchLootTierGroupOffset);
	}

	bool IsAlreadySearched()
	{
		static auto bAlreadySearchedOffset = this->GetOffset("bAlreadySearched");
		static auto bAlreadySearchedFieldMask = GetFieldMask(this->GetProperty("bAlreadySearched"));
		return this->ReadBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask);
	}

	FVector& GetLootSpawnLocation_Athena()
	{
		static auto LootSpawnLocation_AthenaOffset = this->GetOffset("LootSpawnLocation_Athena");
		return this->Get<FVector>(LootSpawnLocation_AthenaOffset);
	}

	void SetAlreadySearched(bool bNewValue, bool bOnRep = true)
	{
		static auto bAlreadySearchedOffset = this->GetOffset("bAlreadySearched");
		static auto bAlreadySearchedFieldMask = GetFieldMask(this->GetProperty("bAlreadySearched"));
		this->SetBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask, bNewValue);

		if (bOnRep)
		{
			static auto OnRep_bAlreadySearchedFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingContainer.OnRep_bAlreadySearched");
			this->ProcessEvent(OnRep_bAlreadySearchedFn);
		}
	}

	FVector& GetLootSpawnLocation()
	{
		static auto LootSpawnLocationOffset = GetOffset("LootSpawnLocation");
		return Get<FVector>(LootSpawnLocationOffset);
	}

	float& GetLootNoiseRange()
	{
		static auto LootNoiseRangeOffset = GetOffset("LootNoiseRange");
		return Get<float>(LootNoiseRangeOffset);
	}

	void BounceContainer()
	{
		static auto BounceContainerFn = FindObject<UFunction>("/Script/FortniteGame.BuildingContainer.BounceContainer");
		this->ProcessEvent(BounceContainerFn);
	}

	bool SpawnLoot(AFortPawn* Pawn);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.BuildingContainer");
		return Class;
	}
};