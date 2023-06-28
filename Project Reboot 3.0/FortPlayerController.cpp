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
#include "gui.h"
#include "FortAthenaMutator_InventoryOverride.h"
#include "FortAthenaMutator_TDM.h"

void AFortPlayerController::ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ClientReportDamagedResourceBuilding");

	struct { ABuildingSMActor* BuildingSMActor; EFortResourceType PotentialResourceType; int PotentialResourceCount; bool bDestroyed; bool bJustHitWeakspot; }
	AFortPlayerController_ClientReportDamagedResourceBuilding_Params{BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakspot};

	this->ProcessEvent(fn, &AFortPlayerController_ClientReportDamagedResourceBuilding_Params);
}

void AFortPlayerController::ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution)
{
	static auto ClientEquipItemFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ClientEquipItem") 
		? FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ClientEquipItem") 
		: FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ClientEquipItem");

	if (ClientEquipItemFn)
	{
		struct
		{
			FGuid                                       ItemGuid;                                                 // (ConstParm, Parm, ZeroConstructor, ReferenceParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               bForceExecution;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} AFortPlayerController_ClientEquipItem_Params{ ItemGuid, bForceExecution };

		this->ProcessEvent(ClientEquipItemFn, &AFortPlayerController_ClientEquipItem_Params);
	}
}

bool AFortPlayerController::DoesBuildFree()
{
	if (Globals::bInfiniteMaterials)
		return true;

	static auto bBuildFreeOffset = GetOffset("bBuildFree");
	static auto bBuildFreeFieldMask = GetFieldMask(GetProperty("bBuildFree"));
	return ReadBitfieldValue(bBuildFreeOffset, bBuildFreeFieldMask);
}

void AFortPlayerController::DropAllItems(const std::vector<UFortItemDefinition*>& IgnoreItemDefs, bool bIgnoreSecondaryQuickbar, bool bRemoveIfNotDroppable, bool RemovePickaxe)
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

	auto PickaxeInstance = WorldInventory->GetPickaxeInstance();

	for (int i = 0; i < ItemInstances.Num(); ++i)
	{
		auto ItemInstance = ItemInstances.at(i);

		if (!ItemInstance)
			continue;

		auto ItemEntry = ItemInstance->GetItemEntry();

		if (RemovePickaxe && ItemInstance == PickaxeInstance)
		{
			GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });
			continue;
		}

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
	
		PickupCreateData CreateData;
		CreateData.ItemEntry = ItemEntry;
		CreateData.SpawnLocation = Location;
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetPlayerValue();

		AFortPickup::SpawnPickup(CreateData);
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

	static auto UpdatePlayerCustomCharacterPartsVisualizationFn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.UpdatePlayerCustomCharacterPartsVisualization");

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

		auto CosmeticLoadout = GetCosmeticLoadoutOffset() != -1 ? this->GetCosmeticLoadout() : nullptr;

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

						for (int i = 0; i < BackpackCharacterParts.Num(); ++i)
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
		else
		{
			static auto HeroTypeOffset = PlayerStateAsFort->GetOffset("HeroType");
			ApplyHID(PawnAsFort, PlayerStateAsFort->Get(HeroTypeOffset));
		}

		PlayerStateAsFort->ForceNetUpdate();
		PawnAsFort->ForceNetUpdate();
		this->ForceNetUpdate();

		return;
	}

	UFortKismetLibrary::StaticClass()->ProcessEvent(UpdatePlayerCustomCharacterPartsVisualizationFn, &PlayerStateAsFort);

	PlayerStateAsFort->ForceNetUpdate();
	PawnAsFort->ForceNetUpdate();
	this->ForceNetUpdate();
}

void AFortPlayerController::ServerLoadingScreenDroppedHook(UObject* Context, FFrame* Stack, void* Ret)
{
	LOG_INFO(LogDev, "ServerLoadingScreenDroppedHook!");

	auto PlayerController = (AFortPlayerController*)Context;

	PlayerController->ApplyCosmeticLoadout();

	return ServerLoadingScreenDroppedOriginal(Context, Stack, Ret);
}

