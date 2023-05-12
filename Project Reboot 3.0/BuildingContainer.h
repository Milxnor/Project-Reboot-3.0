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

	UStaticMesh*& GetSearchedMesh()
	{
		static auto SearchedMeshOffset = GetOffset("SearchedMesh");
		return Get<UStaticMesh*>(SearchedMeshOffset);
	}

	UStaticMesh*& GetStaticMesh()
	{
		static auto StaticMeshOffset = GetOffset("StaticMesh");
		return Get<UStaticMesh*>(StaticMeshOffset);
	}

	void BuildingContainer_SetMeshSet(FMeshSet* MeshSet);

	bool IsAlreadySearched()
	{
		static auto bAlreadySearchedOffset = this->GetOffset("bAlreadySearched");
		static auto bAlreadySearchedFieldMask = GetFieldMask(this->GetProperty("bAlreadySearched"));
		return this->ReadBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask);
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

	bool SpawnLoot(AFortPawn* Pawn);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.BuildingContainer");
		return Class;
	}
};