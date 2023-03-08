#include "FortPlayerController.h"

#include "Rotator.h"
#include "BuildingSMActor.h"
#include "FortGameModeAthena.h"

#include "FortPlayerState.h"
#include "BuildingWeapons.h"

#include "ActorComponent.h"
#include "FortPlayerStateAthena.h"
#include "globals.h"

void AFortPlayerController::ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ClientReportDamagedResourceBuilding");

	struct { ABuildingSMActor* BuildingSMActor; EFortResourceType PotentialResourceType; int PotentialResourceCount; bool bDestroyed; bool bJustHitWeakspot; }
	AFortPlayerController_ClientReportDamagedResourceBuilding_Params{BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakspot};

	this->ProcessEvent(fn, &AFortPlayerController_ClientReportDamagedResourceBuilding_Params);
}

void AFortPlayerController::ServerExecuteInventoryItemHook(AFortPlayerController* PlayerController, FGuid ItemGuid)
{
	auto ItemInstance = PlayerController->GetWorldInventory()->FindItemInstance(ItemGuid);
	auto Pawn = Cast<AFortPawn>(PlayerController->GetPawn());

	if (!ItemInstance || !Pawn)
		return;

	auto ItemDefinition = ItemInstance->GetItemEntry()->GetItemDefinition();

	if (auto DecoItemDefinition = Cast<UFortDecoItemDefinition>(ItemDefinition))
	{
		Pawn->PickUpActor(nullptr, DecoItemDefinition); // todo check ret value?
		Pawn->GetCurrentWeapon()->GetItemEntryGuid() = ItemGuid;

		static auto FortDecoTool_ContextTrapStaticClass = FindObject<UClass>("/Script/FortniteGame.FortDecoTool_ContextTrap");

		if (Pawn->GetCurrentWeapon()->IsA(FortDecoTool_ContextTrapStaticClass))
		{
			static auto ContextTrapItemDefinitionOffset = Pawn->GetCurrentWeapon()->GetOffset("ContextTrapItemDefinition");
			Pawn->GetCurrentWeapon()->Get<UObject*>(ContextTrapItemDefinitionOffset) = DecoItemDefinition;
		}

		return;
	}

	if (!ItemDefinition)
		return;

	if (auto Weapon = Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemDefinition, ItemInstance->GetItemEntry()->GetItemGuid()))
	{

	}
}

void AFortPlayerController::ServerAttemptAircraftJumpHook(AFortPlayerController* PC, FRotator ClientRotation)
{
	if (Fortnite_Version == 17.30 && Globals::bGoingToPlayEvent)
		return; // We want to be teleported back to the UFO but we dont use chooseplayerstart

	auto PlayerController = Cast<APlayerController>(Engine_Version < 424 ? PC : ((UActorComponent*)PC)->GetOwner());

	LOG_INFO(LogDev, "PlayerController: {}", __int64(PlayerController));

	if (!PlayerController)
		return;

	// if (!PlayerController->bInAircraft) 
		// return;

	auto GameMode = (AFortGameModeAthena*)GetWorld()->GetGameMode();
	auto GameState = GameMode->GetGameStateAthena();

	static auto AircraftsOffset = GameState->GetOffset("Aircrafts");
	auto Aircrafts = GameState->GetPtr<TArray<AActor*>>(AircraftsOffset);

	if (Aircrafts->Num() <= 0)
		return;

	auto NewPawn = GameMode->SpawnDefaultPawnForHook(GameMode, (AController*)PlayerController, Aircrafts->at(0));
	PlayerController->Possess(NewPawn);

	// PC->ServerRestartPlayer();
}

void AFortPlayerController::ServerCreateBuildingActorHook(AFortPlayerController* PlayerController, FCreateBuildingActorData CreateBuildingData)
{
	auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	if (!PlayerStateAthena)
		return;

	UClass* BuildingClass = nullptr;
	FVector BuildLocation;
	FRotator BuildRotator;
	bool bMirrored;

	if (Fortnite_Version >= 8.30)
	{
		BuildLocation = CreateBuildingData.BuildLoc;
		BuildRotator = CreateBuildingData.BuildRot;
		bMirrored = CreateBuildingData.bMirrored;

		static auto BroadcastRemoteClientInfoOffset = PlayerController->GetOffset("BroadcastRemoteClientInfo");
		auto BroadcastRemoteClientInfo = PlayerController->Get(BroadcastRemoteClientInfoOffset);

		static auto RemoteBuildableClassOffset = BroadcastRemoteClientInfo->GetOffset("RemoteBuildableClass");
		BuildingClass = BroadcastRemoteClientInfo->Get<UClass*>(RemoteBuildableClassOffset);
	}
	else
	{

	}

	// LOG_INFO(LogDev, "BuildingClass {}", __int64(BuildingClass));

	if (!BuildingClass)
		return;

	TArray<ABuildingSMActor*> ExistingBuildings;
	char idk;
	static __int64 (*CantBuild)(UObject*, UObject*, FVector, FRotator, char, TArray<ABuildingSMActor*>*, char*) = decltype(CantBuild)(Addresses::CantBuild);
	bool bCanBuild = !CantBuild(GetWorld(), BuildingClass, BuildLocation, BuildRotator, bMirrored, &ExistingBuildings, &idk);

	if (!bCanBuild)
	{
		// LOG_INFO(LogDev, "cant build");
		return;
	}
	
	for (int i = 0; i < ExistingBuildings.Num(); i++)
	{
		auto ExistingBuilding = ExistingBuildings.At(i);

		ExistingBuilding->K2_DestroyActor();
	}

	ExistingBuildings.Free();

	FTransform Transform{};
	Transform.Translation = BuildLocation;
	Transform.Rotation = BuildRotator.Quaternion();
	Transform.Scale3D = { 1, 1, 1 };

	auto BuildingActor = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingClass, Transform);

	if (!BuildingActor)
		return;

	BuildingActor->SetPlayerPlaced(true);
	BuildingActor->SetTeam(PlayerStateAthena->GetTeamIndex());
	BuildingActor->InitializeBuildingActor(PlayerController, BuildingActor, true);
}

