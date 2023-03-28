#include "FortPlayerController.h"

#include "Rotator.h"
#include "BuildingSMActor.h"
#include "FortGameModeAthena.h"

#include "FortPlayerState.h"
#include "BuildingWeapons.h"

#include "ActorComponent.h"
#include "FortPlayerStateAthena.h"
#include "globals.h"
#include "FortPlayerControllerAthena.h"
#include "BuildingContainer.h"
#include "FortLootPackage.h"
#include "FortPickup.h"
#include "FortPlayerPawn.h"
#include <memcury.h>
#include "KismetStringLibrary.h"

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

	if (!ItemDefinition)
		return;

	// LOG_INFO(LogDev, "ItemDefinition: {}", ItemDefinition->GetFullName());

	static auto FortGadgetItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortGadgetItemDefinition");

	if (ItemDefinition->IsA(FortGadgetItemDefinitionClass))
	{
		static auto GetWeaponItemDefinition = FindObject<UFunction>("/Script/FortniteGame.FortGadgetItemDefinition.GetWeaponItemDefinition");

		if (GetWeaponItemDefinition)
		{
			ItemDefinition->ProcessEvent(GetWeaponItemDefinition, &ItemDefinition);
		}
		else
		{
			static auto GetDecoItemDefinition = FindObject<UFunction>("/Script/FortniteGame.FortGadgetItemDefinition.GetDecoItemDefinition");
			ItemDefinition->ProcessEvent(GetDecoItemDefinition, &ItemDefinition);
		}
	}

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

	if (Engine_Version < 420) // wtf
	{
		static auto CurrentWeaponListOffset = Pawn->GetOffset("CurrentWeaponList");
		auto& CurrentWeaponList = Pawn->Get<TArray<AFortWeapon*>>(CurrentWeaponListOffset);

		CurrentWeaponList.Data = nullptr;
		CurrentWeaponList.ArrayNum = 0;
		CurrentWeaponList.ArrayMax = 0;
	}

	if (auto Weapon = Pawn->EquipWeaponDefinition((UFortWeaponItemDefinition*)ItemDefinition, ItemInstance->GetItemEntry()->GetItemGuid()))
	{
		if (Engine_Version < 420)
		{
			static auto FortWeap_BuildingToolClass = FindObject<UClass>("/Script/FortniteGame.FortWeap_BuildingTool");

			if (!Weapon->IsA(FortWeap_BuildingToolClass))
				return;

			auto BuildingTool = Weapon;

			using UBuildingEditModeMetadata = UObject;
			using UFortBuildingItemDefinition = UObject;

			static auto OnRep_DefaultMetadataFn = FindObject<UFunction>("/Script/FortniteGame.FortWeap_BuildingTool.OnRep_DefaultMetadata");
			static auto DefaultMetadataOffset = BuildingTool->GetOffset("DefaultMetadata");

			static auto RoofPiece = FindObject<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
			static auto FloorPiece = FindObject<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
			static auto WallPiece = FindObject<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
			static auto StairPiece = FindObject<UFortBuildingItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");

			if (ItemDefinition == RoofPiece)
			{
				static auto RoofMetadata = FindObject<UBuildingEditModeMetadata>("/Game/Building/EditModePatterns/Roof/EMP_Roof_RoofC.EMP_Roof_RoofC");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = RoofMetadata;
			}
			else if (ItemDefinition == StairPiece)
			{
				static auto StairMetadata = FindObject<UBuildingEditModeMetadata>("/Game/Building/EditModePatterns/Stair/EMP_Stair_StairW.EMP_Stair_StairW");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = StairMetadata;
			}
			else if (ItemDefinition == WallPiece)
			{
				static auto WallMetadata = FindObject<UBuildingEditModeMetadata>("/Game/Building/EditModePatterns/Wall/EMP_Wall_Solid.EMP_Wall_Solid");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = WallMetadata;
			}
			else if (ItemDefinition == FloorPiece)
			{
				static auto FloorMetadata = FindObject<UBuildingEditModeMetadata>("/Game/Building/EditModePatterns/Floor/EMP_Floor_Floor.EMP_Floor_Floor");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = FloorMetadata;
			}

			BuildingTool->ProcessEvent(OnRep_DefaultMetadataFn);
		}
	}
}

