#pragma once

#include <random>

#include "Actor.h"

#include "GameplayAbilityTypes.h"
#include "DataTableFunctionLibrary.h"
#include "SoftObjectPtr.h"

static inline float CalcuateCurveMinAndMax(FScalableFloat* Min, FScalableFloat* Max, float Multiplier = 100.f) // returns 000 not 0.00 (forgot techinal name for this)
{
	float MinSpawnPercent = UDataTableFunctionLibrary::EvaluateCurveTableRow(Min->GetCurve().CurveTable, Min->GetCurve().RowName, 0);
	float MaxSpawnPercent = UDataTableFunctionLibrary::EvaluateCurveTableRow(Max->GetCurve().CurveTable, Max->GetCurve().RowName, 0);

	std::random_device MinMaxRd;
	std::mt19937 MinMaxGen(MinMaxRd());
	std::uniform_int_distribution<> MinMaxDis(MinSpawnPercent * Multiplier, MaxSpawnPercent * Multiplier + 1); // + 1 ?

	float SpawnPercent = MinMaxDis(MinMaxGen);

	return SpawnPercent;
}

struct FBuildingGameplayActorSpawnDetails
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.BuildingGameplayActorSpawnDetails");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	FScalableFloat* GetSpawnHeight()
	{
		static auto SpawnHeightOffset = FindOffsetStruct("/Script/FortniteGame.BuildingGameplayActorSpawnDetails", "SpawnHeight");
		return (FScalableFloat*)(__int64(this) + SpawnHeightOffset);
	}

	UClass*& GetBuildingGameplayActorClass()
	{
		static auto BuildingGameplayActorClassOffset = FindOffsetStruct("/Script/FortniteGame.BuildingGameplayActorSpawnDetails", "BuildingGameplayActorClass");
		return *(UClass**)(__int64(this) + BuildingGameplayActorClassOffset);
	}

	UClass*& GetTargetActorClass()
	{
		static auto TargetActorClassOffset = FindOffsetStruct("/Script/FortniteGame.BuildingGameplayActorSpawnDetails", "TargetActorClass");
		return *(UClass**)(__int64(this) + TargetActorClassOffset);
	}
};

struct FVehicleClassDetails
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.VehicleClassDetails");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	TSoftObjectPtr<UClass>& GetVehicleClass()
	{
		static auto VehicleClassOffset = FindOffsetStruct("/Script/FortniteGame.VehicleClassDetails", "VehicleClass");
		return *(TSoftObjectPtr<UClass>*)(__int64(this) + VehicleClassOffset);
	}

	FScalableFloat* GetVehicleMinSpawnPercent()
	{
		static auto VehicleMinSpawnPercentOffset = FindOffsetStruct("/Script/FortniteGame.VehicleClassDetails", "VehicleMinSpawnPercent");
		return (FScalableFloat*)(__int64(this) + VehicleMinSpawnPercentOffset);
	}

	FScalableFloat* GetVehicleMaxSpawnPercent()
	{
		static auto VehicleMaxSpawnPercentOffset = FindOffsetStruct("/Script/FortniteGame.VehicleClassDetails", "VehicleMaxSpawnPercent");
		return (FScalableFloat*)(__int64(this) + VehicleMaxSpawnPercentOffset);
	}
};

class AFortAthenaMapInfo : public AActor
{
public:
	TArray<FVehicleClassDetails>& GetVehicleClassDetails()
	{
		static auto VehicleClassDetailsOffset = GetOffset("VehicleClassDetails");
		return Get<TArray<FVehicleClassDetails>>(VehicleClassDetailsOffset);
	}

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

	TArray<FBuildingGameplayActorSpawnDetails>& GetBuildingGameplayActorSpawnDetails()
	{
		static auto BuildingGameplayActorSpawnDetailsOffset = GetOffset("BuildingGameplayActorSpawnDetails");
		return Get<TArray<FBuildingGameplayActorSpawnDetails>>(BuildingGameplayActorSpawnDetailsOffset);
	}

	FScalableFloat* GetLlamaQuantityMin()
	{
		static auto LlamaQuantityMinOffset = GetOffset("LlamaQuantityMin");
		return GetPtr<FScalableFloat>(LlamaQuantityMinOffset);
	}

	FScalableFloat* GetLlamaQuantityMax()
	{
		static auto LlamaQuantityMaxOffset = GetOffset("LlamaQuantityMax");
		return GetPtr<FScalableFloat>(LlamaQuantityMaxOffset);
	}

	UClass* GetLlamaClass()
	{
		static auto LlamaClassOffset = GetOffset("LlamaClass", false);

		if (LlamaClassOffset == -1)
			return nullptr;

		return Get<UClass*>(LlamaClassOffset);
	}

	AActor*& GetAircraftDropVolume() // actually AVolume
	{
		static auto AircraftDropVolumeOffset = GetOffset("AircraftDropVolume");
		return Get<AActor*>(AircraftDropVolumeOffset);
	}

	FVector PickSupplyDropLocation(FVector Center, float Radius);
	void SpawnLlamas();
};