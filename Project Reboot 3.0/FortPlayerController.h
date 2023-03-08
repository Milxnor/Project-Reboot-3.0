#pragma once

#include "PlayerController.h"
#include "FortInventory.h"
#include "FortPawn.h"

#include "Rotator.h"
#include "BuildingSMActor.h"

struct FCreateBuildingActorData { uint32_t BuildingClassHandle; FVector BuildLoc; FRotator BuildRot; bool bMirrored; };

struct FFortAthenaLoadout
{
	UObject*& GetCharacter()
	{
		static auto CharacterOffset = FindOffsetStruct("/Script/FortniteGame.FortAthenaLoadout", "Character");
		return *(UObject**)(__int64(this) + CharacterOffset);
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

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerController");
		return Class;
	}

	FFortAthenaLoadout* GetCosmeticLoadout()
	{
		static auto CosmeticLoadoutPCOffset = this->GetOffset("CosmeticLoadoutPC", false);

		if (CosmeticLoadoutPCOffset == 0)
			CosmeticLoadoutPCOffset = this->GetOffset("CustomizationLoadout");

		auto CosmeticLoadout = this->GetPtr<FFortAthenaLoadout>(CosmeticLoadoutPCOffset);

		return CosmeticLoadout;
	}

	static void ServerExecuteInventoryItemHook(AFortPlayerController* PlayerController, FGuid ItemGuid);

	static void ServerAttemptAircraftJumpHook(AFortPlayerController* PC, FRotator ClientRotation);
	static void ServerCreateBuildingActorHook(AFortPlayerController* PlayerController, FCreateBuildingActorData CreateBuildingData);
	
	static void ServerPlayEmoteItemHook(AFortPlayerController* PlayerController, UObject* EmoteAsset);

	static void ServerBeginEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit);
	static void ServerEditBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int RotationIterations, char bMirrored);
	static void ServerEndEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing);
};