void AFortPlayerController::ServerAttemptInteractHook(UObject* Context, FFrame* Stack, void* Ret)
{
	// static auto LlamaClass = FindObject<UClass>("/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C");
	static auto FortAthenaSupplyDropClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaSupplyDrop");

	LOG_INFO(LogInteraction, "ServerAttemptInteract!");

	auto Params = Stack->Locals;

	static bool bIsUsingComponent = FindObject<UClass>("/Script/FortniteGame.FortControllerComponent_Interaction");

	AFortPlayerControllerAthena* PlayerController = bIsUsingComponent ? Cast<AFortPlayerControllerAthena>(((UActorComponent*)Context)->GetOwner()) :
		Cast<AFortPlayerControllerAthena>(Context);

	if (!PlayerController)
		return;

	std::string StructName = bIsUsingComponent ? "/Script/FortniteGame.FortControllerComponent_Interaction.ServerAttemptInteract" : "/Script/FortniteGame.FortPlayerController.ServerAttemptInteract";

	static auto ReceivingActorOffset = FindOffsetStruct(StructName, "ReceivingActor");
	auto ReceivingActor = *(AActor**)(__int64(Params) + ReceivingActorOffset);

	// LOG_INFO(LogInteraction, "ReceivingActor: {}", __int64(ReceivingActor));

	if (!ReceivingActor)
		return;

	// LOG_INFO(LogInteraction, "ReceivingActor Name: {}", ReceivingActor->GetFullName());

	FVector LocationToSpawnLoot = ReceivingActor->GetActorLocation() + ReceivingActor->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 };

	static auto FortAthenaVehicleClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaVehicle");

	if (auto BuildingContainer = Cast<ABuildingContainer>(ReceivingActor))
	{
		static auto bAlreadySearchedOffset = BuildingContainer->GetOffset("bAlreadySearched");
		static auto SearchBounceDataOffset = BuildingContainer->GetOffset("SearchBounceData");
		static auto bAlreadySearchedFieldMask = GetFieldMask(BuildingContainer->GetProperty("bAlreadySearched"));
		static auto SearchAnimationCountOffset = FindOffsetStruct("/Script/FortniteGame.FortSearchBounceData", "SearchAnimationCount");
		
		auto SearchBounceData = BuildingContainer->GetPtr<void>(SearchBounceDataOffset);

		if (BuildingContainer->ReadBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask))
			return;

		// LOG_INFO(LogInteraction, "bAlreadySearchedFieldMask: {}", bAlreadySearchedFieldMask);

		BuildingContainer->SetBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask, true);
		(*(int*)(__int64(SearchBounceData) + SearchAnimationCountOffset))++;

		static auto OnRep_bAlreadySearchedFn = FindObject<UFunction>("/Script/FortniteGame.BuildingContainer.OnRep_bAlreadySearched");
		BuildingContainer->ProcessEvent(OnRep_bAlreadySearchedFn);

		static auto SearchLootTierGroupOffset = BuildingContainer->GetOffset("SearchLootTierGroup");
		auto RedirectedLootTier = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode(), false)->RedirectLootTier(BuildingContainer->Get<FName>(SearchLootTierGroupOffset));

		LOG_INFO(LogInteraction, "RedirectedLootTier: {}", RedirectedLootTier.ToString());

		auto LootDrops = PickLootDrops(RedirectedLootTier, true);

		LOG_INFO(LogInteraction, "LootDrops.size(): {}", LootDrops.size());

		for (int i = 0; i < LootDrops.size(); i++)
		{
			auto& lootDrop = LootDrops.at(i);
			AFortPickup::SpawnPickup(lootDrop.ItemDefinition, LocationToSpawnLoot, lootDrop.Count, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset, lootDrop.LoadedAmmo
				// , (AFortPawn*)PlayerController->GetPawn() // should we put this here?
			);
		}

		// if (BuildingContainer->ShouldDestroyOnSearch())
			// BuildingContainer->K2_DestroyActor();
	}
	else if (ReceivingActor->IsA(FortAthenaVehicleClass))
	{
		auto Vehicle = ReceivingActor;
		ServerAttemptInteractOriginal(Context, Stack, Ret);

		return;

		auto Pawn = (AFortPlayerPawn*)PlayerController->GetMyFortPawn();

		if (!Pawn)
			return;

		static auto FindSeatIndexFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.FindSeatIndex");
		static auto GetSeatWeaponComponentFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.GetSeatWeaponComponent");

		if (!GetSeatWeaponComponentFn)
			return;

		struct { AFortPlayerPawn* PlayerPawn; int ReturnValue; } AFortAthenaVehicle_FindSeatIndex_Params{Pawn};
		Vehicle->ProcessEvent(FindSeatIndexFn, &AFortAthenaVehicle_FindSeatIndex_Params);

		auto SeatIndex = AFortAthenaVehicle_FindSeatIndex_Params.ReturnValue;

		struct { int SeatIndex; UObject* ReturnValue; } AFortAthenaVehicle_GetSeatWeaponComponent_Params{};

		Vehicle->ProcessEvent(GetSeatWeaponComponentFn, &AFortAthenaVehicle_GetSeatWeaponComponent_Params);

		auto WeaponComponent = AFortAthenaVehicle_GetSeatWeaponComponent_Params.ReturnValue;

		if (!WeaponComponent)
			return;

		static auto WeaponSeatDefinitionStructSize = FindObject<UClass>("/Script/FortniteGame.WeaponSeatDefinition")->GetPropertiesSize();
		static auto VehicleWeaponOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "VehicleWeapon");
		static auto SeatIndexOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "SeatIndex");
		static auto WeaponSeatDefinitionsOffset = WeaponComponent->GetOffset("WeaponSeatDefinitions");
		auto& WeaponSeatDefinitions = WeaponComponent->Get<TArray<__int64>>(WeaponSeatDefinitionsOffset);

		for (int i = 0; i < WeaponSeatDefinitions.Num(); i++)
		{
			auto WeaponSeat = WeaponSeatDefinitions.AtPtr(i, WeaponSeatDefinitionStructSize);

			if (*(int*)(__int64(WeaponSeat) + SeatIndexOffset) != SeatIndex)
				continue;

			auto VehicleWeaponDef = *(UFortWeaponItemDefinition**)(__int64(WeaponSeat) + VehicleWeaponOffset);

			LOG_INFO(LogDev, "Vehicle eaponb: {}", __int64(VehicleWeaponDef));

			if (!VehicleWeaponDef)
				continue;

			LOG_INFO(LogDev, "Vehicle eaponb name: {}", VehicleWeaponDef->GetFullName());

			auto WorldInventory = PlayerController->GetWorldInventory();
			auto ahh = WorldInventory->AddItem(VehicleWeaponDef, nullptr);

			auto newitem = ahh.first[0];

			LOG_INFO(LogDev, "newitem: {}", __int64(newitem));

			if (!newitem)
				return;

			PlayerController->ServerExecuteInventoryItemHook(PlayerController, newitem->GetItemEntry()->GetItemGuid());

			WorldInventory->Update();
			break;
		}

		return;
	}
	/*
	else if (ReceivingActor->IsA(FortAthenaSupplyDropClass))
	{
		auto LootTierGroup = ReceivingActor->IsA(LlamaClass) ? UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaLlama") : UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaSupplyDrop"); // SupplyDrop->GetLootTierGroupOverride();

		auto LootDrops = PickLootDrops(LootTierGroup);

		for (auto& LootDrop : LootDrops)
		{
			AFortPickup::SpawnPickup(LootDrop.ItemDefinition, LocationToSpawnLoot, LootDrop.Count, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::SupplyDrop);
		}
	}
	*/

	return ServerAttemptInteractOriginal(Context, Stack, Ret);
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

	auto NewPawnAsFort = Cast<AFortPawn>(NewPawn);

	if (NewPawnAsFort)
		NewPawnAsFort->SetHealth(100);

	// PlayerController->ServerRestartPlayer();
}

