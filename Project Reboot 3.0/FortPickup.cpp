#include "FortPickup.h"

#include "FortPawn.h"
#include "FortItemDefinition.h"
#include "FortPlayerState.h"
#include "FortPlayerPawn.h"
#include "FortPlayerController.h"
#include <memcury.h>

void AFortPickup::TossPickup(FVector FinalLocation, AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.TossPickup");

	struct { FVector FinalLocation; AFortPawn* ItemOwner; int OverrideMaxStackCount; bool bToss;
	EFortPickupSourceTypeFlag InPickupSourceTypeFlags; EFortPickupSpawnSource InPickupSpawnSource; }
	AFortPickup_TossPickup_Params{FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource};

	this->ProcessEvent(fn, &AFortPickup_TossPickup_Params);
}

AFortPickup* AFortPickup::SpawnPickup(FFortItemEntry* ItemEntry, FVector Location,
	EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource,
	class AFortPawn* Pawn, UClass* OverrideClass, bool bToss)
{
	// static auto FortPickupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickup");
	static auto FortPickupAthenaClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickupAthena");
	auto PlayerState = Pawn ? Cast<AFortPlayerState>(Pawn->GetPlayerState()) : nullptr;

	FActorSpawnParameters SpawnParameters{};
	// SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (auto Pickup = GetWorld()->SpawnActor<AFortPickup>(OverrideClass ? OverrideClass : FortPickupAthenaClass, Location, FQuat(), FVector(1, 1, 1), SpawnParameters))
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

		auto OldGuid = PrimaryPickupItemEntry->GetItemGuid();

		if (false)
		{
			CopyStruct(PrimaryPickupItemEntry, ItemEntry, FFortItemEntry::GetStructSize(), FFortItemEntry::GetStruct());
		}
		else
		{
			PrimaryPickupItemEntry->GetItemDefinition() = ItemEntry->GetItemDefinition();
			PrimaryPickupItemEntry->GetCount() = ItemEntry->GetCount();
			PrimaryPickupItemEntry->GetLoadedAmmo() = ItemEntry->GetLoadedAmmo();
		}

		PrimaryPickupItemEntry->GetItemGuid() = OldGuid;

		// Pickup->OnRep_PrimaryPickupItemEntry();

		// static auto OptionalOwnerIDOffset = Pickup->GetOffset("OptionalOwnerID");
		// Pickup->Get<int>(OptionalOwnerIDOffset) = PlayerState ? PlayerState->GetWorldPlayerId() : -1;

		if (bToss)
		{
			PickupSource |= EFortPickupSourceTypeFlag::Tossed;
		}

		Pickup->TossPickup(Location, Pawn, 0, bToss, PickupSource, SpawnSource);

		if (PickupSource == EFortPickupSourceTypeFlag::Container) // crashes if we do this then tosspickup
		{
			static auto bTossedFromContainerOffset = Pickup->GetOffset("bTossedFromContainer");
			Pickup->Get<bool>(bTossedFromContainerOffset) = true;
			// Pickup->OnRep_TossedFromContainer();
		}

		return Pickup;
	}

	return nullptr;
}

AFortPickup* AFortPickup::SpawnPickup(UFortItemDefinition* ItemDef, FVector Location, int Count, EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource,
	int LoadedAmmo, AFortPawn* Pawn, UClass* OverrideClass, bool bToss)
{
	auto ItemEntry = FFortItemEntry::MakeItemEntry(ItemDef, Count, LoadedAmmo);
	auto Pickup = SpawnPickup(ItemEntry, Location, PickupSource, SpawnSource, Pawn, OverrideClass, bToss);
	// VirtualFree(ItemEntry);
	return Pickup;
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

	bool bWasHoldingSameItemWhenSwap = false;

	FGuid NewSwappedItem = FGuid(-1, -1, -1, -1);

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
				if (ItemInstanceToSwap && ItemDefinitionToSwap->CanBeDropped() && !bHasSwapped) // swap
				{
					auto SwappedPickup = SpawnPickup(ItemEntryToSwap, PawnLoc,
						EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn);

					auto CurrentWeapon = Pawn->GetCurrentWeapon();

					if (CurrentWeapon)
					{
						bWasHoldingSameItemWhenSwap = CurrentWeapon->GetItemEntryGuid()  == ItemInstanceToSwap->GetItemEntry()->GetItemGuid();
					}

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
					WorldInventory->ModifyCount(ItemInstance, AmountToStack, false, &Pairs, false, true);
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

				auto NewAndModifiedInstances = WorldInventory->AddItem(PickupEntry, nullptr, true, NewItemCount);

				auto NewVehicleInstance = NewAndModifiedInstances.first[0];

				if (!NewVehicleInstance)
					continue;
				else
					cpyCount -= NewItemCount;

				if constexpr (bTestPrinting)
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
		static auto ClientEquipItemFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerAthena.ClientEquipItem") ? FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerAthena.ClientEquipItem") : FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.ClientEquipItem");

		if (ClientEquipItemFn)
		{
			struct
			{
				FGuid                                       ItemGuid;                                                 // (ConstParm, Parm, ZeroConstructor, ReferenceParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool                                               bForceExecution;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			} AFortPlayerController_ClientEquipItem_Params{ NewSwappedItem, true };

			PlayerController->ProcessEvent(ClientEquipItemFn, &AFortPlayerController_ClientEquipItem_Params);
		}
	}

	return CompletePickupAnimationOriginal(Pickup);
}

UClass* AFortPickup::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPickup");
	return Class;
}