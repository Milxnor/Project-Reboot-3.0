#pragma once

#include "PlayerController.h"
#include "FortInventory.h"
#include "FortPawn.h"

#include "Rotator.h"
#include "BuildingSMActor.h"
#include "Stack.h"

struct FFortAthenaLoadout
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.FortAthenaLoadout");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	UObject*& GetCharacter()
	{
		static auto CharacterOffset = FindOffsetStruct("/Script/FortniteGame.FortAthenaLoadout", "Character");
		return *(UObject**)(__int64(this) + CharacterOffset);
	}

	UObject*& GetBackpack()
	{
		static auto BackpackOffset = FindOffsetStruct("/Script/FortniteGame.FortAthenaLoadout", "Backpack");
		return *(UObject**)(__int64(this) + BackpackOffset);
	}

	UObject*& GetPickaxe()
	{
		static auto PickaxeOffset = FindOffsetStruct("/Script/FortniteGame.FortAthenaLoadout", "Pickaxe");
		return *(UObject**)(__int64(this) + PickaxeOffset);
	}
};

class AFortPlayerController : public APlayerController
{
public:
	static inline void (*ClientOnPawnDiedOriginal)(AFortPlayerController* PlayerController, void* DeathReport);
	static inline void (*ServerCreateBuildingActorOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*ServerAttemptInteractOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*ServerEditBuildingActorOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*DropSpecificItemOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline AActor* (*SpawnToyInstanceOriginal)(UObject* Context, FFrame* Stack, AActor** Ret);

	void ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot);

	AFortInventory*& GetWorldInventory()
	{
		static auto WorldInventoryOffset = GetOffset("WorldInventory");
		return Get<AFortInventory*>(WorldInventoryOffset);
	}

	AFortPawn*& GetMyFortPawn() // AFortPlayerPawn
	{
		static auto MyFortPawnOffset = GetOffset("MyFortPawn");
		return Get<AFortPawn*>(MyFortPawnOffset);
	}

	FFortAthenaLoadout* GetCosmeticLoadout()
	{
		static auto CosmeticLoadoutPCOffset = this->GetOffset("CosmeticLoadoutPC", false);

		if (CosmeticLoadoutPCOffset == -1)
			CosmeticLoadoutPCOffset = this->GetOffset("CustomizationLoadout");

		if (CosmeticLoadoutPCOffset == -1)
			return nullptr;

		auto CosmeticLoadout = this->GetPtr<FFortAthenaLoadout>(CosmeticLoadoutPCOffset);

		return CosmeticLoadout;
	}

	bool DoesBuildFree();
	void DropAllItems(const std::vector<UFortItemDefinition*>& IgnoreItemDefs, bool bIgnoreSecondaryQuickbar = false, bool bRemoveIfNotDroppable = false);

	static void ServerExecuteInventoryItemHook(AFortPlayerController* PlayerController, FGuid ItemGuid);
	static void ServerAttemptInteractHook(UObject* Context, FFrame* Stack, void* Ret);

	static void ServerAttemptAircraftJumpHook(AFortPlayerController* PC, FRotator ClientRotation);
	// static void ServerCreateBuildingActorHook(AFortPlayerController* PlayerController, FCreateBuildingActorData CreateBuildingData);
	static void ServerCreateBuildingActorHook(UObject* Context, FFrame* Stack, void* Ret);
	static AActor* SpawnToyInstanceHook(UObject* Context, FFrame* Stack, AActor** Ret);
	static void DropSpecificItemHook(UObject* Context, FFrame& Stack, void* Ret);

	static void ServerDropAllItemsHook(AFortPlayerController* PlayerController, UFortItemDefinition* IgnoreItemDef);

	static void ServerAttemptInventoryDropHook(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count);
	static void ServerPlayEmoteItemHook(AFortPlayerController* PlayerController, UObject* EmoteAsset);
	static void ClientOnPawnDiedHook(AFortPlayerController* PlayerController, void* DeathReport);

	static void ServerBeginEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
	// static void ServerEditBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int RotationIterations, char bMirrored);
	static void ServerEditBuildingActorHook(UObject* Context, FFrame& Stack, void* Ret);
	static void ServerEndEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerController");
		return Class;
	}
};