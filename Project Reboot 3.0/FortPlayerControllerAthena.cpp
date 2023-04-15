#include "FortPlayerControllerAthena.h"
#include "FortPlayerPawn.h"
#include "FortKismetLibrary.h"

#include "SoftObjectPtr.h"
#include "globals.h"
#include "GameplayStatics.h"
#include "hooking.h"

void AFortPlayerControllerAthena::ServerRequestSeatChangeHook(AFortPlayerControllerAthena* PlayerController, int TargetSeatIndex)
{
	auto Pawn = Cast<AFortPlayerPawn>(PlayerController->GetPawn());
	
	if (!Pawn)
		return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);

	auto Vehicle = Pawn->GetVehicle();

	if (!Vehicle)
		return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);

	auto OldVehicleWeaponDefinition = Pawn->GetVehicleWeaponDefinition(Vehicle);
	
	LOG_INFO(LogDev, "OldVehicleWeaponDefinition: {}", OldVehicleWeaponDefinition ? OldVehicleWeaponDefinition->GetFullName() : "BadRead");

	if (!OldVehicleWeaponDefinition)
		return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);

	auto OldVehicleWeaponInstance = WorldInventory->FindItemInstance(OldVehicleWeaponDefinition);

	if (OldVehicleWeaponInstance)
	{
		bool bShouldUpdate = false;
		WorldInventory->RemoveItem(OldVehicleWeaponInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, OldVehicleWeaponInstance->GetItemEntry()->GetCount(), true);

		if (bShouldUpdate)
			WorldInventory->Update();
	}

	auto RequestingVehicleWeaponDefinition = Vehicle->GetVehicleWeaponForSeat(TargetSeatIndex);

	if (!RequestingVehicleWeaponDefinition)
	{
		auto PickaxeInstance = WorldInventory->GetPickaxeInstance();

		if (!PickaxeInstance)
			return;

		AFortPlayerController::ServerExecuteInventoryItemHook(PlayerController, PickaxeInstance->GetItemEntry()->GetItemGuid()); // Bad, we should equip the last weapon.
		return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);
	}

	auto NewAndModifiedInstances = WorldInventory->AddItem(RequestingVehicleWeaponDefinition, nullptr);
	auto RequestedVehicleInstance = NewAndModifiedInstances.first[0];

	if (!RequestedVehicleInstance)
		return;

	WorldInventory->Update();

	auto RequestedVehicleWeapon = Pawn->EquipWeaponDefinition(RequestingVehicleWeaponDefinition, RequestedVehicleInstance->GetItemEntry()->GetItemGuid());

	return ServerRequestSeatChangeOriginal(PlayerController, TargetSeatIndex);
}

void AFortPlayerControllerAthena::ServerRestartPlayerHook(AFortPlayerControllerAthena* Controller)
{
	static auto FortPlayerControllerZoneDefault = FindObject<UClass>(L"/Script/FortniteGame.Default__FortPlayerControllerZone");
	static auto ServerRestartPlayerFn = FindObject<UFunction>(L"/Script/Engine.PlayerController.ServerRestartPlayer");
	static auto ZoneServerRestartPlayer = __int64(FortPlayerControllerZoneDefault->VFTable[GetFunctionIdxOrPtr(ServerRestartPlayerFn) / 8]);
	static void (*ZoneServerRestartPlayerOriginal)(AFortPlayerController*) = decltype(ZoneServerRestartPlayerOriginal)(__int64(ZoneServerRestartPlayer));
	
	LOG_INFO(LogDev, "Call 0x{:x}!", ZoneServerRestartPlayer - __int64(_ReturnAddress()));
	return ZoneServerRestartPlayerOriginal(Controller);
}

void AFortPlayerControllerAthena::ServerGiveCreativeItemHook(AFortPlayerControllerAthena* Controller, FFortItemEntry CreativeItem)
{
	// Don't worry, the validate has a check if it is a creative enabled mode or not, but we need to add a volume check and permission check I think.

	auto CreativeItemPtr = &CreativeItem;
	auto ItemDefinition = CreativeItemPtr->GetItemDefinition();

	if (!ItemDefinition)
		return;

	bool bShouldUpdate = false;
	auto LoadedAmmo = -1; // CreativeItemPtr->GetLoadedAmmo()
	Controller->GetWorldInventory()->AddItem(ItemDefinition, &bShouldUpdate, CreativeItemPtr->GetCount(), LoadedAmmo, false);

	if (bShouldUpdate)
		Controller->GetWorldInventory()->Update(Controller);
}

void AFortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIslandHook(AFortPlayerControllerAthena* Controller)
{
	auto Pawn = Controller->GetMyFortPawn();

	if (!Pawn)
		return;

	static auto FortPlayerStartCreativeClass = FindObject<UClass>("/Script/FortniteGame.FortPlayerStartCreative");
	auto AllCreativePlayerStarts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortPlayerStartCreativeClass);

	for (int i = 0; i < AllCreativePlayerStarts.Num(); i++)
	{
		auto CurrentPlayerStart = AllCreativePlayerStarts.at(i);

		static auto PlayerStartTagsOffset = CurrentPlayerStart->GetOffset("PlayerStartTags");
		auto bHasSpawnTag = CurrentPlayerStart->Get<FGameplayTagContainer>(PlayerStartTagsOffset).Contains("Playground.LobbyIsland.Spawn");

		if (!bHasSpawnTag)
			continue;

		Pawn->TeleportTo(CurrentPlayerStart->GetActorLocation(), Pawn->GetActorRotation());
		break;
	}

	AllCreativePlayerStarts.Free();
}

void AFortPlayerControllerAthena::ServerAcknowledgePossessionHook(APlayerController* Controller, APawn* Pawn)
{
	static auto AcknowledgedPawnOffset = Controller->GetOffset("AcknowledgedPawn");
	Controller->Get<APawn*>(AcknowledgedPawnOffset) = Pawn;

	auto ControllerAsFort = Cast<AFortPlayerController>(Controller);
	auto PawnAsFort = Cast<AFortPlayerPawn>(Pawn);
	auto PlayerStateAsFort = Cast<AFortPlayerState>(Pawn->GetPlayerState());

	if (!PawnAsFort)
		return;

	if (Globals::bNoMCP)
	{
		static auto CustomCharacterPartClass = FindObject<UClass>("/Script/FortniteGame.CustomCharacterPart");
		static auto backpackPart = LoadObject("/Game/Characters/CharacterParts/Backpacks/NoBackpack.NoBackpack", CustomCharacterPartClass);

		// PawnAsFort->ServerChoosePart(EFortCustomPartType::Backpack, backpackPart);

		return;
	}

	ControllerAsFort->ApplyCosmeticLoadout();
}

void AFortPlayerControllerAthena::ServerPlaySquadQuickChatMessageHook(AFortPlayerControllerAthena* PlayerController, __int64 ChatEntry, __int64 SenderID)
{
	using UAthenaEmojiItemDefinition = UFortItemDefinition;

	static auto EmojiComm = FindObject<UAthenaEmojiItemDefinition>("/Game/Athena/Items/Cosmetics/Dances/Emoji/Emoji_Comm.Emoji_Comm");
	PlayerController->ServerPlayEmoteItemHook(PlayerController, EmojiComm);
}

void AFortPlayerControllerAthena::GetPlayerViewPointHook(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation)
{
	// I don't know why but GetActorEyesViewPoint only works on some versions.
	/* static auto GetActorEyesViewPointFn = FindObject<UFunction>(L"/Script/Engine.Actor.GetActorEyesViewPoint");
	static auto GetActorEyesViewPointIndex = GetFunctionIdxOrPtr(GetActorEyesViewPointFn) / 8;

	void (*GetActorEyesViewPointOriginal)(AActor* Actor, FVector* OutLocation, FRotator* OutRotation) = decltype(GetActorEyesViewPointOriginal)(PlayerController->VFTable[GetActorEyesViewPointIndex]);
	return GetActorEyesViewPointOriginal(PlayerController, &Location, &Rotation); */

	if (auto MyFortPawn = PlayerController->GetMyFortPawn())
	{
		Location = MyFortPawn->GetActorLocation();
		Rotation = PlayerController->GetControlRotation();
		return;
	}

	return AFortPlayerControllerAthena::GetPlayerViewPointOriginal(PlayerController, Location, Rotation);
}

void AFortPlayerControllerAthena::ServerReadyToStartMatchHook(AFortPlayerControllerAthena* PlayerController)
{
	LOG_INFO(LogDev, "ServerReadyToStartMatch!");

	if (Fortnite_Version <= 2.5) // techinally we should do this at the end of OnReadyToStartMatch
	{
		static auto QuickBarsOffset = PlayerController->GetOffset("QuickBars", false);

		if (QuickBarsOffset != -1)
		{
			auto& QuickBars = PlayerController->Get<AActor*>(QuickBarsOffset);

			LOG_INFO(LogDev, "QuickBarsOld: {}", __int64(QuickBars));

			if (QuickBars)
				return ServerReadyToStartMatchOriginal(PlayerController);

			static auto FortQuickBarsClass = FindObject<UClass>("/Script/FortniteGame.FortQuickBars");

			QuickBars = GetWorld()->SpawnActor<AActor>(FortQuickBarsClass);

			LOG_INFO(LogDev, "QuickBarsNew: {}", __int64(QuickBars));

			if (!QuickBars)
				return ServerReadyToStartMatchOriginal(PlayerController);

			PlayerController->Get<AActor*>(QuickBarsOffset)->SetOwner(PlayerController);
		}
	}

	return ServerReadyToStartMatchOriginal(PlayerController);
}