void AFortPlayerController::ServerDropAllItemsHook(AFortPlayerController* PlayerController, UFortItemDefinition* IgnoreItemDef)
{
	auto Pawn = PlayerController->GetMyFortPawn();

	if (!Pawn)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();
	auto Location = Pawn->GetActorLocation();

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		auto ItemInstance = ItemInstances.at(i);

		if (!ItemInstance)
			continue;

		auto ItemEntry = ItemInstance->GetItemEntry();
		auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->GetItemDefinition());

		if (!WorldItemDefinition)
			continue;

		// if (!WorldItemDefinition->ShouldDropOnDeath())
			// continue;

		AFortPickup::SpawnPickup(WorldItemDefinition, Location, ItemEntry->GetCount(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination,
			ItemEntry->GetLoadedAmmo()); // I do playerelimation flag here ik we shouldnt but it makes it go in circle iirc

		WorldInventory->RemoveItem(ItemEntry->GetItemGuid(), nullptr, ItemEntry->GetCount());
	}

	WorldInventory->Update();
}

void AFortPlayerController::ServerCreateBuildingActorHook(UObject* Context, FFrame* Stack, void* Ret)
{
	auto PlayerController = Cast<AFortPlayerController>(Context);

	if (!PlayerController) // ??
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);

	auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	if (!PlayerStateAthena)
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);

	UClass* BuildingClass = nullptr;
	FVector BuildLocation;
	FRotator BuildRotator;
	bool bMirrored;

	if (Fortnite_Version >= 8.30)
	{
		struct FCreateBuildingActorData { uint32_t BuildingClassHandle; FVector BuildLoc; FRotator BuildRot; bool bMirrored; };
		auto CreateBuildingData = (FCreateBuildingActorData*)Stack->Locals;

		BuildLocation = CreateBuildingData->BuildLoc;
		BuildRotator = CreateBuildingData->BuildRot;
		bMirrored = CreateBuildingData->bMirrored;

		static auto BroadcastRemoteClientInfoOffset = PlayerController->GetOffset("BroadcastRemoteClientInfo");
		auto BroadcastRemoteClientInfo = PlayerController->Get(BroadcastRemoteClientInfoOffset);

		static auto RemoteBuildableClassOffset = BroadcastRemoteClientInfo->GetOffset("RemoteBuildableClass");
		BuildingClass = BroadcastRemoteClientInfo->Get<UClass*>(RemoteBuildableClassOffset);
	}
	else
	{
		struct FBuildingClassData { UClass* BuildingClass; int PreviousBuildingLevel; int UpgradeLevel; };
		struct SCBAParams { FBuildingClassData BuildingClassData; FVector BuildLoc; FRotator BuildRot; bool bMirrored; };

		auto Params = (SCBAParams*)Stack->Locals;

		BuildingClass = Params->BuildingClassData.BuildingClass;
		BuildLocation = Params->BuildLoc;
		BuildRotator = Params->BuildRot;
		bMirrored = Params->bMirrored;
	}

	// LOG_INFO(LogDev, "BuildingClass {}", __int64(BuildingClass));

	if (!BuildingClass)
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);

	TArray<ABuildingSMActor*> ExistingBuildings;
	char idk;
	static __int64 (*CantBuild)(UObject*, UObject*, FVector, FRotator, char, TArray<ABuildingSMActor*>*, char*) = decltype(CantBuild)(Addresses::CantBuild);
	bool bCanBuild = !CantBuild(GetWorld(), BuildingClass, BuildLocation, BuildRotator, bMirrored, &ExistingBuildings, &idk);

	if (!bCanBuild)
	{
		// LOG_INFO(LogDev, "cant build");
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
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
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);

	// static auto OwnerPersistentIDOffset = BuildingActor->GetOffset("OwnerPersistentID");
	// BuildingActor->Get<int>(OwnerPersistentIDOffset) = PlayerStateAthena->GetWorldPlayerId();

	BuildingActor->SetPlayerPlaced(true);
	BuildingActor->InitializeBuildingActor(PlayerController, BuildingActor, true);
	BuildingActor->SetTeam(PlayerStateAthena->GetTeamIndex());

	return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
}

