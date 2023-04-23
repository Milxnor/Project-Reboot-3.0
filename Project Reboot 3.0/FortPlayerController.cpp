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
#include "FortGadgetItemDefinition.h"
#include "FortAbilitySet.h"
#include "vendingmachine.h"
#include "KismetSystemLibrary.h"

void AFortPlayerController::ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ClientReportDamagedResourceBuilding");

	struct { ABuildingSMActor* BuildingSMActor; EFortResourceType PotentialResourceType; int PotentialResourceCount; bool bDestroyed; bool bJustHitWeakspot; }
	AFortPlayerController_ClientReportDamagedResourceBuilding_Params{BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakspot};

	this->ProcessEvent(fn, &AFortPlayerController_ClientReportDamagedResourceBuilding_Params);
}

bool AFortPlayerController::DoesBuildFree()
{
	if (Globals::bInfiniteMaterials)
		return true;

	static auto bBuildFreeOffset = GetOffset("bBuildFree");
	static auto bBuildFreeFieldMask = GetFieldMask(GetProperty("bBuildFree"));
	return ReadBitfieldValue(bBuildFreeOffset, bBuildFreeFieldMask);
}

void AFortPlayerController::DropAllItems(const std::vector<UFortItemDefinition*>& IgnoreItemDefs, bool bIgnoreSecondaryQuickbar, bool bRemoveIfNotDroppable)
{
	auto Pawn = this->GetMyFortPawn();

	if (!Pawn)
		return;

	auto WorldInventory = this->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();
	auto Location = Pawn->GetActorLocation();

	std::vector<std::pair<FGuid, int>> GuidAndCountsToRemove;

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		auto ItemInstance = ItemInstances.at(i);

		if (!ItemInstance)
			continue;

		auto ItemEntry = ItemInstance->GetItemEntry();
		auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->GetItemDefinition());

		if (!WorldItemDefinition || std::find(IgnoreItemDefs.begin(), IgnoreItemDefs.end(), WorldItemDefinition) != IgnoreItemDefs.end())
			continue;

		if (bIgnoreSecondaryQuickbar && !IsPrimaryQuickbar(WorldItemDefinition))
			continue;

		if (!bRemoveIfNotDroppable && !WorldItemDefinition->CanBeDropped())
			continue;

		GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });

		if (bRemoveIfNotDroppable && !WorldItemDefinition->CanBeDropped())
			continue;
	
		AFortPickup::SpawnPickup(WorldItemDefinition, Location, ItemEntry->GetCount(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset,
			ItemEntry->GetLoadedAmmo());
	}

	for (auto& Pair : GuidAndCountsToRemove)
	{
		WorldInventory->RemoveItem(Pair.first, nullptr, Pair.second, true);
	}

	WorldInventory->Update();
}

void AFortPlayerController::ApplyCosmeticLoadout()
{
	auto PlayerStateAsFort = Cast<AFortPlayerStateAthena>(GetPlayerState());

	if (!PlayerStateAsFort)
		return;

	auto PawnAsFort = Cast<AFortPlayerPawn>(GetMyFortPawn());

	if (!PawnAsFort)
		return;

	static auto UpdatePlayerCustomCharacterPartsVisualizationFn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization");

	if (!UpdatePlayerCustomCharacterPartsVisualizationFn)
	{
		if (Addresses::ApplyCharacterCustomization)
		{
			static void* (*ApplyCharacterCustomizationOriginal)(AFortPlayerState* a1, AFortPawn* a3) = decltype(ApplyCharacterCustomizationOriginal)(Addresses::ApplyCharacterCustomization);
			ApplyCharacterCustomizationOriginal(PlayerStateAsFort, PawnAsFort);

			PlayerStateAsFort->ForceNetUpdate();
			PawnAsFort->ForceNetUpdate();
			this->ForceNetUpdate();

			return;
		}

		auto CosmeticLoadout = this->GetCosmeticLoadout();

		if (CosmeticLoadout)
		{
			/* static auto Pawn_CosmeticLoadoutOffset = PawnAsFort->GetOffset("CosmeticLoadout");

			if (Pawn_CosmeticLoadoutOffset != -1)
			{
				CopyStruct(PawnAsFort->GetPtr<__int64>(Pawn_CosmeticLoadoutOffset), CosmeticLoadout, FFortAthenaLoadout::GetStructSize());
			} */

			auto Character = CosmeticLoadout->GetCharacter();

			// LOG_INFO(LogDev, "Character: {}", __int64(Character));
			// LOG_INFO(LogDev, "Character Name: {}", Character ? Character->GetFullName() : "InvalidObject");

			if (PawnAsFort)
			{
				ApplyCID(PawnAsFort, Character, false);

				auto Backpack = CosmeticLoadout->GetBackpack();

				if (Backpack)
				{
					static auto CharacterPartsOffset = Backpack->GetOffset("CharacterParts");

					if (CharacterPartsOffset != -1)
					{
						auto& BackpackCharacterParts = Backpack->Get<TArray<UObject*>>(CharacterPartsOffset);

						for (int i = 0; i < BackpackCharacterParts.Num(); i++)
						{
							auto BackpackCharacterPart = BackpackCharacterParts.at(i);

							if (!BackpackCharacterPart)
								continue;

							PawnAsFort->ServerChoosePart(EFortCustomPartType::Backpack, BackpackCharacterPart);
						}

						// UFortKismetLibrary::ApplyCharacterCosmetics(GetWorld(), BackpackCharacterParts, PlayerStateAsFort, &aa);
					}
				}
			}
		}

		PlayerStateAsFort->ForceNetUpdate();
		PawnAsFort->ForceNetUpdate();
		this->ForceNetUpdate();

		return;
	}

	UFortKismetLibrary::StaticClass()->ProcessEvent(UpdatePlayerCustomCharacterPartsVisualizationFn, &PlayerStateAsFort);
}

