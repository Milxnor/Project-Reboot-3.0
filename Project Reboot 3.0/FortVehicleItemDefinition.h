#pragma once

#include "SoftObjectPtr.h"
#include "FortWorldItemDefinition.h"
#include "GameplayAbilityTypes.h"

class UFortVehicleItemDefinition : public UFortWorldItemDefinition
{
public:
	FScalableFloat* GetMinPercentWithGas()
	{
		static auto MinPercentWithGasOffset = GetOffset("MinPercentWithGas");
		return GetPtr<FScalableFloat>(MinPercentWithGasOffset);
	}

	FScalableFloat* GetMaxPercentWithGas()
	{
		static auto MaxPercentWithGasOffset = GetOffset("MaxPercentWithGas");
		return GetPtr<FScalableFloat>(MaxPercentWithGasOffset);
	}

	FScalableFloat* GetVehicleMinSpawnPercent()
	{
		static auto VehicleMinSpawnPercentOffset = GetOffset("VehicleMinSpawnPercent");
		return GetPtr<FScalableFloat>(VehicleMinSpawnPercentOffset);
	}

	FScalableFloat* GetVehicleMaxSpawnPercent()
	{
		static auto VehicleMaxSpawnPercentOffset = GetOffset("VehicleMaxSpawnPercent");
		return GetPtr<FScalableFloat>(VehicleMaxSpawnPercentOffset);
	}

	TSoftObjectPtr<UClass>* GetVehicleActorClassSoft()
	{
		static auto VehicleActorClassOffset = GetOffset("VehicleActorClass");
		return GetPtr<TSoftObjectPtr<UClass>>(VehicleActorClassOffset);
	}

	UClass* GetVehicleActorClass()
	{
		static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
		return GetVehicleActorClassSoft()->Get(BGAClass, true);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortVehicleItemDefinition");
		return Class;
	}
};