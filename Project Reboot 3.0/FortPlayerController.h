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
	static inline void (*ServerLoadingScreenDroppedOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*ServerAttemptAircraftJumpOriginal)(AFortPlayerController* PC, FRotator ClientRotation);

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

	int GetCosmeticLoadoutOffset()
	{
		static auto CosmeticLoadoutPCOffset = this->GetOffset("CosmeticLoadoutPC", false);

		if (CosmeticLoadoutPCOffset == -1)
			CosmeticLoadoutPCOffset = this->GetOffset("CustomizationLoadout", false);

		if (CosmeticLoadoutPCOffset == -1)
			return -1;

		return CosmeticLoadoutPCOffset;
	}

	FFortAthenaLoadout* GetCosmeticLoadout()
	{
		static auto CosmeticLoadoutPCOffset = GetCosmeticLoadoutOffset();
		auto CosmeticLoadout = this->GetPtr<FFortAthenaLoadout>(CosmeticLoadoutPCOffset);

		return CosmeticLoadout;
	}

	UFortItem* AddPickaxeToInventory()
	{
		auto CosmeticLoadout = GetCosmeticLoadout();
		auto CosmeticLoadoutPickaxe = CosmeticLoadout ? CosmeticLoadout->GetPickaxe() : nullptr;
	
		static auto WeaponDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.AthenaPickaxeItemDefinition", "WeaponDefinition");

		auto PickaxeDefinition = CosmeticLoadoutPickaxe ? CosmeticLoadoutPickaxe->Get<UFortItemDefinition*>(WeaponDefinitionOffset)
			: FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

		auto WorldInventory = GetWorldInventory();

		if (!WorldInventory || WorldInventory->GetPickaxeInstance())
			return nullptr;

		auto NewAndModifiedInstances = WorldInventory->AddItem(PickaxeDefinition, nullptr);
		WorldInventory->Update();

		return NewAndModifiedInstances.first.size() > 0 ? NewAndModifiedInstances.first[0] : nullptr;
	}

	TSet<FGuid>& GetGadgetTrackedAttributeItemInstanceIds() // actually in zone
	{
		static auto GadgetTrackedAttributeItemInstanceIdsOffset = GetOffset("GadgetTrackedAttributeItemInstanceIds");
		return Get<TSet<FGuid>>(GadgetTrackedAttributeItemInstanceIdsOffset);
	}

	bool IsPlayingEmote()
	{
		static auto IsPlayingEmoteFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.IsPlayingEmote");
		bool Ret;
		this->ProcessEvent(IsPlayingEmoteFn, &Ret);
		return Ret;
	}

	bool& ShouldTryPickupSwap()
	{
		static auto bTryPickupSwapOffset = GetOffset("bTryPickupSwap");
		return Get<bool>(bTryPickupSwapOffset);
	}
	
	bool HasTryPickupSwap()
	{
		static auto bTryPickupSwapOffset = GetOffset("bTryPickupSwap", false);
		return bTryPickupSwapOffset != -1;
	}

	void ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution);

	bool DoesBuildFree();
	void DropAllItems(const std::vector<UFortItemDefinition*>& IgnoreItemDefs, bool bIgnoreSecondaryQuickbar = false, bool bRemoveIfNotDroppable = false, bool RemovePickaxe = false);
	void ApplyCosmeticLoadout();

	static void ServerSuicideHook(AFortPlayerController* PlayerController);

	static void ServerLoadingScreenDroppedHook(UObject* Context, FFrame* Stack, void* Ret);
	static void ServerRepairBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair);
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