void AFortPlayerController::ServerLoadingScreenDroppedHook(UObject* Context, FFrame* Stack, void* Ret)
{
	auto PlayerController = (AFortPlayerController*)Context;

	PlayerController->ApplyCosmeticLoadout();

	return ServerLoadingScreenDroppedOriginal(Context, Stack, Ret);
}

void AFortPlayerController::ServerRepairBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair)
{
	if (!BuildingActorToRepair)
		return;

	float BuildingHealthPercent = BuildingActorToRepair->GetHealthPercent();

	// todo not hardcode these?
	float BuildingCost = 10;
	float RepairCostMultiplier = 0.75;

	float BuildingHealthPercentLost = 1.0 - BuildingHealthPercent;
	float RepairCostUnrounded = (BuildingCost * BuildingHealthPercentLost) * RepairCostMultiplier;
	float RepairCost = std::floor(RepairCostUnrounded > 0 ? RepairCostUnrounded < 1 ? 1 : RepairCostUnrounded : 0);

	if (RepairCost < 0)
		return;

	auto ResourceItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActorToRepair->GetResourceType());

	if (!ResourceItemDefinition)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	if (!PlayerController->DoesBuildFree())
	{
		auto ResourceInstance = WorldInventory->FindItemInstance(ResourceItemDefinition);

		if (!ResourceInstance)
			return;

		bool bShouldUpdate = false;

		if (!WorldInventory->RemoveItem(ResourceInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, RepairCost))
			return;

		if (bShouldUpdate)
			WorldInventory->Update();
	}

	struct
	{
		AFortPlayerController* RepairingController;                                      // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		int                                                ResourcesSpent;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	}ABuildingSMActor_RepairBuilding_Params{ PlayerController, RepairCost };

	static auto RepairBuildingFn = FindObject<UFunction>("/Script/FortniteGame.BuildingSMActor.RepairBuilding");
	BuildingActorToRepair->ProcessEvent(RepairBuildingFn, &ABuildingSMActor_RepairBuilding_Params);
	// PlayerController->FortClientPlaySoundAtLocation(PlayerController->StartRepairSound, BuildingActorToRepair->K2_GetActorLocation(), 0, 0);
}

void AFortPlayerController::ServerExecuteInventoryItemHook(AFortPlayerController* PlayerController, FGuid ItemGuid)
{
	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto ItemInstance = WorldInventory->FindItemInstance(ItemGuid);
	auto Pawn = Cast<AFortPlayerPawn>(PlayerController->GetPawn());

	if (!ItemInstance || !Pawn)
		return;

	FGuid OldGuid = Pawn->GetCurrentWeapon() ? Pawn->GetCurrentWeapon()->GetItemEntryGuid() : FGuid(-1, -1, -1, -1);
	UFortItem* OldInstance = OldGuid == FGuid(-1, -1, -1, -1) ? nullptr : WorldInventory->FindItemInstance(OldGuid);
	auto ItemDefinition = ItemInstance->GetItemEntry()->GetItemDefinition();

	if (!ItemDefinition)
		return;

	// LOG_INFO(LogDev, "Equipping ItemDefinition: {}", ItemDefinition->GetFullName());

	static auto FortGadgetItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortGadgetItemDefinition");

	UFortGadgetItemDefinition* GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemDefinition);

	if (GadgetItemDefinition)
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

		// LOG_INFO(LogDev, "Equipping Gadget: {}", ItemDefinition->GetFullName());
	}

	if (auto DecoItemDefinition = Cast<UFortDecoItemDefinition>(ItemDefinition))
	{
		Pawn->PickUpActor(nullptr, DecoItemDefinition); // todo check ret value? // I checked on 1.7.2 and it only returns true if the new weapon is a FortDecoTool
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

			UBuildingEditModeMetadata* OldMetadata = nullptr; // Newer versions
			OldMetadata = BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset);

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

			BuildingTool->ProcessEvent(OnRep_DefaultMetadataFn, &OldMetadata);
		}
	}
}

