#pragma once

#include "Actor.h"
#include "FortPawn.h"
#include "Class.h"

namespace EFortPickupSourceTypeFlag
{
	static inline UEnum* GetEnum()
	{
		static auto Enum = FindObject<UEnum>(L"/Script/FortniteGame.EFortPickupSourceTypeFlag");
		return Enum;
	}

	static inline int64 GetPlayerValue()
	{
		static auto PlayerValue = GetEnum() ? GetEnum()->GetValue("Player") : -1;
		return PlayerValue;
	}

	static inline int64 GetContainerValue()
	{
		static auto ContainerValue = GetEnum() ? GetEnum()->GetValue("Container") : -1;
		return ContainerValue;
	}

	static inline int64 GetFloorLootValue()
	{
		static auto FloorLootValue = GetEnum() ? GetEnum()->GetValue("FloorLoot") : -1;
		return FloorLootValue;
	}

	static inline int64 GetOtherValue()
	{
		static auto OtherValue = GetEnum() ? GetEnum()->GetValue("Other") : -1;
		return OtherValue;
	}

	static inline int64 GetTossedValue()
	{
		static auto TossedValue = GetEnum() ? GetEnum()->GetValue("Tossed") : -1;
		return TossedValue;
	}
}

namespace EFortPickupSpawnSource
{
	static inline UEnum* GetEnum()
	{
		static auto Enum = FindObject<UEnum>(L"/Script/FortniteGame.EFortPickupSpawnSource");
		return Enum;
	}

	static inline int64 GetPlayerEliminationValue()
	{
		static auto PlayerEliminationValue = GetEnum() ? GetEnum()->GetValue("PlayerElimination") : -1;
		return PlayerEliminationValue;
	}

	static inline int64 GetSupplyDropValue()
	{
		static auto SupplyDropValue = GetEnum() ? GetEnum()->GetValue("SupplyDrop") : -1;
		return SupplyDropValue;
	}
}

struct PickupCreateData
{
	FFortItemEntry* ItemEntry = nullptr;
	AFortPawn* PawnOwner = nullptr;
	int OverrideCount = -1;
	UClass* OverrideClass = nullptr;
	bool bToss = false;
	class AFortPickup* IgnoreCombineTarget = nullptr;
	bool bRandomRotation = false;
	uint8 SourceType = 0;
	uint8 Source = 0;
	FVector SpawnLocation = FVector(0, 0, 0);
	bool bShouldFreeItemEntryWhenDeconstructed = false;

	~PickupCreateData()
	{
		if (bShouldFreeItemEntryWhenDeconstructed)
		{
			// real

			FFortItemEntry::FreeItemEntry(ItemEntry);

			if (bUseFMemoryRealloc)
			{
				static void (*FreeOriginal)(void* Original) = decltype(FreeOriginal)(Addresses::Free);

				if (FreeOriginal)
					FreeOriginal(ItemEntry);
			}
			else
			{
				VirtualFree(ItemEntry, 0, MEM_RELEASE);
			}
		}
	}
};

enum class EFortPickupTossState : uint8
{
	NotTossed = 0,
	InProgress = 1,
	AtRest = 2,
	EFortPickupTossState_MAX = 3,
};

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

	EFortPickupTossState& GetTossState()
	{
		static auto TossStateOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "TossState");
		return *(EFortPickupTossState*)(__int64(this) + TossStateOffset);
	}

	AFortPawn*& GetItemOwner()
	{
		static auto ItemOwnerOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "ItemOwner");
		return *(AFortPawn**)(__int64(this) + ItemOwnerOffset);
	}

	class AFortPickup*& GetCombineTarget()
	{
		static auto CombineTargetOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "CombineTarget");
		return *(AFortPickup**)(__int64(this) + CombineTargetOffset);
	}

	FVector& GetStartDirection()
	{
		static auto StartDirectionOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "StartDirection");
		return *(FVector*)(__int64(this) + StartDirectionOffset);
	}

	FVector& GetFinalTossRestLocation()
	{
		static auto FinalTossRestLocationOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "FinalTossRestLocation");
		return *(FVector*)(__int64(this) + FinalTossRestLocationOffset);
	}

	FVector& GetLootInitialPosition()
	{
		static auto LootInitialPositionOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "LootInitialPosition");
		return *(FVector*)(__int64(this) + LootInitialPositionOffset);
	}

	FVector& GetLootFinalPosition()
	{
		static auto LootFinalPositionOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "LootFinalPosition");
		return *(FVector*)(__int64(this) + LootFinalPositionOffset);
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

	void TossPickup(FVector FinalLocation, class AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, uint8 InPickupSourceTypeFlags, uint8 InPickupSpawnSource);
	void SpawnMovementComponent(); // BAD You probably don't wanna use unless absolutely necessary

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

	void OnRep_PickupLocationData()
	{
		static auto OnRep_PickupLocationDataFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_PickupLocationData");
		this->ProcessEvent(OnRep_PickupLocationDataFn);
	}

	static AFortPickup* SpawnPickup(PickupCreateData& PickupData);
	
	static AFortPickup* SpawnPickup(FFortItemEntry* ItemEntry, FVector Location, 
		uint8 PickupSource = 0, uint8 SpawnSource = 0,
		class AFortPawn* Pawn = nullptr, UClass* OverrideClass = nullptr, bool bToss = true, int OverrideCount = -1, AFortPickup* IgnoreCombinePickup = nullptr);

	static void CombinePickupHook(AFortPickup* Pickup);
	static char CompletePickupAnimationHook(AFortPickup* Pickup);

	static UClass* StaticClass();
};