void AFortPlayerController::ServerPlayEmoteItemHook(AFortPlayerController* PlayerController, UObject* EmoteAsset)
{
	auto PlayerState = (AFortPlayerStateAthena*)PlayerController->GetPlayerState();
	auto Pawn = PlayerController->GetPawn();

	if (!EmoteAsset || !PlayerState || !Pawn)
		return;

	UObject* AbilityToUse = nullptr;

	if (!AbilityToUse)
	{
		static auto EmoteGameplayAbilityDefault = FindObject("/Game/Abilities/Emotes/GAB_Emote_Generic.Default__GAB_Emote_Generic_C");
		AbilityToUse = EmoteGameplayAbilityDefault;
	}

	if (!AbilityToUse)
		return;

	int outHandle = 0;

	FGameplayAbilitySpecHandle Handle{};
	Handle.GenerateNewHandle();

	FGameplayAbilitySpec* Spec = MakeNewSpec((UClass*)AbilityToUse, EmoteAsset, true);

	static unsigned int* (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent * ASC, int* outHandle, __int64 Spec)
		= decltype(GiveAbilityAndActivateOnce)(Addresses::GiveAbilityAndActivateOnce);

	GiveAbilityAndActivateOnce(PlayerState->GetAbilitySystemComponent(), &outHandle, __int64(Spec));
}

void AFortPlayerController::ServerBeginEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
	if (!BuildingActorToEdit || !BuildingActorToEdit->IsPlayerPlaced())
		return;

	auto Pawn = Cast<AFortPawn>(PlayerController->GetPawn(), false);

	if (!Pawn)
		return;

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

	if (auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(EditToolDef, FGuid{})))
	{
		BuildingActorToEdit->GetEditingPlayer() = PlayerController->GetPlayerState();
		// Onrep?

		EditTool->GetEditActor() = BuildingActorToEdit;
		// Onrep?
	}
}

void AFortPlayerController::ServerEditBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int RotationIterations, char bMirrored)
{
	auto PlayerState = (AFortPlayerState*)PlayerController->GetPlayerState();

	if (!BuildingActorToEdit || !NewBuildingClass || BuildingActorToEdit->IsDestroyed() || BuildingActorToEdit->GetEditingPlayer() != PlayerState)
		return;

	// if (!PlayerState || PlayerState->GetTeamIndex() != BuildingActorToEdit->GetTeamIndex()) 
		// return;

	BuildingActorToEdit->GetEditingPlayer() = nullptr;

	static ABuildingSMActor* (*BuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) =
		decltype(BuildingSMActorReplaceBuildingActor)(Addresses::ReplaceBuildingActor);

	if (auto BuildingActor = BuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass, 
		BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, PlayerController))
	{
		BuildingActor->SetPlayerPlaced(true);

		if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
			BuildingActor->SetTeam(PlayerState->GetTeamIndex());

		// BuildingActor->OnRep_Team();
	}
}

void AFortPlayerController::ServerEndEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing)
{
	auto Pawn = PlayerController->GetMyFortPawn();

	if (!BuildingActorToStopEditing || !Pawn
		|| BuildingActorToStopEditing->GetEditingPlayer() != PlayerController->GetPlayerState()
		|| BuildingActorToStopEditing->IsDestroyed())
		return;

	auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->GetCurrentWeapon());

	BuildingActorToStopEditing->GetEditingPlayer() = nullptr;
	// BuildingActorToStopEditing->OnRep_EditingPlayer();

	if (EditTool)
	{
		EditTool->GetEditActor() = nullptr;
		// EditTool->OnRep_EditActor();
	}
}