void AFortPlayerController::DropSpecificItemHook(UObject* Context, FFrame& Stack, void* Ret)
{
	UFortItemDefinition* DropItemDef = nullptr;

	Stack.StepCompiledIn(&DropItemDef);

	if (!DropItemDef)
		return;

	auto PlayerController = Cast<AFortPlayerController>(Context);

	if (!PlayerController)
		return DropSpecificItemOriginal(Context, Stack, Ret);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return DropSpecificItemOriginal(Context, Stack, Ret);

	auto ItemInstance = WorldInventory->FindItemInstance(DropItemDef);

	if (!ItemInstance)
		return DropSpecificItemOriginal(Context, Stack, Ret);

	PlayerController->ServerAttemptInventoryDropHook(PlayerController, ItemInstance->GetItemEntry()->GetItemGuid(), ItemInstance->GetItemEntry()->GetCount());

	return DropSpecificItemOriginal(Context, Stack, Ret);
}

void AFortPlayerController::ServerAttemptInventoryDropHook(AFortPlayerController* PlayerController, FGuid ItemGuid, int Count)
{
	LOG_INFO(LogDev, "ServerAttemptInventoryDropHook!");

	auto Pawn = PlayerController->GetMyFortPawn();

	if (Count < 0 || !Pawn)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();
	auto ReplicatedEntry = WorldInventory->FindReplicatedEntry(ItemGuid);

	if (!ReplicatedEntry)
		return;

	auto ItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntry->GetItemDefinition());

	if (!ItemDefinition || !ItemDefinition->CanBeDropped())
		return;

	auto Pickup = AFortPickup::SpawnPickup(ItemDefinition, Pawn->GetActorLocation(), Count,
		EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, ReplicatedEntry->GetLoadedAmmo(), Pawn);

	if (!Pickup)
		return;

	bool bShouldUpdate = false;

	if (!WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, Count))
		return;

	if (bShouldUpdate)
		WorldInventory->Update();
}