void AFortPlayerController::ServerAttemptInteractHook(UObject* Context, FFrame* Stack, void* Ret)
{
	// static auto LlamaClass = FindObject<UClass>("/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C");
	static auto FortAthenaSupplyDropClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaSupplyDrop");
	static auto BuildingItemCollectorActorClass = FindObject<UClass>("/Script/FortniteGame.BuildingItemCollectorActor");

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
	static auto SearchAnimationCountOffset = FindOffsetStruct("/Script/FortniteGame.FortSearchBounceData", "SearchAnimationCount");

	if (auto BuildingContainer = Cast<ABuildingContainer>(ReceivingActor))
	{
		static auto bAlreadySearchedOffset = BuildingContainer->GetOffset("bAlreadySearched");
		static auto SearchBounceDataOffset = BuildingContainer->GetOffset("SearchBounceData");
		static auto bAlreadySearchedFieldMask = GetFieldMask(BuildingContainer->GetProperty("bAlreadySearched"));
		
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
		auto Vehicle = (AFortAthenaVehicle*)ReceivingActor;
		ServerAttemptInteractOriginal(Context, Stack, Ret);
		
		if (!AreVehicleWeaponsEnabled())
			return;

		auto Pawn = (AFortPlayerPawn*)PlayerController->GetMyFortPawn();

		if (!Pawn)
			return;

		auto VehicleWeaponDefinition = Pawn->GetVehicleWeaponDefinition(Vehicle);

		if (!VehicleWeaponDefinition)
		{
			LOG_INFO(LogDev, "Invalid VehicleWeaponDefinition!");
			return;
		}

		LOG_INFO(LogDev, "Equipping {}", VehicleWeaponDefinition->GetFullName());

		auto WorldInventory = PlayerController->GetWorldInventory();

		if (!WorldInventory)
			return;

		auto NewAndModifiedInstances = WorldInventory->AddItem(VehicleWeaponDefinition, nullptr);
		auto NewVehicleInstance = NewAndModifiedInstances.first[0];

		if (!NewVehicleInstance)
			return;

		WorldInventory->Update();

		auto VehicleWeapon = Pawn->EquipWeaponDefinition(VehicleWeaponDefinition, NewVehicleInstance->GetItemEntry()->GetItemGuid());
		// PlayerController->ServerExecuteInventoryItemHook(PlayerController, newitem->GetItemEntry()->GetItemGuid());

		/* if (WeaponComponent)
		{
			static auto bWeaponEquippedOffset = WeaponComponent->GetOffset("bWeaponEquipped");
			WeaponComponent->Get<bool>(bWeaponEquippedOffset) = true;

			static auto CachedWeaponOffset = WeaponComponent->GetOffset("CachedWeapon");
			WeaponComponent->Get<AFortWeapon*>(CachedWeaponOffset) = VehicleWeapon;

			static auto CachedWeaponDefOffset = WeaponComponent->GetOffset("CachedWeaponDef");
			WeaponComponent->Get<UFortWeaponItemDefinition*>(CachedWeaponDefOffset) = VehicleWeaponDefinition;
		} */

		return;
	}
	else if (ReceivingActor->IsA(BuildingItemCollectorActorClass))
	{
		auto WorldInventory = PlayerController->GetWorldInventory();

		if (!WorldInventory)
			return ServerAttemptInteractOriginal(Context, Stack, Ret);

		auto ItemCollector = ReceivingActor;
		static auto ActiveInputItemOffset = ItemCollector->GetOffset("ActiveInputItem");
		auto CurrentMaterial = ItemCollector->Get<UFortWorldItemDefinition*>(ActiveInputItemOffset); // InteractType->OptionalObjectData

		if (!CurrentMaterial)
			return ServerAttemptInteractOriginal(Context, Stack, Ret);

		int Index = 0;

		// this is a weird way of getting the current item collection we are on.

		static auto StoneItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto MetalItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

		if (CurrentMaterial == StoneItemData)
			Index = 1;
		else if (CurrentMaterial == MetalItemData)
			Index = 2;

		static auto ItemCollectionsOffset = ItemCollector->GetOffset("ItemCollections");
		auto& ItemCollections = ItemCollector->Get<TArray<FCollectorUnitInfo>>(ItemCollectionsOffset);

		auto ItemCollection = ItemCollections.AtPtr(Index, FCollectorUnitInfo::GetPropertiesSize());

		if (Fortnite_Version < 8.10)
		{
			auto Cost = ItemCollection->GetInputCount()->GetValue();

			if (!CurrentMaterial)
				return ServerAttemptInteractOriginal(Context, Stack, Ret);

			auto MatInstance = WorldInventory->FindItemInstance(CurrentMaterial);

			if (!MatInstance)
				return ServerAttemptInteractOriginal(Context, Stack, Ret);

			bool bShouldUpdate = false;

			if (!WorldInventory->RemoveItem(MatInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, Cost, true))
				return ServerAttemptInteractOriginal(Context, Stack, Ret);

			if (bShouldUpdate)
				WorldInventory->Update();
		}

		for (int z = 0; z < ItemCollection->GetOutputItemEntry()->Num(); z++)
		{
			auto Entry = ItemCollection->GetOutputItemEntry()->AtPtr(z, FFortItemEntry::GetStructSize());

			AFortPickup::SpawnPickup(Entry->GetItemDefinition(), LocationToSpawnLoot, Entry->GetCount(), 
				EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, Entry->GetLoadedAmmo(), PlayerController->GetMyFortPawn());
		}

		static auto bCurrentInteractionSuccessOffset = ItemCollector->GetOffset("bCurrentInteractionSuccess", false);

		if (bCurrentInteractionSuccessOffset != -1)
		{
			static auto bCurrentInteractionSuccessFieldMask = GetFieldMask(ItemCollector->GetProperty("bCurrentInteractionSuccess"));
			ItemCollector->SetBitfieldValue(bCurrentInteractionSuccessOffset, bCurrentInteractionSuccessFieldMask, true); // idek if this is needed
		}

		static auto DoVendDeath = FindObject<UFunction>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C.DoVendDeath");

		if (DoVendDeath)
		{
			ItemCollector->ProcessEvent(DoVendDeath);
			ItemCollector->K2_DestroyActor();
		}
	}

	return ServerAttemptInteractOriginal(Context, Stack, Ret);
}

