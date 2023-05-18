#pragma once

#include "Object.h"

#include "FortResourceItemDefinition.h"
#include "FortPlayerController.h"
#include "BuildingSMActor.h"
#include "FortPickup.h"

using UFortInventoryOwnerInterface = UObject;

struct FSpawnItemVariantParams
{
	static UStruct* GetStruct()
	{
		static auto SpawnItemVariantsParamsStruct = FindObject<UStruct>("/Script/FortniteGame.SpawnItemVariantParams");
		return SpawnItemVariantsParamsStruct;
	}

	static int GetStructSize()
	{
		static auto StructSize = GetStruct()->GetPropertiesSize();
		return StructSize;
	}

	UFortWorldItemDefinition*& GetWorldItemDefinition()
	{
		static auto WorldItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "WorldItemDefinition");
		return *(UFortWorldItemDefinition**)(__int64(this) + WorldItemDefinitionOffset);
	}

	int& GetNumberToSpawn()
	{
		static auto NumberToSpawnOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "NumberToSpawn");
		return *(int*)(__int64(this) + NumberToSpawnOffset);
	}

	uint8& GetSourceType()
	{
		static auto SourceTypeOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "SourceType");
		return *(uint8*)(__int64(this) + SourceTypeOffset);
	}

	uint8& GetSource()
	{
		static auto SourceOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "Source");
		return *(uint8*)(__int64(this) + SourceOffset);
	}

	FVector& GetDirection()
	{
		static auto DirectionOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "Direction");
		return *(FVector*)(__int64(this) + DirectionOffset);
	}

	FVector& GetPosition()
	{
		static auto PositionOffset = FindOffsetStruct("/Script/FortniteGame.SpawnItemVariantParams", "Position");

		if (PositionOffset == -1)
			PositionOffset = 0x2C; // wtf

		return *(FVector*)(__int64(this) + PositionOffset);
	}
};

class UFortKismetLibrary : public UObject
{
public:
	static inline void (*K2_GiveItemToPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveItemFromPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveItemFromPlayerByGuidOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*GiveItemToInventoryOwnerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveFortItemFromPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline AFortPickup* (*K2_SpawnPickupInWorldOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static inline bool (*PickLootDropsOriginal)(UObject* Context, FFrame& Stack, bool* Ret);
	static inline AFortPickup* (*K2_SpawnPickupInWorldWithClassOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static inline void (*CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_SpawnPickupInWorldWithLootTierOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline bool (*SpawnInstancedPickupInWorldOriginal)(UObject* Context, FFrame& Stack, bool* Ret);
	static inline void (*SpawnItemVariantPickupInWorldOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_GiveBuildingResourceOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*PickLootDropsWithNamedWeightsOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline UObject* (*GetAIDirectorOriginal)(UObject* Context, FFrame& Stack, UObject** Ret);
	static inline UObject* (*GetAIGoalManagerOriginal)(UObject* Context, FFrame& Stack, UObject** Ret);

	static UFortResourceItemDefinition* K2_GetResourceItemDefinition(EFortResourceType ResourceType);
	static void ApplyCharacterCosmetics(UObject* WorldContextObject, const TArray<UObject*>& CharacterParts, UObject* PlayerState, bool* bSuccess);
	static FVector FindGroundLocationAt(UWorld* World, AActor* IgnoreActor, FVector InLocation, float TraceStartZ, float TraceEndZ, FName TraceName);

	static void PickLootDropsWithNamedWeightsHook(UObject* Context, FFrame& Stack, void* Ret);
	static void SpawnItemVariantPickupInWorldHook(UObject* Context, FFrame& Stack, void* Ret);
	static bool SpawnInstancedPickupInWorldHook(UObject* Context, FFrame& Stack, bool* Ret);
	static void K2_SpawnPickupInWorldWithLootTierHook(UObject* Context, FFrame& Stack, void* Ret);
	static void CreateTossAmmoPickupForWeaponItemDefinitionAtLocationHook(UObject* Context, FFrame& Stack, void* Ret);
	static void GiveItemToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveItemFromPlayerByGuidHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_GiveItemToPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_GiveBuildingResourceHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveFortItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static AFortPickup* K2_SpawnPickupInWorldHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static AFortPickup* K2_SpawnPickupInWorldWithClassHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static UObject* GetAIDirectorHook(UObject* Context, FFrame& Stack, UObject** Ret);
	static UObject* GetAIGoalManagerHook(UObject* Context, FFrame& Stack, UObject** Ret);
	static bool PickLootDropsHook(UObject* Context, FFrame& Stack, bool* Ret);

	static UClass* StaticClass();
};