void AFortPlayerController::ServerPlayEmoteItemHook(AFortPlayerController* PlayerController, UObject* EmoteAsset)
{
	auto PlayerState = (AFortPlayerStateAthena*)PlayerController->GetPlayerState();
	auto Pawn = PlayerController->GetPawn();

	if (!EmoteAsset || !PlayerState || !Pawn)
		return;

	UObject* AbilityToUse = nullptr;

	static auto AthenaSprayItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.AthenaSprayItemDefinition");

	if (EmoteAsset->IsA(AthenaSprayItemDefinitionClass))
	{
		static auto SprayGameplayAbilityDefault = FindObject("/Game/Abilities/Sprays/GAB_Spray_Generic.Default__GAB_Spray_Generic_C");
		AbilityToUse = SprayGameplayAbilityDefault;
	}

	if (!AbilityToUse)
	{
		static auto EmoteGameplayAbilityDefault = FindObject("/Game/Abilities/Emotes/GAB_Emote_Generic.Default__GAB_Emote_Generic_C");
		AbilityToUse = EmoteGameplayAbilityDefault;
	}

	if (!AbilityToUse)
		return;

	int outHandle = 0;

	FGameplayAbilitySpec* Spec = MakeNewSpec((UClass*)AbilityToUse, EmoteAsset, true);

	static unsigned int* (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* ASC, int* outHandle, __int64 Spec, FGameplayEventData* TriggerEventData)
		= decltype(GiveAbilityAndActivateOnce)(Addresses::GiveAbilityAndActivateOnce); // EventData is only on ue500?

	GiveAbilityAndActivateOnce(PlayerState->GetAbilitySystemComponent(), &outHandle, __int64(Spec), nullptr);
}

uint8 ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO = false)
{
	static auto ToDeathCauseFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerStateAthena.ToDeathCause");

	if (ToDeathCauseFn)
	{
		struct
		{
			FGameplayTagContainer                       InTags;                                                   // (ConstParm, Parm, OutParm, ReferenceParm, NativeAccessSpecifierPublic)
			bool                                               bWasDBNO;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			uint8_t                                        ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} AFortPlayerStateAthena_ToDeathCause_Params{ TagContainer, bWasDBNO };

		AFortPlayerStateAthena::StaticClass()->ProcessEvent(ToDeathCauseFn, &AFortPlayerStateAthena_ToDeathCause_Params);

		return AFortPlayerStateAthena_ToDeathCause_Params.ReturnValue;
	}

	static bool bHaveFoundAddress = false;

	static uint64 Addr = 0;

	if (!bHaveFoundAddress)
	{
		bHaveFoundAddress = true;

		if (Engine_Version == 420)
			Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 0F B6 FA 48 8B D9 E8 ? ? ? ? 33 F6 48 89 74 24").Get();

		if (!Addr)
		{
			LOG_WARN(LogPlayer, "Failed to find ToDeathCause address!");
			return 0;
		}
	}

	if (!Addr)
	{
		return 0;
	}

	static uint8 (*sub_7FF7AB499410)(FGameplayTagContainer TagContainer, char bWasDBNOIg) = decltype(sub_7FF7AB499410)(Addr);
	return sub_7FF7AB499410(TagContainer, bWasDBNO);
}