void AFortPlayerController::ServerAttemptAircraftJumpHook(AFortPlayerController* PC, FRotator ClientRotation)
{
	auto PlayerController = Cast<AFortPlayerController>(Engine_Version < 424 ? PC : ((UActorComponent*)PC)->GetOwner());

	if (Engine_Version < 424 && !Globals::bLateGame.load())
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	if (Fortnite_Version == 17.30 && Globals::bGoingToPlayEvent)
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation); // We want to be teleported back to the UFO but we dont use chooseplayerstart

	LOG_INFO(LogDev, "PlayerController: {}", __int64(PlayerController));

	if (!PlayerController)
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	// if (!PlayerController->bInAircraft) 
		// return;

	auto GameMode = (AFortGameModeAthena*)GetWorld()->GetGameMode();
	auto GameState = GameMode->GetGameStateAthena();

	static auto AircraftsOffset = GameState->GetOffset("Aircrafts");
	auto Aircrafts = GameState->GetPtr<TArray<AActor*>>(AircraftsOffset);

	if (Aircrafts->Num() <= 0)
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	auto NewPawn = GameMode->SpawnDefaultPawnForHook(GameMode, (AController*)PlayerController, Aircrafts->at(0));
	PlayerController->Possess(NewPawn);

	auto NewPawnAsFort = Cast<AFortPawn>(NewPawn);

	if (NewPawnAsFort)
	{
		NewPawnAsFort->SetHealth(100);
		
		if (Globals::bLateGame)
			NewPawnAsFort->SetShield(100);
	}

	// PlayerController->ServerRestartPlayer();

	if (Globals::bLateGame)
	{
		static int LastNum1 = 124;

		if (LastNum1 != AmountOfRestarts)
		{
			auto SafeZoneIndicator = GameMode->GetSafeZoneIndicator();

			if (SafeZoneIndicator)
			{
				LastNum1 = AmountOfRestarts;

				SafeZoneIndicator->SkipShrinkSafeZone();
				SafeZoneIndicator->SkipShrinkSafeZone();
			}
		}

		static auto WoodItemData = FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		static auto StoneItemData = FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto MetalItemData = FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

		static auto Rifle = FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03");
		static auto Shotgun = FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03");
		static auto SMG = FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03");

		static auto MiniShields = FindObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall");

		static auto Shells = FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
		static auto Medium = FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
		static auto Light = FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
		static auto Heavy = FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");

		auto WorldInventory = PlayerController->GetWorldInventory();

		if (!WorldInventory)
			return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

		WorldInventory->AddItem(WoodItemData, nullptr, 500);
		WorldInventory->AddItem(StoneItemData, nullptr, 500);
		WorldInventory->AddItem(MetalItemData, nullptr, 500);
		WorldInventory->AddItem(Rifle, nullptr, 1);
		WorldInventory->AddItem(Shotgun, nullptr, 1);
		WorldInventory->AddItem(SMG, nullptr, 1);
		WorldInventory->AddItem(MiniShields, nullptr, 6);
		WorldInventory->AddItem(Shells, nullptr, 999);
		WorldInventory->AddItem(Medium, nullptr, 999);
		WorldInventory->AddItem(Light, nullptr, 999);
		WorldInventory->AddItem(Heavy, nullptr, 999);

		WorldInventory->Update();
	}

	// return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);
}

void AFortPlayerController::ServerDropAllItemsHook(AFortPlayerController* PlayerController, UFortItemDefinition* IgnoreItemDef)
{
	LOG_INFO(LogDev, "DropAllItems!");
	PlayerController->DropAllItems({ IgnoreItemDef });
}

