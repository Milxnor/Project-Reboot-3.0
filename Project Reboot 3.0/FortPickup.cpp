#include "FortPickup.h"

#include "FortPawn.h"
#include "FortItemDefinition.h"
#include "FortPlayerState.h"
#include "FortPlayerPawn.h"
#include "FortPlayerController.h"

void AFortPickup::TossPickup(FVector FinalLocation, AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.TossPickup");

	struct { FVector FinalLocation; AFortPawn* ItemOwner; int OverrideMaxStackCount; bool bToss;
	EFortPickupSourceTypeFlag InPickupSourceTypeFlags; EFortPickupSpawnSource InPickupSpawnSource; }
	AFortPickup_TossPickup_Params{FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource};

	this->ProcessEvent(fn, &AFortPickup_TossPickup_Params);
}

AFortPickup* AFortPickup::SpawnPickup(UFortItemDefinition* ItemDef, FVector Location, int Count, EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource, int LoadedAmmo, AFortPawn* Pawn, UClass* OverrideClass)
{
	static auto FortPickupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickup");
	auto PlayerState = Pawn ? Cast<AFortPlayerState>(Pawn->GetPlayerState()) : nullptr;

	FActorSpawnParameters SpawnParameters{};
	// SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (auto Pickup = GetWorld()->SpawnActor<AFortPickup>(OverrideClass ? OverrideClass : FortPickupClass, Location, FQuat(), FVector(1, 1, 1), SpawnParameters))
	{
		static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");
		Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) = Pawn;

		/* static auto SpecialActorIDOffset = Pickup->GetOffset("SpecialActorID");

		if (auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDef))
		{
			static auto PickupSpecialActorUniqueIDOffset = WorldItemDefinition->GetOffset("PickupSpecialActorUniqueID");
			auto& PickupSpecialActorUniqueID = WorldItemDefinition->Get<FName>(PickupSpecialActorUniqueIDOffset);
			Pickup->Get<FName>(SpecialActorIDOffset) = PickupSpecialActorUniqueID;
		} */

		auto PrimaryPickupItemEntry = Pickup->GetPrimaryPickupItemEntry();

		PrimaryPickupItemEntry->GetCount() = Count;
		PrimaryPickupItemEntry->GetItemDefinition() = ItemDef;
		PrimaryPickupItemEntry->GetLoadedAmmo() = LoadedAmmo;

		// static auto OptionalOwnerIDOffset = Pickup->GetOffset("OptionalOwnerID");
		// Pickup->Get<int>(OptionalOwnerIDOffset) = PlayerState ? PlayerState->GetWorldPlayerId() : -1;

		bool bToss = true;

		if (bToss)
		{
			PickupSource |= EFortPickupSourceTypeFlag::Tossed;
		}

		Pickup->TossPickup(Location, Pawn, 0, bToss, PickupSource, SpawnSource);

		if (PickupSource == EFortPickupSourceTypeFlag::Container)
		{
			static auto bTossedFromContainerOffset = Pickup->GetOffset("bTossedFromContainer");
			Pickup->Get<bool>(bTossedFromContainerOffset) = true;
			// Pickup->OnRep_TossedFromContainer();
		} // crashes if we do this then tosspickup

		return Pickup;
	}

	return nullptr;
}

char AFortPickup::CompletePickupAnimationHook(AFortPickup* Pickup)
{
	constexpr bool bTestPrinting = false; // we could just use our own logger but eh

	if constexpr (bTestPrinting)
		LOG_INFO(LogDev, "CompletePickupAnimationHook!");

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
	
	if constexpr (bTestPrinting)
		LOG_INFO(LogDev, "Start cpyCount: {}", cpyCount);

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
				PrimarySlotsFilled++;
			}

			// LOG_INFO(LogDev, "[{}] PrimarySlotsFilled: {}", i, PrimarySlotsFilled);

			bIsInventoryFull = (PrimarySlotsFilled /* - 6 */) >= 5;

			if (bIsInventoryFull) // probs shouldnt do in loop but alr
			{
				if (ItemInstanceToSwap && ItemDefinitionToSwap->CanBeDropped() && !bHasSwapped)
				{
					auto SwappedPickup = SpawnPickup(ItemDefinitionToSwap, PawnLoc, ItemEntryToSwap->GetCount(), 
						EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, ItemEntryToSwap->GetLoadedAmmo(), Pawn);

					WorldInventory->RemoveItem(CurrentItemGuid, nullptr, ItemEntryToSwap->GetCount(), true);

					bHasSwapped = true;

					if constexpr (bTestPrinting)
						LOG_INFO(LogDev, "[{}] Swapping: {}", i, ItemDefinitionToSwap->GetFullName());

					continue; // ???
				}
			}

			if (CurrentItemEntry->GetItemDefinition() == PickupItemDefinition)
			{
				if constexpr (bTestPrinting)
					LOG_INFO(LogDev, "[{}] Found stack of item!", i);

				if (CurrentItemEntry->GetCount() < PickupItemDefinition->GetMaxStackSize())
				{
					int OverStack = CurrentItemEntry->GetCount() + cpyCount - PickupItemDefinition->GetMaxStackSize();
					int AmountToStack = OverStack > 0 ? cpyCount - OverStack : cpyCount;

					cpyCount -= AmountToStack;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->ModifyCount(ItemInstance, AmountToStack, false, &Pairs, false);
					PairsToMarkDirty.push_back(Pairs);

					bEverStacked = true;

					if constexpr (bTestPrinting)
						LOG_INFO(LogDev, "[{}] We are stacking {}.", i, AmountToStack);

					// if (cpyCount > 0)
						// break;
				}

				bDoesStackExist = true;
			}

			if ((bIsInventoryFull || bForceOverflow) && cpyCount > 0) // overflow
			{
				if constexpr (bTestPrinting)
					LOG_INFO(LogDev, "[{}] Overflow", i);

				UFortWorldItemDefinition* ItemDefinitionToSpawn = PickupItemDefinition;
				int AmountToSpawn = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				SpawnPickup(ItemDefinitionToSpawn, PawnLoc, AmountToSpawn, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, -1, Pawn);
				cpyCount -= AmountToSpawn;
				bForceOverflow = false;
			}

			if (cpyCount <= 0)
				break;
		}

		if (cpyCount > 0 && !bIsInventoryFull)
		{
			if constexpr (bTestPrinting)
				LOG_INFO(LogDev, "Attempting to add to inventory.");

			if (bDoesStackExist ? PickupItemDefinition->DoesAllowMultipleStacks() : true)
			{
				auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				auto NewItem = WorldInventory->AddItem(PickupItemDefinition, nullptr,
					NewItemCount, PickupEntry->GetLoadedAmmo(), true);

				if constexpr (bTestPrinting)
					LOG_INFO(LogDev, "Added item with count {} to inventory.", NewItemCount);

				// if (NewItem)
					cpyCount -= NewItemCount;
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

	return CompletePickupAnimationOriginal(Pickup);
}