void AFortPlayerController::ServerRepairBuildingActorHook(AFortPlayerController* PlayerController, ABuildingSMActor* BuildingActorToRepair)
{
	if (!BuildingActorToRepair 
		// || !BuildingActorToRepair->GetWorld()
		)
		return;

	if (BuildingActorToRepair->GetEditingPlayer())
	{
		// ClientSendMessage
		return;
	}

	float BuildingHealthPercent = BuildingActorToRepair->GetHealthPercent();

	// todo not hardcode these

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

	struct { AFortPlayerController* RepairingController; int ResourcesSpent; } ABuildingSMActor_RepairBuilding_Params{ PlayerController, RepairCost };

	static auto RepairBuildingFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingSMActor.RepairBuilding");
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

	static auto FortGadgetItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.FortGadgetItemDefinition");

	UFortGadgetItemDefinition* GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemDefinition);

	if (GadgetItemDefinition)
	{
		static auto GetWeaponItemDefinition = FindObject<UFunction>(L"/Script/FortniteGame.FortGadgetItemDefinition.GetWeaponItemDefinition");

		if (GetWeaponItemDefinition)
		{
			ItemDefinition->ProcessEvent(GetWeaponItemDefinition, &ItemDefinition);
		}
		else
		{
			static auto GetDecoItemDefinition = FindObject<UFunction>(L"/Script/FortniteGame.FortGadgetItemDefinition.GetDecoItemDefinition");
			ItemDefinition->ProcessEvent(GetDecoItemDefinition, &ItemDefinition);
		}

		// LOG_INFO(LogDev, "Equipping Gadget: {}", ItemDefinition->GetFullName());
	}

	if (auto DecoItemDefinition = Cast<UFortDecoItemDefinition>(ItemDefinition))
	{
		Pawn->PickUpActor(nullptr, DecoItemDefinition); // todo check ret value? // I checked on 1.7.2 and it only returns true if the new weapon is a FortDecoTool
		Pawn->GetCurrentWeapon()->GetItemEntryGuid() = ItemGuid;

		static auto FortDecoTool_ContextTrapStaticClass = FindObject<UClass>(L"/Script/FortniteGame.FortDecoTool_ContextTrap");

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
			static auto FortWeap_BuildingToolClass = FindObject<UClass>(L"/Script/FortniteGame.FortWeap_BuildingTool");

			if (!Weapon->IsA(FortWeap_BuildingToolClass))
				return;

			auto BuildingTool = Weapon;

			using UBuildingEditModeMetadata = UObject;
			using UFortBuildingItemDefinition = UObject;

			static auto OnRep_DefaultMetadataFn = FindObject<UFunction>(L"/Script/FortniteGame.FortWeap_BuildingTool.OnRep_DefaultMetadata");
			static auto DefaultMetadataOffset = BuildingTool->GetOffset("DefaultMetadata");

			static auto RoofPiece = FindObject<UFortBuildingItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
			static auto FloorPiece = FindObject<UFortBuildingItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
			static auto WallPiece = FindObject<UFortBuildingItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
			static auto StairPiece = FindObject<UFortBuildingItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");

			UBuildingEditModeMetadata* OldMetadata = nullptr; // Newer versions
			OldMetadata = BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset);

			if (ItemDefinition == RoofPiece)
			{
				static auto RoofMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Roof/EMP_Roof_RoofC.EMP_Roof_RoofC");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = RoofMetadata;
			}
			else if (ItemDefinition == StairPiece)
			{
				static auto StairMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Stair/EMP_Stair_StairW.EMP_Stair_StairW");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = StairMetadata;
			}
			else if (ItemDefinition == WallPiece)
			{
				static auto WallMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Wall/EMP_Wall_Solid.EMP_Wall_Solid");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = WallMetadata;
			}
			else if (ItemDefinition == FloorPiece)
			{
				static auto FloorMetadata = FindObject<UBuildingEditModeMetadata>(L"/Game/Building/EditModePatterns/Floor/EMP_Floor_Floor.EMP_Floor_Floor");
				BuildingTool->Get<UBuildingEditModeMetadata*>(DefaultMetadataOffset) = FloorMetadata;
			}

			BuildingTool->ProcessEvent(OnRep_DefaultMetadataFn, &OldMetadata);
		}
	}
}

