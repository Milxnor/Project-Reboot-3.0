#pragma once

#include "Object.h"

#include "FortResourceItemDefinition.h"
#include "FortPlayerController.h"
#include "BuildingSMActor.h"
#include "FortPickup.h"

class UFortKismetLibrary : public UObject
{
public:
	using UFortInventoryOwnerInterface = UObject;

	static inline void (*K2_GiveItemToPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveItemFromPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveItemFromPlayerByGuidOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*GiveItemToInventoryOwnerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*K2_RemoveFortItemFromPlayerOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline AFortPickup* (*K2_SpawnPickupInWorldOriginal)(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static inline bool (*PickLootDropsOriginal)(UObject* Context, FFrame& Stack, bool* Ret);

	static UFortResourceItemDefinition* K2_GetResourceItemDefinition(EFortResourceType ResourceType);
	static void ApplyCharacterCosmetics(UObject* WorldContextObject, const TArray<UObject*>& CharacterParts, UObject* PlayerState, bool* bSuccess);

	static void GiveItemToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveItemFromPlayerByGuidHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_GiveItemToPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static void K2_RemoveFortItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret);
	static AFortPickup* K2_SpawnPickupInWorldHook(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static bool PickLootDropsHook(UObject* Context, FFrame& Stack, bool* Ret);

	static UClass* StaticClass();
};