void AFortPlayerController::ServerCreateBuildingActorHook(UObject* Context, FFrame* Stack, void* Ret)
{
	auto PlayerController = (AFortPlayerController*)Context;

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

	auto GameState = Cast<AFortGameStateAthena>(((AFortGameMode*)GetWorld()->GetGameMode())->GetGameState());

	auto StructuralSupportSystem = GameState->GetStructuralSupportSystem();

	if (StructuralSupportSystem)
	{
		if (!StructuralSupportSystem->IsWorldLocValid(BuildLocation))
		{
			return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
		}
	}

	if (!GameState->IsPlayerBuildableClass(BuildingClass))
	{
		LOG_INFO(LogDev, "Cheater most likely.");
		// PlayerController->GetAnticheatComponent().AddAndCheck(Severity::HIGH);
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
	}

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

	auto MatDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->GetResourceType());
	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);

	auto MatInstance = WorldInventory->FindItemInstance(MatDefinition);

	bool bBuildFree = PlayerController->DoesBuildFree();

	bool bShouldDestroy = MatInstance && MatInstance->GetItemEntry() ? MatInstance->GetItemEntry()->GetCount() < 10 : true;

	if (bShouldDestroy && !bBuildFree)
	{
		BuildingActor->SilentDie();
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
	}

	BuildingActor->SetPlayerPlaced(true);
	BuildingActor->InitializeBuildingActor(PlayerController, BuildingActor, true);
	BuildingActor->SetTeam(PlayerStateAthena->GetTeamIndex()); // required?

	if (!bBuildFree)
	{
		bool bShouldUpdate = false;
		WorldInventory->RemoveItem(MatInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, 10);

		if (bShouldUpdate)
			WorldInventory->Update();
	}

	/*

	GET_PLAYLIST(GameState);

	if (CurrentPlaylist)
	{
		// CurrentPlaylist->ApplyModifiersToActor(BuildingActor); // seems automatic
	} */

	return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
}