void AFortPlayerController::ServerAttemptInteractHook(UObject* Context, FFrame* Stack, void* Ret)
{
	// static auto LlamaClass = FindObject<UClass>(L"/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C");
	static auto FortAthenaSupplyDropClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaSupplyDrop");
	static auto BuildingItemCollectorActorClass = FindObject<UClass>(L"/Script/FortniteGame.BuildingItemCollectorActor");

	LOG_INFO(LogInteraction, "ServerAttemptInteract!");

	auto Params = Stack->Locals;

	static bool bIsUsingComponent = FindObject<UClass>(L"/Script/FortniteGame.FortControllerComponent_Interaction");

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

	static auto FortAthenaVehicleClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaVehicle");
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

		BuildingContainer->SpawnLoot(PlayerController->GetMyFortPawn());

		BuildingContainer->SetBitfieldValue(bAlreadySearchedOffset, bAlreadySearchedFieldMask, true);
		(*(int*)(__int64(SearchBounceData) + SearchAnimationCountOffset))++;
		BuildingContainer->BounceContainer();

		BuildingContainer->ForceNetUpdate(); // ?

		static auto OnRep_bAlreadySearchedFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingContainer.OnRep_bAlreadySearched");
		// BuildingContainer->ProcessEvent(OnRep_bAlreadySearchedFn);

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

		/* static auto GetSeatWeaponComponentFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.GetSeatWeaponComponent");

		if (GetSeatWeaponComponentFn)
		{
			struct { int SeatIndex; UObject* ReturnValue; } AFortAthenaVehicle_GetSeatWeaponComponent_Params{};

			Vehicle->ProcessEvent(GetSeatWeaponComponentFn, &AFortAthenaVehicle_GetSeatWeaponComponent_Params);

			UObject* WeaponComponent = AFortAthenaVehicle_GetSeatWeaponComponent_Params.ReturnValue;

			if (!WeaponComponent)
				return;

			static auto WeaponSeatDefinitionStructSize = FindObject<UClass>("/Script/FortniteGame.WeaponSeatDefinition")->GetPropertiesSize();
			static auto VehicleWeaponOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "VehicleWeapon");
			static auto SeatIndexOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "SeatIndex");
			static auto WeaponSeatDefinitionsOffset = WeaponComponent->GetOffset("WeaponSeatDefinitions");
			auto& WeaponSeatDefinitions = WeaponComponent->Get<TArray<__int64>>(WeaponSeatDefinitionsOffset);

			for (int i = 0; i < WeaponSeatDefinitions.Num(); ++i)
			{
				auto WeaponSeat = WeaponSeatDefinitions.AtPtr(i, WeaponSeatDefinitionStructSize);

				if (*(int*)(__int64(WeaponSeat) + SeatIndexOffset) != Vehicle->FindSeatIndex(Pawn))
					continue;

				auto VehicleGrantedWeaponItem = (TWeakObjectPtr<UFortItem>*)(__int64(WeaponSeat) + 0x20);

				VehicleGrantedWeaponItem->ObjectIndex = NewVehicleInstance->InternalIndex;
				VehicleGrantedWeaponItem->ObjectSerialNumber = GetItemByIndex(NewVehicleInstance->InternalIndex)->SerialNumber;

				static auto bWeaponEquippedOffset = WeaponComponent->GetOffset("bWeaponEquipped");
				WeaponComponent->Get<bool>(bWeaponEquippedOffset) = true;

				break;
			}
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

		static auto StoneItemData = FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto MetalItemData = FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

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

			PickupCreateData CreateData;
			CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(Entry->GetItemDefinition(), Entry->GetCount(), Entry->GetLoadedAmmo(), MAX_DURABILITY, Entry->GetLevel());
			CreateData.SpawnLocation = LocationToSpawnLoot;
			CreateData.PawnOwner = PlayerController->GetMyFortPawn(); // hmm
			CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

			AFortPickup::SpawnPickup(CreateData);
		}

		static auto bCurrentInteractionSuccessOffset = ItemCollector->GetOffset("bCurrentInteractionSuccess", false);

		if (bCurrentInteractionSuccessOffset != -1)
		{
			static auto bCurrentInteractionSuccessFieldMask = GetFieldMask(ItemCollector->GetProperty("bCurrentInteractionSuccess"));
			ItemCollector->SetBitfieldValue(bCurrentInteractionSuccessOffset, bCurrentInteractionSuccessFieldMask, true); // idek if this is needed
		}

		static auto DoVendDeath = FindObject<UFunction>(L"/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C.DoVendDeath");

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
	auto PlayerController = Cast<AFortPlayerControllerAthena>(Engine_Version < 424 ? PC : ((UActorComponent*)PC)->GetOwner());

	if (Engine_Version < 424 && !Globals::bLateGame.load())
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	if (!PlayerController)
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	// if (!PlayerController->bInAircraft) 
		// return;

	LOG_INFO(LogDev, "ServerAttemptAircraftJumpHook!");

	auto GameMode = (AFortGameModeAthena*)GetWorld()->GetGameMode();
	auto GameState = GameMode->GetGameStateAthena();

	AActor* AircraftToJumpFrom = nullptr;

	static auto AircraftsOffset = GameState->GetOffset("Aircrafts", false);

	if (AircraftsOffset == -1)
	{
		static auto AircraftOffset = GameState->GetOffset("Aircraft");
		AircraftToJumpFrom = GameState->Get<AActor*>(AircraftOffset);
	}
	else
	{
		auto Aircrafts = GameState->GetPtr<TArray<AActor*>>(AircraftsOffset);
		AircraftToJumpFrom = Aircrafts->Num() > 0 ? Aircrafts->at(0) : nullptr; // skunky
	}

	if (!AircraftToJumpFrom)
		return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);

	if (false)
	{
		auto NewPawn = GameMode->SpawnDefaultPawnForHook(GameMode, (AController*)PlayerController, AircraftToJumpFrom);
		PlayerController->Possess(NewPawn);
	}
	else
	{
		if (false)
		{
			// honestly idk why this doesnt work

			auto NAME_Inactive = UKismetStringLibrary::Conv_StringToName(L"NAME_Inactive");

			LOG_INFO(LogDev, "name Comp: {}", NAME_Inactive.ComparisonIndex.Value);

			PlayerController->GetStateName() = NAME_Inactive;
			PlayerController->SetPlayerIsWaiting(true);
			PlayerController->ServerRestartPlayer();
		}
		else
		{
			GameMode->RestartPlayer(PlayerController);
		}

		// we are supposed to do some skydivign stuff here but whatever
	}

	auto NewPawnAsFort = PlayerController->GetMyFortPawn();

	if (Fortnite_Version >= 18) // TODO (Milxnor) Find a better fix and move this
	{
		static auto StormEffectClass = FindObject<UClass>(L"/Game/Athena/SafeZone/GE_OutsideSafeZoneDamage.GE_OutsideSafeZoneDamage_C");
		auto PlayerState = PlayerController->GetPlayerStateAthena();

		PlayerState->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(StormEffectClass, 1, PlayerState->GetAbilitySystemComponent());
	}

	if (NewPawnAsFort)
	{
		NewPawnAsFort->SetHealth(100); // needed with server restart player?
		
		if (Globals::bLateGame)
		{
			NewPawnAsFort->SetShield(100);

			NewPawnAsFort->TeleportTo(AircraftToJumpFrom->GetActorLocation(), FRotator());
		}
	}

	// PlayerController->ServerRestartPlayer();
	// return ServerAttemptAircraftJumpOriginal(PC, ClientRotation);
}

void AFortPlayerController::ServerSuicideHook(AFortPlayerController* PlayerController)
{
	LOG_INFO(LogDev, "Suicide!");

	auto Pawn = PlayerController->GetPawn();

	if (!Pawn)
		return;

	// theres some other checks here idk

	if (!Pawn->IsA(AFortPlayerPawn::StaticClass())) // Why FortPlayerPawn? Ask Fortnite
		return;

	// suicide doesn't actually call force kill but its basically the same function

	static auto ForceKillFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.ForceKill"); // exists on 1.2 and 19.10 with same params so I assume it's the same on every other build.

	FGameplayTag DeathReason; // unused on 1.7.2
	AActor* KillerActor = nullptr; // its just 0 in suicide (not really but easiest way to explain it)

	struct { FGameplayTag DeathReason; AController* KillerController; AActor* KillerActor; } AFortPawn_ForceKill_Params{ DeathReason, PlayerController, KillerActor };

	Pawn->ProcessEvent(ForceKillFn, &AFortPawn_ForceKill_Params);

	//PlayerDeathReport->ServerTimeForRespawn && PlayerDeathReport->ServerTimeForResurrect = 0? // I think this is what they do on 1.7.2 I'm too lazy to double check though.
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

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
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
		ExistingBuildings.Free();
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
	}

	FTransform Transform{};
	Transform.Translation = BuildLocation;
	Transform.Rotation = BuildRotator.Quaternion();
	Transform.Scale3D = { 1, 1, 1 };

	auto BuildingActor = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingClass, Transform);

	if (!BuildingActor)
	{
		ExistingBuildings.Free();
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
	}

	auto MatDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->GetResourceType());

	auto MatInstance = WorldInventory->FindItemInstance(MatDefinition);

	bool bBuildFree = PlayerController->DoesBuildFree();

	// LOG_INFO(LogDev, "MatInstance->GetItemEntry()->GetCount(): {}", MatInstance->GetItemEntry()->GetCount());

	int MinimumMaterial = 10;
	bool bShouldDestroy = MatInstance && MatInstance->GetItemEntry() ? MatInstance->GetItemEntry()->GetCount() < MinimumMaterial : true;

	if (bShouldDestroy && !bBuildFree)
	{
		ExistingBuildings.Free();
		BuildingActor->SilentDie();
		return ServerCreateBuildingActorOriginal(Context, Stack, Ret);
	}

	for (int i = 0; i < ExistingBuildings.Num(); ++i)
	{
		auto ExistingBuilding = ExistingBuildings.At(i);

		ExistingBuilding->K2_DestroyActor();
	}

	ExistingBuildings.Free();

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

	auto Params = CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, false, PlayerController);
	auto NewToy = GetWorld()->SpawnActor<AActor>(ToyClass, SpawnPosition, Params);
	// free(Params); // ?

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
	LOG_INFO(LogDev, "ServerAttemptInventoryDropHook Dropping: {}", Count);

	auto Pawn = PlayerController->GetMyFortPawn();

	if (Count < 0 || !Pawn)
		return;

	if (auto PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController))
	{
		if (PlayerControllerAthena->IsInGhostMode())
			return;
	}

	// TODO If the player is in a vehicle and has a vehicle weapon, don't let them drop.

	auto WorldInventory = PlayerController->GetWorldInventory();
	auto ReplicatedEntry = WorldInventory->FindReplicatedEntry(ItemGuid);

	if (!ReplicatedEntry || ReplicatedEntry->GetCount() < Count)
		return;

	auto ItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntry->GetItemDefinition());

	if (!ItemDefinition || !ItemDefinition->CanBeDropped())
		return;

	static auto DropBehaviorOffset = ItemDefinition->GetOffset("DropBehavior", false);

	EWorldItemDropBehavior DropBehavior = DropBehaviorOffset != -1 ? ItemDefinition->GetDropBehavior() : EWorldItemDropBehavior::EWorldItemDropBehavior_MAX;

	if (!ItemDefinition->ShouldIgnoreRespawningOnDrop() && DropBehavior != EWorldItemDropBehavior::DestroyOnDrop)
	{
		PickupCreateData CreateData;
		CreateData.ItemEntry = ReplicatedEntry;
		CreateData.SpawnLocation = Pawn->GetActorLocation();
		CreateData.bToss = true;
		CreateData.OverrideCount = Count;
		CreateData.PawnOwner = Pawn;
		CreateData.bRandomRotation = true;
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetPlayerValue();
		CreateData.bShouldFreeItemEntryWhenDeconstructed = false;

		auto Pickup = AFortPickup::SpawnPickup(CreateData);

		if (!Pickup)
			return;
	}

	bool bShouldUpdate = false;

	if (!WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, Count, true, DropBehavior == EWorldItemDropBehavior::DropAsPickupDestroyOnEmpty))
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

	auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

	if (!AbilitySystemComponent)
		return;

	UObject* AbilityToUse = nullptr;

	static auto AthenaSprayItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaSprayItemDefinition");
	static auto AthenaToyItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaToyItemDefinition");

	if (EmoteAsset->IsA(AthenaSprayItemDefinitionClass))
	{
		static auto SprayGameplayAbilityDefault = FindObject(L"/Game/Abilities/Sprays/GAB_Spray_Generic.Default__GAB_Spray_Generic_C");
		AbilityToUse = SprayGameplayAbilityDefault;
	}

	else if (EmoteAsset->IsA(AthenaToyItemDefinitionClass))
	{
		static auto ToySpawnAbilityOffset = EmoteAsset->GetOffset("ToySpawnAbility");
		auto& ToySpawnAbilitySoft = EmoteAsset->Get<TSoftObjectPtr<UClass>>(ToySpawnAbilityOffset);

		static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");

		auto ToySpawnAbility = ToySpawnAbilitySoft.Get(BGAClass, true);

		if (ToySpawnAbility)
			AbilityToUse = ToySpawnAbility->CreateDefaultObject();
	}

	// LOG_INFO(LogDev, "Before AbilityToUse: {}", AbilityToUse ? AbilityToUse->GetFullName() : "InvalidObject");

	if (!AbilityToUse)
	{
		static auto EmoteGameplayAbilityDefault = FindObject(L"/Game/Abilities/Emotes/GAB_Emote_Generic.Default__GAB_Emote_Generic_C");
		AbilityToUse = EmoteGameplayAbilityDefault;
	}

	if (!AbilityToUse)
		return;

	static auto AthenaDanceItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaDanceItemDefinition");

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

	if (!Spec)
		return;

	static unsigned int* (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent* ASC, int* outHandle, __int64 Spec, FGameplayEventData* TriggerEventData) = decltype(GiveAbilityAndActivateOnce)(Addresses::GiveAbilityAndActivateOnce); // EventData is only on ue500?

	if (GiveAbilityAndActivateOnce)
	{
		GiveAbilityAndActivateOnce(AbilitySystemComponent, &outHandle, __int64(Spec), nullptr);
	}
}

uint8 ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO = false, AFortPawn* Pawn = nullptr)
{
	static auto ToDeathCauseFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.ToDeathCause");

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

DWORD WINAPI SpectateThread(LPVOID PC)
{
	auto PlayerController = (UObject*)PC;

	if (!PlayerController->IsValidLowLevel())
		return 0;

	auto SpectatingPC = Cast<AFortPlayerControllerAthena>(PlayerController);

	if (!SpectatingPC)
		return 0;

	Sleep(3000);

	LOG_INFO(LogDev, "bugha!");

	SpectatingPC->SpectateOnDeath();

	return 0;
}

DWORD WINAPI RestartThread(LPVOID)
{
	// We should probably use unreal engine's timing system for this.
	// There is no way to restart that I know of without closing the connection to the clients.

	bIsInAutoRestart = true;

	float SecondsBeforeRestart = 10;
	Sleep(SecondsBeforeRestart * 1000);

	LOG_INFO(LogDev, "Auto restarting!");

	Restart();

	bIsInAutoRestart = false;

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

	auto DeathLocation = DeadPawn->GetActorLocation();

	static auto FallDamageEnumValue = 1;

	uint8_t DeathCause = 0;

	if (Fortnite_Version > 1.8 || Fortnite_Version == 1.11)
	{
		auto DeathInfo = DeadPlayerState->GetDeathInfo(); // Alloc<void>(DeathInfoStructSize);
		DeadPlayerState->ClearDeathInfo();

		auto/*&*/ Tags = MemberOffsets::FortPlayerPawn::CorrectTags == 0 ? FGameplayTagContainer()
			: DeadPawn->Get<FGameplayTagContainer>(MemberOffsets::FortPlayerPawn::CorrectTags);
		// *(FGameplayTagContainer*)(__int64(DeathReport) + MemberOffsets::DeathReport::Tags);

		// LOG_INFO(LogDev, "Tags: {}", Tags.ToStringSimple(true));

		DeathCause = ToDeathCause(Tags, false, DeadPawn); // DeadPawn->IsDBNO() ??

		LOG_INFO(LogDev, "DeathCause: {}", (int)DeathCause);
		LOG_INFO(LogDev, "DeadPawn->IsDBNO(): {}", DeadPawn->IsDBNO());
		LOG_INFO(LogDev, "KillerPlayerState: {}", __int64(KillerPlayerState));

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

		static auto OnRep_DeathInfoFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.OnRep_DeathInfo");

		if (OnRep_DeathInfoFn)
		{
			DeadPlayerState->ProcessEvent(OnRep_DeathInfoFn);
		}

		if (KillerPlayerState && KillerPlayerState != DeadPlayerState)
		{
			if (MemberOffsets::FortPlayerStateAthena::KillScore != -1)
				KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::KillScore)++;

			if (MemberOffsets::FortPlayerStateAthena::TeamKillScore != -1)
				KillerPlayerState->Get<int>(MemberOffsets::FortPlayerStateAthena::TeamKillScore)++;

			KillerPlayerState->ClientReportKill(DeadPlayerState);

			/* LoopMutators([&](AFortAthenaMutator* Mutator) {
				if (auto TDM_Mutator = Cast<AFortAthenaMutator_TDM>(Mutator))
				{
					struct
					{
						int                                                EventId;                                                  // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
						int                                                EventParam1;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
						int                                                EventParam2;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
						int                                                EventParam3;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
					} AFortAthenaMutator_TDM_OnMutatorGameplayEvent_Params{ 1, 0, 0, 0 }; 

					static auto TDM_OnMutatorGameplayEventFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaMutator_TDM.OnMutatorGameplayEvent");
					TDM_Mutator->ProcessEvent(TDM_OnMutatorGameplayEventFn, &AFortAthenaMutator_TDM_OnMutatorGameplayEvent_Params);
				}
				}); */

			// KillerPlayerState->OnRep_Kills();
		}

		// LOG_INFO(LogDev, "Reported kill.");

		if (AmountOfHealthSiphon != 0)
		{
			if (KillerPawn && KillerPawn != DeadPawn)
			{
				float Health = KillerPawn->GetHealth();
				float Shield = KillerPawn->GetShield();

				int MaxHealth = 100;
				int MaxShield = 100;
				int AmountGiven = 0;
				/*
				int ShieldGiven = 0;
				int HealthGiven = 0;
				*/

				if ((MaxHealth - Health) > 0)
				{
					int AmountToGive = MaxHealth - Health >= AmountOfHealthSiphon ? AmountOfHealthSiphon : MaxHealth - Health;
					KillerPawn->SetHealth(Health + AmountToGive);
					AmountGiven += AmountToGive;
				}

				if ((MaxShield - Shield) > 0 && AmountGiven < AmountOfHealthSiphon)
				{
					int AmountToGive = MaxShield - Shield >= AmountOfHealthSiphon ? AmountOfHealthSiphon : MaxShield - Shield;
					AmountToGive -= AmountGiven;

					if (AmountToGive > 0)
					{
						KillerPawn->SetShield(Shield + AmountToGive);
						AmountGiven += AmountToGive;
					}
				}
				
				if (AmountGiven > 0)
				{

				}
			}
		}
	}

	bool bIsRespawningAllowed = GameState->IsRespawningAllowed(DeadPlayerState);

	if (!bIsRespawningAllowed)
	{
		bool bDropInventory = true;

		LoopMutators([&](AFortAthenaMutator* Mutator)
			{
				if (auto FortAthenaMutator_InventoryOverride = Cast<AFortAthenaMutator_InventoryOverride>(Mutator))
				{
					if (FortAthenaMutator_InventoryOverride->GetDropAllItemsOverride(DeadPlayerState->GetTeamIndex()) == EAthenaLootDropOverride::ForceKeep)
					{
						bDropInventory = false;
					}
				}
			}
		);

		if (bDropInventory)
		{
			auto WorldInventory = PlayerController->GetWorldInventory();

			if (WorldInventory)
			{
				auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();

				std::vector<std::pair<FGuid, int>> GuidAndCountsToRemove;

				for (int i = 0; i < ItemInstances.Num(); ++i)
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

					PickupCreateData CreateData;
					CreateData.ItemEntry = ItemEntry;
					CreateData.SourceType = EFortPickupSourceTypeFlag::GetPlayerValue();
					CreateData.Source = EFortPickupSpawnSource::GetPlayerEliminationValue();
					CreateData.SpawnLocation = DeathLocation;

					AFortPickup::SpawnPickup(CreateData);

					GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });
					// WorldInventory->RemoveItem(ItemEntry->GetItemGuid(), nullptr, ItemEntry->GetCount());
				}

				for (auto& Pair : GuidAndCountsToRemove)
				{
					WorldInventory->RemoveItem(Pair.first, nullptr, Pair.second, true);
				}

				WorldInventory->Update();
			}
		}

		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		LOG_INFO(LogDev, "PlayersLeft: {} IsDBNO: {}", GameState->GetPlayersLeft(), DeadPawn->IsDBNO());

		if (!DeadPawn->IsDBNO())
		{
			if (bHandleDeath)
			{
				if (Fortnite_Version > 1.8 || Fortnite_Version == 1.11)
				{
					static void (*RemoveFromAlivePlayers)(AFortGameModeAthena * GameMode, AFortPlayerController * PlayerController, APlayerState * PlayerState, APawn * FinisherPawn,
						UFortWeaponItemDefinition * FinishingWeapon, uint8_t DeathCause, char a7)
						= decltype(RemoveFromAlivePlayers)(Addresses::RemoveFromAlivePlayers);

					AActor* DamageCauser = *(AActor**)(__int64(DeathReport) + MemberOffsets::DeathReport::DamageCauser);
					UFortWeaponItemDefinition* KillerWeaponDef = nullptr;

					static auto FortProjectileBaseClass = FindObject<UClass>(L"/Script/FortniteGame.FortProjectileBase");

					if (DamageCauser)
					{
						if (DamageCauser->IsA(FortProjectileBaseClass))
						{
							auto Owner = Cast<AFortWeapon>(DamageCauser->GetOwner());
							KillerWeaponDef = Owner->IsValidLowLevel() ? Owner->GetWeaponData() : nullptr; // I just added the IsValidLowLevel check because what if the weapon destroys (idk)?
						}
						if (auto Weapon = Cast<AFortWeapon>(DamageCauser))
						{
							KillerWeaponDef = Weapon->GetWeaponData();
						}
					}

					RemoveFromAlivePlayers(GameMode, PlayerController, KillerPlayerState == DeadPlayerState ? nullptr : KillerPlayerState, KillerPawn, KillerWeaponDef, DeathCause, 0);

					/*

					STATS:

					Note: This isn't the exact order relative to other functions.

					ClientSendMatchStatsForPlayer
					ClientSendTeamStatsForPlayer
					ClientSendEndBattleRoyaleMatchForPlayer

					*/

					// FAthenaMatchStats.Stats[ERewardSource] // hmm

					/*

					// We need to check if their entire team is dead then I think we send it????

					auto DeadControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);

					if (DeadControllerAthena && FAthenaMatchTeamStats::GetStruct())
					{
						auto MatchReport = DeadControllerAthena->GetMatchReport();

						LOG_INFO(LogDev, "MatchReport: {}", __int64(MatchReport));

						if (MatchReport)
						{
							MatchReport->GetTeamStats()->GetPlace() = DeadPlayerState->GetPlace();
							MatchReport->GetTeamStats()->GetTotalPlayers() = AmountOfPlayersWhenBusStart; // hmm
							MatchReport->HasTeamStats() = true;

							DeadControllerAthena->ClientSendTeamStatsForPlayer(MatchReport->GetTeamStats());
						}
					}

					*/

					LOG_INFO(LogDev, "Removed!");
				}

				// LOG_INFO(LogDev, "KillerPlayerState->Place: {}", KillerPlayerState ? KillerPlayerState->GetPlace() : -1);
			}

			if (Fortnite_Version < 6) // Spectating (is this the actual build or is it like 6.10 when they added it auto).
			{
				static auto bAllowSpectateAfterDeathOffset = GameMode->GetOffset("bAllowSpectateAfterDeath");

				bool bAllowSpectate = GameMode->Get<bool>(bAllowSpectateAfterDeathOffset);

				LOG_INFO(LogDev, "bAllowSpectate: {}", bAllowSpectate);

				if (bAllowSpectate)
				{
					LOG_INFO(LogDev, "Starting Spectating!");

					static auto PlayerToSpectateOnDeathOffset = PlayerController->GetOffset("PlayerToSpectateOnDeath");
					PlayerController->Get<APawn*>(PlayerToSpectateOnDeathOffset) = KillerPawn;

					CreateThread(0, 0, SpectateThread, (LPVOID)PlayerController, 0, 0);
				}
			}
		}

		if (IsRestartingSupported() && Globals::bAutoRestart && !bIsInAutoRestart)
		{
			// wtf

			if (GameState->GetGamePhase() > EAthenaGamePhase::Warmup)
			{
				auto AllPlayerStates = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPlayerStateAthena::StaticClass());

				bool bDidSomeoneWin = AllPlayerStates.Num() == 0;

				for (int i = 0; i < AllPlayerStates.Num(); ++i)
				{
					auto CurrentPlayerState = (AFortPlayerStateAthena*)AllPlayerStates.at(i);

					if (CurrentPlayerState->GetPlace() <= 1)
					{
						bDidSomeoneWin = true;
						break;
					}
				}

				// LOG_INFO(LogDev, "bDidSomeoneWin: {}", bDidSomeoneWin);

				// if (GameState->GetGamePhase() == EAthenaGamePhase::EndGame)
				if (bDidSomeoneWin)
				{
					CreateThread(0, 0, RestartThread, 0, 0, 0);
				}
			}
		}
	}

	if (DeadPlayerState->IsBot())
	{
		// AllPlayerBotsToTick.remov3lbah
	}

	DeadPlayerState->EndDBNOAbilities();

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

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

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
		// LOG_INFO(LogDev, "Cheater?");
		// LOG_INFO(LogDev, "BuildingActorToEdit->GetEditingPlayer(): {} PlayerState: {} NewBuildingClass: {} BuildingActorToEdit: {}", BuildingActorToEdit ? __int64(BuildingActorToEdit->GetEditingPlayer()) : -1, __int64(PlayerState), __int64(NewBuildingClass), __int64(BuildingActorToEdit));
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

	static auto EditToolDef = FindObject<UFortWeaponItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

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