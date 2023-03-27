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

	if (auto Pickup = GetWorld()->SpawnActor<AFortPickup>(OverrideClass ? OverrideClass : FortPickupClass, Location))
	{
		static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");

		auto PrimaryPickupItemEntry = Pickup->GetPrimaryPickupItemEntry();

		PrimaryPickupItemEntry->GetCount() = Count;
		PrimaryPickupItemEntry->GetItemDefinition() = ItemDef;
		PrimaryPickupItemEntry->GetLoadedAmmo() = LoadedAmmo;

		// static auto OptionalOwnerIDOffset = Pickup->GetOffset("OptionalOwnerID");
		// Pickup->Get<int>(OptionalOwnerIDOffset) = PlayerState ? PlayerState->GetWorldPlayerId() : -1;

		Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) = Pawn;

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
	// LOG_INFO(LogDev, "Woah!");

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

	if (!ItemInstanceToSwap)
		return CompletePickupAnimationOriginal(Pickup);

	auto ItemEntryToSwap = ItemInstanceToSwap->GetItemEntry();
	auto ItemDefinitionToSwap = ItemEntryToSwap ? Cast<UFortWorldItemDefinition>(ItemEntryToSwap->GetItemDefinition()) : nullptr;
	bool bHasSwapped = false;

	int cpyCount = IncomingCount;

	auto PawnLoc = Pawn->GetActorLocation();
	auto ItemDefGoingInPrimary = IsPrimaryQuickbar(PickupItemDefinition);

	std::vector<std::pair<FFortItemEntry*, FFortItemEntry*>> PairsToMarkDirty; // vector of sets or something so no duplicates??

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

			bIsInventoryFull = (PrimarySlotsFilled /* - 6 */) >= 5;

			if (bIsInventoryFull) // probs shouldnt do in loop but alr
			{
				if (ItemInstanceToSwap && ItemDefinitionToSwap->CanBeDropped() && !bHasSwapped)
				{
					auto SwappedPickup = SpawnPickup(ItemDefinitionToSwap, PawnLoc, ItemEntryToSwap->GetCount(), 
						EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, ItemEntryToSwap->GetLoadedAmmo(), Pawn);

					WorldInventory->RemoveItem(CurrentItemGuid, nullptr, ItemEntryToSwap->GetCount());

					bHasSwapped = true;

					continue; // ???
				}
			}

			if (CurrentItemEntry->GetItemDefinition() == PickupItemDefinition)
			{
				if (CurrentItemEntry->GetCount() < PickupItemDefinition->GetMaxStackSize())
				{
					int OverStack = CurrentItemEntry->GetCount() + cpyCount - PickupItemDefinition->GetMaxStackSize();
					int AmountToStack = OverStack > 0 ? cpyCount - OverStack : cpyCount;

					cpyCount -= AmountToStack;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->ModifyCount(ItemInstance, AmountToStack, false, &Pairs, false);
					PairsToMarkDirty.push_back(Pairs);

					bEverStacked = true;

					// if (cpyCount > 0)
						// break;
				}

				bDoesStackExist = true;
			}

			if ((bIsInventoryFull || bForceOverflow) && cpyCount > 0) // overflow
			{
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
			if (bDoesStackExist ? PickupItemDefinition->DoesAllowMultipleStacks() : true)
			{
				auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				auto NewItem = WorldInventory->AddItem(PickupItemDefinition, nullptr,
					NewItemCount, PickupEntry->GetLoadedAmmo(), true);

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