#pragma once

#include "FortAthenaMutator.h"
#include "GameplayAbilityTypes.h"
#include "reboot.h"

struct FFortPieSliceSpawnData
{
	FScalableFloat                              SpawnDirection;                                           // 0x0000(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              SpawnDirectionDeviation;                                  // 0x0020(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              MinSpawnDistanceFromCenter;                               // 0x0040(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              MaxSpawnDistanceFromCenter;                               // 0x0060(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
};

struct FHeistExitCraftSpawnData : public FFortPieSliceSpawnData
{
	FScalableFloat                              SpawnDelayTime;                                           // 0x0080(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              SafeZonePhaseWhenToSpawn;                                 // 0x00A0(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              SafeZonePhaseWhereToSpawn;                                // 0x00C0(0x0020) (Edit, BlueprintVisible, DisableEditOnInstance, NativeAccessSpecifierPublic)
};

class AFortAthenaMutator_Heist : public AFortAthenaMutator
{
public:
	/* TArray<FHeistExitCraftSpawnData>& GetHeistExitCraftSpawnData()
	{
		static auto HeistExitCraftSpawnDataOffset = GetOffset("HeistExitCraftSpawnData");
		return Get<TArray<FHeistExitCraftSpawnData>>(HeistExitCraftSpawnDataOffset);
	} */

	float& GetSpawnExitCraftTime()
	{
		static auto SpawnExitCraftTimeOffset = GetOffset("SpawnExitCraftTime");
		return Get<float>(SpawnExitCraftTimeOffset);
	}
	
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaMutator_Heist");
		return Class;
	}
};