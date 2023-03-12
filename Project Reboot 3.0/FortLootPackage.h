#pragma once

#include <vector>

#include "Array.h"
#include "FortWorldItemDefinition.h"
#include "SoftObjectPtr.h"

struct LootDrop
{
	UFortItemDefinition* ItemDefinition;
	int Count;
	int LoadedAmmo;
};

struct FFortLootPackageData
{
public:
	FName& GetLootPackageID()
	{
		static auto LootPackageIDOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "LootPackageID");
		return *(FName*)(__int64(this) + LootPackageIDOffset);
	}

	float& GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "Weight");
		return *(float*)(__int64(this) + WeightOffset);
	}

	FString& GetLootPackageCall()
	{
		static auto LootPackageCallOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "LootPackageCall");
		return *(FString*)(__int64(this) + LootPackageCallOffset);
	}

	TSoftObjectPtr<UFortItemDefinition>& GetItemDefinition()
	{
		static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "ItemDefinition");
		return *(TSoftObjectPtr<UFortItemDefinition>*)(__int64(this) + ItemDefinitionOffset);
	}

	int& GetCount()
	{
		static auto CountOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "Count");
		return *(int*)(__int64(this) + CountOffset);
	}
};

struct FFortLootTierData
{
public:
	float& GetNumLootPackageDrops()
	{
		static auto NumLootPackageDropsOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "NumLootPackageDrops");
		return *(float*)(__int64(this) + NumLootPackageDropsOffset);
	}

	FName& GetTierGroup()
	{
		static auto TierGroupOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "TierGroup");
		return *(FName*)(__int64(this) + TierGroupOffset);
	}

	float& GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "Weight");
		return *(float*)(__int64(this) + WeightOffset);
	}

	FName& GetLootPackage()
	{
		static auto LootPackageOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackage");
		return *(FName*)(__int64(this) + LootPackageOffset);
	}

	TArray<int>& GetLootPackageCategoryWeightArray()
	{
		static auto LootPackageCategoryWeightArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryWeightArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryWeightArrayOffset);
	}

	TArray<int>& GetLootPackageCategoryMinArray()
	{
		static auto LootPackageCategoryMinArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryMinArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryMinArrayOffset);
	}

	TArray<int>& GetLootPackageCategoryMaxArray()
	{
		static auto LootPackageCategoryMaxArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryMaxArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryMaxArrayOffset);
	}
};

std::vector<LootDrop> PickLootDrops(FName TierGroupName, bool bPrint = false, int recursive = 0);