AActor* AFortPlayerController::SpawnToyInstanceHook(UObject* Context, FFrame* Stack, AActor** Ret)
{
	LOG_INFO(LogDev, "SpawnToyInstance!");

	auto PlayerController = Cast<AFortPlayerController>(Context);

	UClass* ToyClass = nullptr;
	FTransform SpawnPosition;

	Stack->StepCompiledIn(&ToyClass);
	Stack->StepCompiledIn(&SpawnPosition);

	SpawnToyInstanceOriginal(Context, Stack, Ret);

	if (!ToyClass)
		return nullptr;

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.Owner = PlayerController;
	auto NewToy = GetWorld()->SpawnActor<AActor>(ToyClass, SpawnPosition, SpawnParameters);

	static auto ActiveToyInstancesOffset = PlayerController->GetOffset("ActiveToyInstances");
	auto& ActiveToyInstances = PlayerController->Get<TArray<AActor*>>(ActiveToyInstancesOffset);
	
	static auto ToySummonCountsOffset = PlayerController->GetOffset("ToySummonCounts");
	auto& ToySummonCounts = PlayerController->Get<TMap<UClass*, int>>(ToySummonCountsOffset);

	// ActiveToyInstances.Add(NewToy);

	*Ret = NewToy;
	return *Ret;
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

	if (!ItemDefinition->ShouldIgnoreRespawningOnDrop() && ItemDefinition->GetDropBehavior() != EWorldItemDropBehavior::DestroyOnDrop)
	{
		auto Pickup = AFortPickup::SpawnPickup(ReplicatedEntry, Pawn->GetActorLocation(),
			EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);

		if (!Pickup)
			return;
	}

	bool bShouldUpdate = false;

	if (!WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, Count, true))
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
	static auto AthenaToyItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.AthenaToyItemDefinition");

	if (EmoteAsset->IsA(AthenaSprayItemDefinitionClass))
	{
		static auto SprayGameplayAbilityDefault = FindObject("/Game/Abilities/Sprays/GAB_Spray_Generic.Default__GAB_Spray_Generic_C");
		AbilityToUse = SprayGameplayAbilityDefault;
	}

	else if (EmoteAsset->IsA(AthenaToyItemDefinitionClass))
	{
		static auto ToySpawnAbilityOffset = EmoteAsset->GetOffset("ToySpawnAbility");
		auto& ToySpawnAbilitySoft = EmoteAsset->Get<TSoftObjectPtr<UClass>>(ToySpawnAbilityOffset);

		static auto BGAClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");

		auto ToySpawnAbility = ToySpawnAbilitySoft.Get(BGAClass, true);

		if (ToySpawnAbility)
			AbilityToUse = ToySpawnAbility->CreateDefaultObject();
	}

	// LOG_INFO(LogDev, "Before AbilityToUse: {}", AbilityToUse ? AbilityToUse->GetFullName() : "InvalidObject");

	if (!AbilityToUse)
	{
		static auto EmoteGameplayAbilityDefault = FindObject("/Game/Abilities/Emotes/GAB_Emote_Generic.Default__GAB_Emote_Generic_C");
		AbilityToUse = EmoteGameplayAbilityDefault;
	}

	if (!AbilityToUse)
		return;

	static auto AthenaDanceItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.AthenaDanceItemDefinition");

	if (EmoteAsset->IsA(AthenaDanceItemDefinitionClass))
	{
		static auto EmoteAsset_bMovingEmoteOffset = EmoteAsset->GetOffset("bMovingEmote", false);
		static auto bMovingEmoteOffset = Pawn->GetOffset("bMovingEmote", false);

		if (bMovingEmoteOffset != -1 && EmoteAsset_bMovingEmoteOffset != -1)
		{
			static auto bMovingEmoteFieldMask = GetFieldMask(Pawn->GetProperty("bMovingEmote"));
			static auto EmoteAsset_bMovingEmoteFieldMask = GetFieldMask(EmoteAsset->GetProperty("bMovingEmote"));
			Pawn->SetBitfieldValue(bMovingEmoteOffset, bMovingEmoteFieldMask, EmoteAsset->ReadBitfieldValue(EmoteAsset_bMovingEmoteOffset, EmoteAsset_bMovingEmoteFieldMask));
		}

		static auto bMoveForwardOnlyOffset = EmoteAsset->GetOffset("bMoveForwardOnly", false);
		static auto bMovingEmoteForwardOnlyOffset = Pawn->GetOffset("bMovingEmoteForwardOnly", false);

		if (bMovingEmoteForwardOnlyOffset != -1 && bMoveForwardOnlyOffset != -1)
		{
			static auto bMovingEmoteForwardOnlyFieldMask = GetFieldMask(Pawn->GetProperty("bMovingEmoteForwardOnly"));
			static auto bMoveForwardOnlyFieldMask = GetFieldMask(EmoteAsset->GetProperty("bMoveForwardOnly"));
			Pawn->SetBitfieldValue(bMovingEmoteOffset, bMovingEmoteForwardOnlyFieldMask, EmoteAsset->ReadBitfieldValue(bMoveForwardOnlyOffset, bMoveForwardOnlyFieldMask));
		}

		static auto WalkForwardSpeedOffset = EmoteAsset->GetOffset("WalkForwardSpeed", false);
		static auto EmoteWalkSpeedOffset = Pawn->GetOffset("EmoteWalkSpeed", false);

		if (EmoteWalkSpeedOffset != -1 && WalkForwardSpeedOffset != -1)
		{
			Pawn->Get<float>(EmoteWalkSpeedOffset) = EmoteAsset->Get<float>(WalkForwardSpeedOffset);
		}
	}

	int outHandle = 0;

	FGameplayAbilitySpec* Spec = MakeNewSpec((UClass*)AbilityToUse, EmoteAsset, true);

	static unsigned int* (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* ASC, int* outHandle, __int64 Spec, FGameplayEventData* TriggerEventData) = decltype(GiveAbilityAndActivateOnce)(Addresses::GiveAbilityAndActivateOnce); // EventData is only on ue500?

	if (GiveAbilityAndActivateOnce)
		GiveAbilityAndActivateOnce(PlayerState->GetAbilitySystemComponent(), &outHandle, __int64(Spec), nullptr);
}

uint8 ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO = false, AFortPawn* Pawn = nullptr)
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

		if (Engine_Version == 419)
			Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 0F B6 F8 48 8B DA 48 8B F1 E8 ? ? ? ? 33 ED").Get();
		if (Engine_Version == 420)
			Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 0F B6 FA 48 8B D9 E8 ? ? ? ? 33 F6 48 89 74 24").Get();
		if (Engine_Version == 421) // 5.1
			Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 0F B6 FA 48 8B D9 E8 ? ? ? ? 33").Get();

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

	if (Engine_Version == 419)
	{
		static uint8(*sub_7FF7AB499410)(AFortPawn* Pawn, FGameplayTagContainer TagContainer, char bWasDBNOIg) = decltype(sub_7FF7AB499410)(Addr);
		return sub_7FF7AB499410(Pawn, TagContainer, bWasDBNO);
	}

	static uint8 (*sub_7FF7AB499410)(FGameplayTagContainer TagContainer, char bWasDBNOIg) = decltype(sub_7FF7AB499410)(Addr);
	return sub_7FF7AB499410(TagContainer, bWasDBNO);
}

std::vector<APlayerController*> PlayerControllersDead; // make atomic?
// std::array<std::atomic<APlayerController*>, 100> PlayerControllersDead;
std::atomic<int> numValidElements(0);

