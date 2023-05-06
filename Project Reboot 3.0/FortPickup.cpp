#include "FortPickup.h"

#include "FortPawn.h"
#include "FortItemDefinition.h"
#include "FortPlayerState.h"
#include "FortPlayerPawn.h"
#include "FortGameModePickup.h"
#include "FortPlayerController.h"
#include <memcury.h>
#include "GameplayStatics.h"
#include "gui.h"

void AFortPickup::TossPickup(FVector FinalLocation, AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.TossPickup");

	struct { FVector FinalLocation; AFortPawn* ItemOwner; int OverrideMaxStackCount; bool bToss;
	EFortPickupSourceTypeFlag InPickupSourceTypeFlags; EFortPickupSpawnSource InPickupSpawnSource; }
	AFortPickup_TossPickup_Params{FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource};

	this->ProcessEvent(fn, &AFortPickup_TossPickup_Params);
}

void AFortPickup::SpawnMovementComponent()
{
	static auto ProjectileMovementComponentClass = FindObject<UClass>("/Script/Engine.ProjectileMovementComponent"); // UFortProjectileMovementComponent

	static auto MovementComponentOffset = this->GetOffset("MovementComponent");
	this->Get(MovementComponentOffset) = UGameplayStatics::SpawnObject(ProjectileMovementComponentClass, this);
}

AFortPickup* AFortPickup::SpawnPickup(FFortItemEntry* ItemEntry, FVector Location,
	EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource,
	class AFortPawn* Pawn, UClass* OverrideClass, bool bToss, int OverrideCount, AFortPickup* IgnoreCombinePickup)
{
	if (bToss)
	{
		PickupSource |= EFortPickupSourceTypeFlag::Tossed;
	}

	// static auto FortPickupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickup");
	static auto FortPickupAthenaClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickupAthena");
	auto PlayerState = Pawn ? Cast<AFortPlayerState>(Pawn->GetPlayerState()) : nullptr;

	FActorSpawnParameters SpawnParameters{};
	// SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (auto Pickup = GetWorld()->SpawnActor<AFortPickup>(OverrideClass ? OverrideClass : FortPickupAthenaClass, Location, FQuat(), FVector(1, 1, 1), SpawnParameters))
	{
		static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");
		Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) = Pawn;

		auto PrimaryPickupItemEntry = Pickup->GetPrimaryPickupItemEntry();

		if (Addresses::PickupInitialize)
		{
			static void (*SetupPickup)(AFortPickup* Pickup, __int64 ItemEntry, TArray<FFortItemEntry> MultiItemPickupEntriesIGuess, bool bSplitOnPickup)
				= decltype(SetupPickup)(Addresses::PickupInitialize);

			TArray<FFortItemEntry> MultiItemPickupEntriesIGuess{};
			SetupPickup(Pickup, __int64(ItemEntry), MultiItemPickupEntriesIGuess, false);
			FFortItemEntry::FreeArrayOfEntries(MultiItemPickupEntriesIGuess);
		}
		else
		{
			PrimaryPickupItemEntry->CopyFromAnotherItemEntry(ItemEntry);
		}
		
		static auto PickupSourceTypeFlagsOffset = Pickup->GetOffset("PickupSourceTypeFlags", false);

		if (PickupSourceTypeFlagsOffset != -1)
			Pickup->Get<int32>(PickupSourceTypeFlagsOffset) |= (int)PickupSource; // Assuming its the same enum on older versions.

		PrimaryPickupItemEntry->GetCount() = OverrideCount == -1 ? ItemEntry->GetCount() : OverrideCount;

		// PrimaryPickupItemEntry->GetItemGuid() = OldGuid;

		// Pickup->OnRep_PrimaryPickupItemEntry();

		// static auto OptionalOwnerIDOffset = Pickup->GetOffset("OptionalOwnerID");
		// Pickup->Get<int>(OptionalOwnerIDOffset) = PlayerState ? PlayerState->GetWorldPlayerId() : -1;
		
		auto PickupLocationData = Pickup->GetPickupLocationData();

		auto CanCombineWithPickup = [&](AActor* OtherPickupActor) -> bool
		{
			auto OtherPickup = (AFortPickup*)OtherPickupActor;

			if (OtherPickup == IgnoreCombinePickup || OtherPickup->GetPickupLocationData()->GetCombineTarget())
				return false;

			if (PrimaryPickupItemEntry->GetItemDefinition() == OtherPickup->GetPrimaryPickupItemEntry()->GetItemDefinition())
			{
				// auto IncomingCount = OtherPickup->GetPrimaryPickupItemEntry()->GetCount();

				// if (PrimaryPickupItemEntry->GetCount() + IncomingCount == PrimaryPickupItemEntry->GetItemDefinition()->GetMaxStackSize())
					// return false;

				if (OtherPickup->GetPrimaryPickupItemEntry()->GetCount() == PrimaryPickupItemEntry->GetItemDefinition()->GetMaxStackSize()) // Other pickup is already at the max size.
					return false;

				return true;
			}

			return false;
		};