void AFortPlayerController::ClientOnPawnDiedHook(AFortPlayerController* PlayerController, void* DeathReport)
{
	auto GameState = Cast<AFortGameStateAthena>(((AFortGameMode*)GetWorld()->GetGameMode())->GetGameState());
	auto DeadPawn = Cast<AFortPlayerPawn>(PlayerController->GetPawn());
	auto DeadPlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());
	auto KillerPawn = Cast<AFortPlayerPawn>(*(AFortPawn**)(__int64(DeathReport) + MemberOffsets::DeathReport::KillerPawn));
	auto KillerPlayerState = Cast<AFortPlayerStateAthena>(*(AFortPlayerState**)(__int64(DeathReport) + MemberOffsets::DeathReport::KillerPlayerState));

	if (!DeadPawn || !GameState || !DeadPlayerState)
		return ClientOnPawnDiedOriginal(PlayerController, DeathReport);

	static auto DeathInfoStruct = FindObject<UStruct>("/Script/FortniteGame.DeathInfo");
	static auto DeathInfoStructSize = DeathInfoStruct->GetPropertiesSize();

	auto DeathLocation = DeadPawn->GetActorLocation();

	static auto FallDamageEnumValue = 1;

	auto DeathInfo = (void*)(__int64(DeadPlayerState) + MemberOffsets::FortPlayerStateAthena::DeathInfo); // Alloc<void>(DeathInfoStructSize);
	RtlSecureZeroMemory(DeathInfo, DeathInfoStructSize);

	auto/*&*/ Tags = MemberOffsets::FortPlayerPawn::CorrectTags == 0 ? FGameplayTagContainer()
		: DeadPawn->Get<FGameplayTagContainer>(MemberOffsets::FortPlayerPawn::CorrectTags);
		// *(FGameplayTagContainer*)(__int64(DeathReport) + MemberOffsets::DeathReport::Tags);

	// LOG_INFO(LogDev, "Tags: {}", Tags.ToStringSimple(true));

	auto DeathCause = ToDeathCause(Tags, false); // DeadPawn->IsDBNO() ??

	LOG_INFO(LogDev, "DeathCause: {}", (int)DeathCause);

	*(bool*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::bDBNO) = DeadPawn->IsDBNO();
	*(uint8*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathCause) = DeathCause;
	*(AActor**)(__int64(DeathInfo) + MemberOffsets::DeathInfo::FinisherOrDowner) = KillerPlayerState ? KillerPlayerState : DeadPlayerState;

	if (MemberOffsets::DeathInfo::DeathLocation != 0)
		*(FVector*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathLocation) = DeathLocation;

	if (MemberOffsets::DeathInfo::DeathTags != 0)
		*(FGameplayTagContainer*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathTags) = Tags;

	if (MemberOffsets::DeathInfo::bInitialized != 0)
		*(bool*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::bInitialized) = true;

	if (DeathCause == FallDamageEnumValue)
	{
		if (MemberOffsets::FortPlayerPawnAthena::LastFallDistance != 0)
			*(float*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::Distance) = DeadPawn->Get<float>(MemberOffsets::FortPlayerPawnAthena::LastFallDistance);
	}
	else
	{
		*(float*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::Distance) = KillerPawn ? KillerPawn->GetDistanceTo(DeadPawn) : 0;
	}

	if (MemberOffsets::FortPlayerState::PawnDeathLocation != 0)
		DeadPlayerState->Get<FVector>(MemberOffsets::FortPlayerState::PawnDeathLocation) = DeathLocation;

	LOG_INFO(LogDev, "Calling OnRep_DeathInfo.");

	static auto OnRep_DeathInfoFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerStateAthena.OnRep_DeathInfo");

	if (OnRep_DeathInfoFn)
		DeadPlayerState->ProcessEvent(OnRep_DeathInfoFn);

	LOG_INFO(LogDev, "Called OnRep_DeathInfo.");

	if (KillerPlayerState && KillerPlayerState != DeadPlayerState)
	{
		KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::KillScore)++;

		if (MemberOffsets::FortPlayerStateAthena::TeamKillScore != 0)
			KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::TeamKillScore)++;

		KillerPlayerState->ClientReportKill(DeadPlayerState);
		// KillerPlayerState->OnRep_Kills();
	}

	LOG_INFO(LogDev, "Reported kill.");

	if (KillerPawn && KillerPawn != DeadPawn)
	{
		KillerPawn->SetHealth(100);
		KillerPawn->SetShield(100);
	}

	bool bIsRespawningAllowed = GameState->IsRespawningAllowed(DeadPlayerState);

	if (!bIsRespawningAllowed)
	{
		auto WorldInventory = PlayerController->GetWorldInventory();

		if (!WorldInventory)
			return ClientOnPawnDiedOriginal(PlayerController, DeathReport);

		auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();

		std::vector<std::pair<FGuid, int>> GuidAndCountsToRemove;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances.at(i);

			LOG_INFO(LogDev, "[{}/{}] CurrentItemInstance {}", i, ItemInstances.Num(), __int64(ItemInstance));

			if (!ItemInstance)
				continue;

			auto ItemEntry = ItemInstance->GetItemEntry();
			auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->GetItemDefinition());

			LOG_INFO(LogDev, "[{}/{}] WorldItemDefinition {}", i, ItemInstances.Num(), WorldItemDefinition ? WorldItemDefinition->GetFullName() : "InvalidObject");

			if (!WorldItemDefinition)
				continue;

			auto ShouldBeDropped = WorldItemDefinition->CanBeDropped(); // WorldItemDefinition->ShouldDropOnDeath();

			LOG_INFO(LogDev, "[{}/{}] ShouldBeDropped {}", i, ItemInstances.Num(), ShouldBeDropped);

			if (!ShouldBeDropped)
				continue;

			AFortPickup::SpawnPickup(WorldItemDefinition, DeathLocation, ItemEntry->GetCount(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination,
				ItemEntry->GetLoadedAmmo());

			GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });
			// WorldInventory->RemoveItem(ItemEntry->GetItemGuid(), nullptr, ItemEntry->GetCount());
		}

		for (auto& Pair : GuidAndCountsToRemove)
		{
			WorldInventory->RemoveItem(Pair.first, nullptr, Pair.second);
		}

		WorldInventory->Update();

		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		if (!DeadPawn->IsDBNO())
		{
			static void (*RemoveFromAlivePlayers)(AFortGameModeAthena* GameMode, AFortPlayerController* PlayerController, APlayerState* PlayerState, APawn* FinisherPawn,
				UFortWeaponItemDefinition* FinishingWeapon, uint8_t DeathCause, char a7)
				= decltype(RemoveFromAlivePlayers)(Addresses::RemoveFromAlivePlayers);

			AActor* DamageCauser = *(AActor**)(__int64(DeathReport) + MemberOffsets::DeathReport::DamageCauser);
			UFortWeaponItemDefinition* KillerWeaponDef = nullptr;

			static auto FortProjectileBaseClass = FindObject<UClass>("/Script/FortniteGame.FortProjectileBase");
			LOG_INFO(LogDev, "FortProjectileBaseClass: {}", __int64(FortProjectileBaseClass));

			if (DamageCauser->IsA(FortProjectileBaseClass))
			{
				LOG_INFO(LogDev, "From a projectile!");
				KillerWeaponDef = ((AFortWeapon*)DamageCauser->GetOwner())->GetWeaponData();
			}
			if (auto Weapon = Cast<AFortWeapon>(DamageCauser))
			{
				LOG_INFO(LogDev, "From a weapon!");
				KillerWeaponDef = Weapon->GetWeaponData();
			}

			LOG_INFO(LogDev, "KillerWeaponDef: {}", KillerWeaponDef ? KillerWeaponDef->GetFullName() : "InvalidObject");

			RemoveFromAlivePlayers(GameMode, PlayerController, KillerPlayerState == DeadPlayerState ? nullptr : KillerPlayerState, KillerPawn, KillerWeaponDef, DeathCause, 0);
		}
	}

	return ClientOnPawnDiedOriginal(PlayerController, DeathReport);
}