DWORD WINAPI SpectateThread(LPVOID)
{
	while (1)
	{
		for (auto PC : PlayerControllersDead)
		// for (int i = 0; i < PlayerControllersDead.size(); i++)
		{
			// auto PC = PlayerControllersDead.at(i).load();

			static auto SpectateOnDeathFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerZone.SpectateOnDeath") ? FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerZone.SpectateOnDeath") : FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerAthena.SpectateOnDeath");
			PC->ProcessEvent(SpectateOnDeathFn);
		}

		Sleep(4000);
	}

	return 0;
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

	auto DeathCause = ToDeathCause(Tags, false, DeadPawn); // DeadPawn->IsDBNO() ??

	LOG_INFO(LogDev, "DeathCause: {}", (int)DeathCause);

	*(bool*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::bDBNO) = DeadPawn->IsDBNO();
	*(uint8*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathCause) = DeathCause;
	*(AActor**)(__int64(DeathInfo) + MemberOffsets::DeathInfo::FinisherOrDowner) = KillerPlayerState ? KillerPlayerState : DeadPlayerState;

	if (MemberOffsets::DeathInfo::DeathLocation != -1)
		*(FVector*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathLocation) = DeathLocation;

	if (MemberOffsets::DeathInfo::DeathTags != -1)
		*(FGameplayTagContainer*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::DeathTags) = Tags;

	if (MemberOffsets::DeathInfo::bInitialized != -1)
		*(bool*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::bInitialized) = true;

	if (DeathCause == FallDamageEnumValue)
	{
		if (MemberOffsets::FortPlayerPawnAthena::LastFallDistance != -1)
			*(float*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::Distance) = DeadPawn->Get<float>(MemberOffsets::FortPlayerPawnAthena::LastFallDistance);
	}
	else
	{
		if (MemberOffsets::DeathInfo::Distance != -1)
			*(float*)(__int64(DeathInfo) + MemberOffsets::DeathInfo::Distance) = KillerPawn ? KillerPawn->GetDistanceTo(DeadPawn) : 0;
	}

	if (MemberOffsets::FortPlayerState::PawnDeathLocation != -1)
		DeadPlayerState->Get<FVector>(MemberOffsets::FortPlayerState::PawnDeathLocation) = DeathLocation;

	static auto OnRep_DeathInfoFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerStateAthena.OnRep_DeathInfo");

	if (OnRep_DeathInfoFn)
	{
		DeadPlayerState->ProcessEvent(OnRep_DeathInfoFn);
	}

	if (KillerPlayerState && KillerPlayerState != DeadPlayerState)
	{
		KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::KillScore)++;

		if (MemberOffsets::FortPlayerStateAthena::TeamKillScore != -1)
			KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::TeamKillScore)++;

		KillerPlayerState->ClientReportKill(DeadPlayerState);
		// KillerPlayerState->OnRep_Kills();
	}

	LOG_INFO(LogDev, "Reported kill.");

	/* if (KillerPawn && KillerPawn != DeadPawn)
	{
		KillerPawn->SetHealth(100);
		KillerPawn->SetShield(100);
	} */

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

			// LOG_INFO(LogDev, "[{}/{}] CurrentItemInstance {}", i, ItemInstances.Num(), __int64(ItemInstance));

			if (!ItemInstance)
				continue;

			auto ItemEntry = ItemInstance->GetItemEntry();
			auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->GetItemDefinition());

			// LOG_INFO(LogDev, "[{}/{}] WorldItemDefinition {}", i, ItemInstances.Num(), WorldItemDefinition ? WorldItemDefinition->GetFullName() : "InvalidObject");

			if (!WorldItemDefinition)
				continue;

			auto ShouldBeDropped = WorldItemDefinition->CanBeDropped(); // WorldItemDefinition->ShouldDropOnDeath();

			// LOG_INFO(LogDev, "[{}/{}] ShouldBeDropped {}", i, ItemInstances.Num(), ShouldBeDropped);

			if (!ShouldBeDropped)
				continue;

			AFortPickup::SpawnPickup(WorldItemDefinition, DeathLocation, ItemEntry->GetCount(), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination,
				ItemEntry->GetLoadedAmmo());

			GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });
			// WorldInventory->RemoveItem(ItemEntry->GetItemGuid(), nullptr, ItemEntry->GetCount());
		}

		for (auto& Pair : GuidAndCountsToRemove)
		{
			WorldInventory->RemoveItem(Pair.first, nullptr, Pair.second, true);
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

			if (DamageCauser)
			{
				if (DamageCauser->IsA(FortProjectileBaseClass))
				{
					LOG_INFO(LogDev, "From a projectile!");
					auto Owner = Cast<AFortWeapon>(DamageCauser->GetOwner());
					KillerWeaponDef = Owner->IsValidLowLevel() ? Owner->GetWeaponData() : nullptr; // I just added the IsValidLowLevel check because what if the weapon destroys?
				}
				if (auto Weapon = Cast<AFortWeapon>(DamageCauser))
				{
					LOG_INFO(LogDev, "From a weapon!");
					KillerWeaponDef = Weapon->GetWeaponData();
				}
			}

			// LOG_INFO(LogDev, "KillerWeaponDef: {}", KillerWeaponDef ? KillerWeaponDef->GetFullName() : "InvalidObject");

			RemoveFromAlivePlayers(GameMode, PlayerController, KillerPlayerState == DeadPlayerState ? nullptr : KillerPlayerState, KillerPawn, KillerWeaponDef, DeathCause, 0);
		
			LOG_INFO(LogDev, "Removed!");

			if (Fortnite_Version < 6) // Spectating
			{
				static auto bAllowSpectateAfterDeathOffset = GameMode->GetOffset("bAllowSpectateAfterDeath");

				bool bAllowSpectate = GameMode->Get<bool>(bAllowSpectateAfterDeathOffset);

				LOG_INFO(LogDev, "bAllowSpectate: {}", bAllowSpectate);

				if (bAllowSpectate)
				{
					LOG_INFO(LogDev, "Starting Spectating!");

					static auto PlayerToSpectateOnDeathOffset = PlayerController->GetOffset("PlayerToSpectateOnDeath");
					PlayerController->Get<APawn*>(PlayerToSpectateOnDeathOffset) = KillerPawn;

					PlayerControllersDead.push_back(PlayerController);

					/* if (numValidElements < PlayerControllersDead.size())
					{
						PlayerControllersDead[numValidElements].store(PlayerController);
						numValidElements.fetch_add(1);
					} */

					static bool bCreatedThread = false;

					if (!bCreatedThread)
					{
						bCreatedThread = true;

						CreateThread(0, 0, SpectateThread, 0, 0, 0);
					}
				}
			}
		}
	}

	return ClientOnPawnDiedOriginal(PlayerController, DeathReport);
}

