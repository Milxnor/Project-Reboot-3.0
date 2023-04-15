#pragma once

#include "Actor.h"
#include "FortPawn.h"

enum class EFortPickupSourceTypeFlag : uint8_t
{
	Other = 0,
	Player = 1,
	Destruction = 2,
	Container = 3,
	AI = 4,
	Tossed = 5,
	FloorLoot = 6,
	EFortPickupSourceTypeFlag_MAX = 7
};

enum class EFortPickupSpawnSource : uint8_t
{
	Unset = 0,
	PlayerElimination = 1,
	Chest = 2,
	SupplyDrop = 3,
	AmmoBox = 4,
	Drone = 5,
	ItemSpawner = 6,
	EFortPickupSpawnSource_MAX = 7
};

ENUM_CLASS_FLAGS(EFortPickupSourceTypeFlag)

struct FFortPickupLocationData
{
	AFortPawn*& GetPickupTarget()
	{
		static auto PickupTargetOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "PickupTarget");
		return *(AFortPawn**)(__int64(this) + PickupTargetOffset);
	}

	float& GetFlyTime()
	{
		static auto FlyTimeOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "FlyTime");
		return *(float*)(__int64(this) + FlyTimeOffset);
	}

	AFortPawn*& GetItemOwner()
	{
		static auto ItemOwnerOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "ItemOwner");
		return *(AFortPawn**)(__int64(this) + ItemOwnerOffset);
	}

	FVector& GetStartDirection()
	{
		static auto StartDirectionOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "StartDirection");
		return *(FVector*)(__int64(this) + StartDirectionOffset);
	}

	FGuid& GetPickupGuid()
	{
		static auto PickupGuidOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "PickupGuid");
		return *(FGuid*)(__int64(this) + PickupGuidOffset);
	}
};

class AFortPickup : public AActor
{
public:
	static inline char (*CompletePickupAnimationOriginal)(AFortPickup* Pickup);

	void TossPickup(FVector FinalLocation, class AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource);

	void OnRep_PrimaryPickupItemEntry()
	{
		static auto OnRep_PrimaryPickupItemEntryFn = FindObject<UFunction>("/Script/FortniteGame.FortPickup.OnRep_PrimaryPickupItemEntry");
		this->ProcessEvent(OnRep_PrimaryPickupItemEntryFn);
	}

	FFortPickupLocationData* GetPickupLocationData()
	{
		static auto PickupLocationDataOffset = this->GetOffset("PickupLocationData");
		return this->GetPtr<FFortPickupLocationData>(PickupLocationDataOffset);
	}	

	FFortItemEntry* GetPrimaryPickupItemEntry()
	{
		static auto PrimaryPickupItemEntryOffset = this->GetOffset("PrimaryPickupItemEntry");
		return this->GetPtr<FFortItemEntry>(PrimaryPickupItemEntryOffset);
	}

	static AFortPickup* SpawnPickup(FFortItemEntry* ItemEntry, FVector Location, 
		EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset,
		class AFortPawn* Pawn = nullptr, UClass* OverrideClass = nullptr, bool bToss = true);

	static AFortPickup* SpawnPickup(class UFortItemDefinition* ItemDef, FVector Location, int Count, 
		EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset, 
		int LoadedAmmo = -1, class AFortPawn* Pawn = nullptr, UClass* OverrideClass = nullptr, bool bToss = true);

	static char CompletePickupAnimationHook(AFortPickup* Pickup);

	static UClass* StaticClass();
};