void AFortPlayerController::ServerBeginEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
	if (!BuildingActorToEdit || !BuildingActorToEdit->IsPlayerPlaced())
		return;

	auto Pawn = PlayerController->GetMyFortPawn();

	if (!Pawn)
		return;

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

	/* auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto EditToolInstance = WorldInventory->FindItemInstance(EditToolDef);

	if (!EditToolInstance)
		return;

	if (auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(EditToolDef, EditToolInstance->GetItemEntry()->GetItemGuid()))) */
	if (auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(EditToolDef, FGuid{})))
	{
		BuildingActorToEdit->GetEditingPlayer() = PlayerController->GetPlayerState();
		// Onrep?

		EditTool->GetEditActor() = BuildingActorToEdit;
		// Onrep?
	}
}

/* void AFortPlayerController::ServerEditBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int RotationIterations, char bMirrored)
{
	auto PlayerState = (AFortPlayerState*)PlayerController->GetPlayerState();

	if (!BuildingActorToEdit || !NewBuildingClass || BuildingActorToEdit->IsDestroyed() || BuildingActorToEdit->GetEditingPlayer() != PlayerState)
		return;

	// if (!PlayerState || PlayerState->GetTeamIndex() != BuildingActorToEdit->GetTeamIndex()) 
		// return;

	BuildingActorToEdit->GetEditingPlayer() = nullptr;

	LOG_INFO(LogDev, "RotationIterations: {}", RotationIterations);

	static ABuildingSMActor* (*BuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) =
		decltype(BuildingSMActorReplaceBuildingActor)(Addresses::ReplaceBuildingActor);

	if (auto BuildingActor = BuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass, 
		BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, PlayerController))
	{
		BuildingActor->SetPlayerPlaced(true);

		// if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
			// BuildingActor->SetTeam(PlayerState->GetTeamIndex());
	}
} */