void AFortPlayerController::ServerBeginEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToEdit)
{
	if (!BuildingActorToEdit || !BuildingActorToEdit->IsPlayerPlaced()) // We need more checks.
		return;

	auto Pawn = PlayerController->GetMyFortPawn();

	if (!Pawn)
		return;

	auto PlayerState = PlayerController->GetPlayerState();

	if (!PlayerState)
		return;

	BuildingActorToEdit->SetEditingPlayer(PlayerState);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

	auto EditToolInstance = WorldInventory->FindItemInstance(EditToolDef);

	if (!EditToolInstance)
		return;

	Pawn->EquipWeaponDefinition(EditToolDef, EditToolInstance->GetItemEntry()->GetItemGuid());

	auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->GetCurrentWeapon());

	if (!EditTool)
		return;

	EditTool->GetEditActor() = BuildingActorToEdit;
	EditTool->OnRep_EditActor();
}

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

	// LOG_INFO(LogDev, "RotationIterations: {}", RotationIterations);

	if (!BuildingActorToEdit || !NewBuildingClass || BuildingActorToEdit->IsDestroyed() || BuildingActorToEdit->GetEditingPlayer() != PlayerState)
	{
		LOG_INFO(LogDev, "Cheater?");
		LOG_INFO(LogDev, "BuildingActorToEdit->GetEditingPlayer(): {} PlayerState: {} NewBuildingClass: {} BuildingActorToEdit: {}", BuildingActorToEdit ? __int64(BuildingActorToEdit->GetEditingPlayer()) : -1, __int64(PlayerState), __int64(NewBuildingClass), __int64(BuildingActorToEdit));
		return ServerEditBuildingActorOriginal(Context, Stack, Ret);
	}

	// if (!PlayerState || PlayerState->GetTeamIndex() != BuildingActorToEdit->GetTeamIndex()) 
		//return ServerEditBuildingActorOriginal(Context, Frame, Ret);

	BuildingActorToEdit->SetEditingPlayer(nullptr);

	static ABuildingSMActor* (*BuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) =
		decltype(BuildingSMActorReplaceBuildingActor)(Addresses::ReplaceBuildingActor);

	if (auto BuildingActor = BuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass,
		BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, PlayerController))
	{
		BuildingActor->SetPlayerPlaced(true);
	}

	// we should do more things here

	return ServerEditBuildingActorOriginal(Context, Stack, Ret);
}

void AFortPlayerController::ServerEndEditingBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToStopEditing)
{
	auto Pawn = PlayerController->GetMyFortPawn();

	if (!BuildingActorToStopEditing || !Pawn
		|| BuildingActorToStopEditing->GetEditingPlayer() != PlayerController->GetPlayerState()
		|| BuildingActorToStopEditing->IsDestroyed())
		return;

	BuildingActorToStopEditing->SetEditingPlayer(nullptr);

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto EditToolInstance = WorldInventory->FindItemInstance(EditToolDef);

	if (!EditToolInstance)
		return;

	Pawn->EquipWeaponDefinition(EditToolDef, EditToolInstance->GetItemEntry()->GetItemGuid());

	auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->GetCurrentWeapon());

	BuildingActorToStopEditing->GetEditingPlayer() = nullptr;
	// BuildingActorToStopEditing->OnRep_EditingPlayer();

	if (EditTool)
	{
		EditTool->GetEditActor() = nullptr;
		EditTool->OnRep_EditActor();
	}
}