		if (Addresses::CombinePickupLea)
		{
			PickupLocationData->GetCombineTarget() = (AFortPickup*)Pickup->GetClosestActor(AFortPickup::StaticClass(), 4, CanCombineWithPickup);
		}

		if (!PickupLocationData->GetCombineTarget()) // I don't think we should call TossPickup for every pickup anyways.
		{
			Pickup->TossPickup(Location, Pawn, 0, bToss, PickupSource, SpawnSource);
		}
		else
		{
			auto ActorLocation = Pickup->GetActorLocation();
			auto CurrentActorLocation = PickupLocationData->GetCombineTarget()->GetActorLocation();

			int Dist = float(sqrtf(powf(CurrentActorLocation.X - ActorLocation.X, 2.0) + powf(CurrentActorLocation.Y - ActorLocation.Y, 2.0) + powf(CurrentActorLocation.Z - ActorLocation.Z, 2.0))) / 100.f;

			// LOG_INFO(LogDev, "Distance: {}", Dist);

			// our little remake of tosspickup

			PickupLocationData->GetLootFinalPosition() = Location;
			PickupLocationData->GetLootInitialPosition() = Pickup->GetActorLocation();
			PickupLocationData->GetFlyTime() = 1.f / Dist; // Higher the dist quicker it should be. // not right
			PickupLocationData->GetItemOwner() = Pawn;
			PickupLocationData->GetFinalTossRestLocation() = PickupLocationData->GetCombineTarget()->GetActorLocation(); // Pickup->GetActorLocation() // ong ong proper

			Pickup->OnRep_PickupLocationData();
			Pickup->ForceNetUpdate();
		}

		if (PickupSource == EFortPickupSourceTypeFlag::Container) // crashes if we do this then tosspickup
		{
			static auto bTossedFromContainerOffset = Pickup->GetOffset("bTossedFromContainer");
			Pickup->Get<bool>(bTossedFromContainerOffset) = true;
			// Pickup->OnRep_TossedFromContainer();
		}

		if (Fortnite_Version < 6)
		{
			Pickup->SpawnMovementComponent();
		}

		return Pickup;
	}

	return nullptr;
}

AFortPickup* AFortPickup::SpawnPickup(UFortItemDefinition* ItemDef, FVector Location, int Count, EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource,
	int LoadedAmmo, AFortPawn* Pawn, UClass* OverrideClass, bool bToss, AFortPickup* IgnoreCombinePickup)
{
	if (LoadedAmmo == -1)
	{
		if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDef)) // bPreventDefaultPreload ?
			LoadedAmmo = WeaponDef->GetClipSize();
		else
			LoadedAmmo = 0;
	}

	auto ItemEntry = FFortItemEntry::MakeItemEntry(ItemDef, Count, LoadedAmmo);
	auto Pickup = SpawnPickup(ItemEntry, Location, PickupSource, SpawnSource, Pawn, OverrideClass, bToss, -1, IgnoreCombinePickup);
	// VirtualFree(ItemEntry);
	return Pickup;
}

void AFortPickup::CombinePickupHook(AFortPickup* Pickup)
{
	// LOG_INFO(LogDev, "CombinePickupHook!");

	auto PickupToCombineInto = (AFortPickup*)Pickup->GetPickupLocationData()->GetCombineTarget();

	if (PickupToCombineInto->IsActorBeingDestroyed())
		return;

	const int IncomingCount = Pickup->GetPrimaryPickupItemEntry()->GetCount();
	const int OriginalCount = PickupToCombineInto->GetPrimaryPickupItemEntry()->GetCount();

	// add more checks?

	auto ItemDefinition = PickupToCombineInto->GetPrimaryPickupItemEntry()->GetItemDefinition();

	int CountToAdd = IncomingCount;

	if (OriginalCount + CountToAdd > ItemDefinition->GetMaxStackSize())
	{
		const int OverStackCount = OriginalCount + CountToAdd - ItemDefinition->GetMaxStackSize();
		CountToAdd = ItemDefinition->GetMaxStackSize() - OriginalCount;

		auto ItemOwner = Pickup->GetPickupLocationData()->GetItemOwner();

		auto NewOverStackPickup = AFortPickup::SpawnPickup(ItemDefinition, PickupToCombineInto->GetActorLocation(), OverStackCount,
			EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, -1, ItemOwner, nullptr, false, PickupToCombineInto);
	}

	PickupToCombineInto->GetPrimaryPickupItemEntry()->GetCount() += CountToAdd;
	PickupToCombineInto->OnRep_PrimaryPickupItemEntry();

	PickupToCombineInto->ForceNetUpdate();
	PickupToCombineInto->FlushNetDormancy();

	Pickup->K2_DestroyActor();
}