void AFortPlayerController::ServerEditBuildingActorHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto PlayerController = (AFortPlayerController*)Context;

	auto PlayerState = (AFortPlayerState*)PlayerController->GetPlayerState();

	auto Params = Stack.Locals;

	static auto RotationIterationsOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "RotationIterations");
	static auto NewBuildingClassOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "NewBuildingClass");
	static auto BuildingActorToEditOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "BuildingActorToEdit");
	static auto bMirroredOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor", "bMirrored");

	auto BuildingActorToEdit = *(ABuildingSMActor**)(__int64(Params) + BuildingActorToEditOffset);
	auto NewBuildingClass = *(UClass**)(__int64(Params) + NewBuildingClassOffset);
	int RotationIterations = Fortnite_Version < 8.30 ? *(int*)(__int64(Params) + RotationIterationsOffset) : (int)(*(uint8*)(__int64(Params) + RotationIterationsOffset));
	auto bMirrored = *(char*)(__int64(Params) + bMirroredOffset);

	LOG_INFO(LogDev, "RotationIterations: {}", RotationIterations);

	if (!BuildingActorToEdit || !NewBuildingClass || BuildingActorToEdit->IsDestroyed() || BuildingActorToEdit->GetEditingPlayer() != PlayerState)
		return ServerEditBuildingActorOriginal(Context, Stack, Ret);

	// if (!PlayerState || PlayerState->GetTeamIndex() != BuildingActorToEdit->GetTeamIndex()) 
		//return ServerEditBuildingActorOriginal(Context, Frame, Ret);

	BuildingActorToEdit->GetEditingPlayer() = nullptr;

	static ABuildingSMActor* (*BuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) =
		decltype(BuildingSMActorReplaceBuildingActor)(Addresses::ReplaceBuildingActor);

	if (auto BuildingActor = BuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass,
		BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, PlayerController))
	{
		BuildingActor->SetPlayerPlaced(true);

		// if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
			// BuildingActor->SetTeam(PlayerState->GetTeamIndex());
	}

	return ServerEditBuildingActorOriginal(Context, Stack, Ret);
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