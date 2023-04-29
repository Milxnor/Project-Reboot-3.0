#pragma once

#include "Actor.h"

#include "GameplayAbilityTypes.h"

class AFortAthenaMapInfo : public AActor
{
public:
	UClass*& GetAmmoBoxClass()
	{
		static auto AmmoBoxClassOffset = GetOffset("AmmoBoxClass");
		return Get<UClass*>(AmmoBoxClassOffset);
	}

	FScalableFloat* GetAmmoBoxMinSpawnPercent()
	{
		static auto AmmoBoxMinSpawnPercentOffset = GetOffset("AmmoBoxMinSpawnPercent");
		return GetPtr<FScalableFloat>(AmmoBoxMinSpawnPercentOffset);
	}

	FScalableFloat* GetAmmoBoxMaxSpawnPercent()
	{
		static auto AmmoBoxMaxSpawnPercentOffset = GetOffset("AmmoBoxMaxSpawnPercent");
		return GetPtr<FScalableFloat>(AmmoBoxMaxSpawnPercentOffset);
	}

	UClass*& GetTreasureChestClass()
	{
		static auto TreasureChestClassOffset = GetOffset("TreasureChestClass");
		return Get<UClass*>(TreasureChestClassOffset);
	}

	FScalableFloat* GetTreasureChestMinSpawnPercent()
	{
		static auto TreasureChestMinSpawnPercentOffset = GetOffset("TreasureChestMinSpawnPercent");
		return GetPtr<FScalableFloat>(TreasureChestMinSpawnPercentOffset);
	}

	FScalableFloat* GetTreasureChestMaxSpawnPercent()
	{
		static auto TreasureChestMaxSpawnPercentOffset = GetOffset("TreasureChestMaxSpawnPercent");
		return GetPtr<FScalableFloat>(TreasureChestMaxSpawnPercentOffset);
	}
};