char AFortPickup::CompletePickupAnimationHook(AFortPickup* Pickup)
{
	auto Pawn = Cast<AFortPlayerPawn>(Pickup->GetPickupLocationData()->GetPickupTarget());

	if (!Pawn)
		return CompletePickupAnimationOriginal(Pickup);

	auto PlayerController = Cast<AFortPlayerController>(Pawn->GetController());

	if (!PlayerController)
		return CompletePickupAnimationOriginal(Pickup);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return CompletePickupAnimationOriginal(Pickup);

	auto PickupEntry = Pickup->GetPrimaryPickupItemEntry();

	auto PickupItemDefinition = Cast<UFortWorldItemDefinition>(PickupEntry->GetItemDefinition());
	auto IncomingCount = PickupEntry->GetCount();

	if (!PickupItemDefinition)
		return CompletePickupAnimationOriginal(Pickup);

	if (auto GameModePickup = Cast<AFortGameModePickup>(Pickup))
	{
		LOG_INFO(LogDev, "GameModePickup!");
		return CompletePickupAnimationOriginal(Pickup);
	}

	auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();

	auto& CurrentItemGuid = Pickup->GetPickupLocationData()->GetPickupGuid(); // Pawn->CurrentWeapon->ItemEntryGuid;

	auto ItemInstanceToSwap = WorldInventory->FindItemInstance(CurrentItemGuid);

	// if (!ItemInstanceToSwap)
		// return CompletePickupAnimationOriginal(Pickup);

	auto ItemEntryToSwap = ItemInstanceToSwap ? ItemInstanceToSwap->GetItemEntry() : nullptr;
	auto ItemDefinitionToSwap = ItemEntryToSwap ? Cast<UFortWorldItemDefinition>(ItemEntryToSwap->GetItemDefinition()) : nullptr;
	bool bHasSwapped = false;

	int cpyCount = IncomingCount;

	auto PawnLoc = Pawn->GetActorLocation();
	auto ItemDefGoingInPrimary = IsPrimaryQuickbar(PickupItemDefinition);

	std::vector<std::pair<FFortItemEntry*, FFortItemEntry*>> PairsToMarkDirty; // vector of sets or something so no duplicates??
	
	if (bDebugPrintSwapping)
		LOG_INFO(LogDev, "Start cpyCount: {}", cpyCount);

	bool bWasHoldingSameItemWhenSwap = false;

	FGuid NewSwappedItem = FGuid(-1, -1, -1, -1);

	bool bForceDontAddItem = false;
	bool bForceOverflow = false;

	while (cpyCount > 0)
	{
		int PrimarySlotsFilled = 0;
		bool bEverStacked = false;
		bool bDoesStackExist = false;

		bool bIsInventoryFull = false;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances.at(i);
			auto CurrentItemEntry = ItemInstance->GetItemEntry();

			if (ItemDefGoingInPrimary && IsPrimaryQuickbar(CurrentItemEntry->GetItemDefinition()))
			{
				int AmountOfSlotsTakenUp = 1; // TODO
				PrimarySlotsFilled += AmountOfSlotsTakenUp;
			}

			// LOG_INFO(LogDev, "[{}] PrimarySlotsFilled: {}", i, PrimarySlotsFilled);

			bIsInventoryFull = (PrimarySlotsFilled /* - 6 */) >= 5;

			if (bIsInventoryFull || (PlayerController->HasTryPickupSwap() ? PlayerController->ShouldTryPickupSwap() : false)) // probs shouldnt do in loop but alr
			{
				if (PlayerController->HasTryPickupSwap())
					PlayerController->ShouldTryPickupSwap() = false;

				if (ItemInstanceToSwap && ItemDefinitionToSwap->CanBeDropped() && !bHasSwapped && ItemDefGoingInPrimary) // swap
				{
					auto SwappedPickup = SpawnPickup(ItemEntryToSwap, PawnLoc,
						EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);

					auto CurrentWeapon = Pawn->GetCurrentWeapon();

					if (CurrentWeapon)
					{
						bWasHoldingSameItemWhenSwap = CurrentWeapon->GetItemEntryGuid()  == ItemInstanceToSwap->GetItemEntry()->GetItemGuid();
					}

					// THIS IS NOT PROPER! We should use the commented code but there are some bugs idk why.

					WorldInventory->RemoveItem(CurrentItemGuid, nullptr, ItemEntryToSwap->GetCount(), true);

					/*
					auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->SwapItem(CurrentItemGuid, PickupEntry, cpyCount, &Pairs);
					PairsToMarkDirty.push_back(Pairs);

					cpyCount -= NewItemCount;
					*/

					bHasSwapped = true;

					if (bDebugPrintSwapping)
						LOG_INFO(LogDev, "[{}] Swapping: {}", i, ItemDefinitionToSwap->GetFullName());

					// bForceDontAddItem = true;

					continue; // ???
				}
			}

			if (CurrentItemEntry->GetItemDefinition() == PickupItemDefinition)
			{
				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "[{}] Found stack of item!", i);

				if (CurrentItemEntry->GetCount() < PickupItemDefinition->GetMaxStackSize())
				{
					int OverStack = CurrentItemEntry->GetCount() + cpyCount - PickupItemDefinition->GetMaxStackSize();
					int AmountToStack = OverStack > 0 ? cpyCount - OverStack : cpyCount;

					cpyCount -= AmountToStack;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->ModifyCount(ItemInstance, AmountToStack, false, &Pairs, false, true);
					PairsToMarkDirty.push_back(Pairs);

					bEverStacked = true;

					if (bDebugPrintSwapping)
						LOG_INFO(LogDev, "[{}] We are stacking {}.", i, AmountToStack);

					// if (cpyCount > 0)
						// break;
				}

				bDoesStackExist = true;
			}

			if ((bIsInventoryFull || bForceOverflow) && cpyCount > 0) // overflow
			{
				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "[{}] Overflow", i);

				UFortWorldItemDefinition* ItemDefinitionToSpawn = PickupItemDefinition;
				int AmountToSpawn = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				int LoadedAmmo = 0;

				// SpawnPickup(ItemDefinitionToSpawn, PawnLoc, AmountToSpawn, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, -1, Pawn);
				SpawnPickup(PickupEntry, PawnLoc, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, nullptr, true, AmountToSpawn);
				cpyCount -= AmountToSpawn;
				bForceOverflow = false;
			}

			if (cpyCount <= 0)
				break;
		}

		if (cpyCount > 0 && !bIsInventoryFull && !bForceDontAddItem)
		{
			if (bDebugPrintSwapping)
				LOG_INFO(LogDev, "Attempting to add to inventory.");

			if (bDoesStackExist ? PickupItemDefinition->DoesAllowMultipleStacks() : true)
			{
				auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				auto NewAndModifiedInstances = WorldInventory->AddItem(PickupEntry, nullptr, true, NewItemCount);

				auto NewVehicleInstance = NewAndModifiedInstances.first[0];

				if (!NewVehicleInstance)
					continue;
				else
					cpyCount -= NewItemCount;

				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "Added item with count {} to inventory.", NewItemCount);

				if (bHasSwapped && NewSwappedItem == FGuid(-1, -1, -1, -1))
				{
					NewSwappedItem = NewVehicleInstance->GetItemEntry()->GetItemGuid();
				}
			}
			else
			{
				bForceOverflow = true;
			}
		}
	}

	// auto Item = GiveItem(PlayerController, ItemDef, cpyCount, CurrentPickup->PrimaryPickupItemEntry.LoadedAmmo, true);

	/* for (int i = 0; i < Pawn->IncomingPickups.Num(); i++)
	{
		Pawn->IncomingPickups[i]->PickupLocationData.PickupGuid = Item->ItemEntry.ItemGuid;
	} */

	WorldInventory->Update(PairsToMarkDirty.size() == 0);

	for (auto& [key, value] : PairsToMarkDirty)
	{
		WorldInventory->GetItemList().MarkItemDirty(key);
		WorldInventory->GetItemList().MarkItemDirty(value);
	}

	if (bWasHoldingSameItemWhenSwap && NewSwappedItem != FGuid(-1, -1, -1, -1))
	{
		PlayerController->ClientEquipItem(NewSwappedItem, true);
	}

	return CompletePickupAnimationOriginal(Pickup);
}

UClass* AFortPickup::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPickup");
	return Class;
}