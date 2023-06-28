#include "FortInventory.h"
#include "FortPlayerController.h"
#include "FortPickup.h"
#include "FortQuickBars.h"
#include "FortPlayerPawnAthena.h"
#include "FortGameStateAthena.h"
#include "FortGameModeAthena.h"
#include "FortGadgetItemDefinition.h"
#include "FortPlayerStateAthena.h"

UFortItem* CreateItemInstance(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count)
{
	UFortItem* NewItemInstance = ItemDefinition->CreateTemporaryItemInstanceBP(Count);

	if (NewItemInstance && PlayerController)
		NewItemInstance->SetOwningControllerForTemporaryItem(PlayerController);

	return NewItemInstance;
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AFortInventory::AddItem(FFortItemEntry* ItemEntry, bool* bShouldUpdate, bool bShowItemToast, int OverrideCount)
{
	if (!ItemEntry || !ItemEntry->GetItemDefinition())
		return std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>>();

	if (bShouldUpdate)
		*bShouldUpdate = false;

	auto ItemDefinition = ItemEntry->GetItemDefinition();

	auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);
	auto& Count = ItemEntry->GetCount();

	auto& ItemInstances = GetItemList().GetItemInstances();

	auto MaxStackSize = ItemDefinition->GetMaxStackSize();

	bool bAllowMultipleStacks = ItemDefinition->DoesAllowMultipleStacks();
	int OverStack = 0;

	std::vector<UFortItem*> NewItemInstances;
	std::vector<UFortItem*> ModifiedItemInstances;

	if (MaxStackSize > 1)
	{
		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto CurrentItemInstance = ItemInstances.at(i);
			auto CurrentEntry = CurrentItemInstance->GetItemEntry();

			if (CurrentEntry->GetItemDefinition() == ItemDefinition)
			{
				if (CurrentEntry->GetCount() < MaxStackSize || !bAllowMultipleStacks)
				{
					OverStack = CurrentEntry->GetCount() + Count - MaxStackSize;

					if (!bAllowMultipleStacks && !(CurrentEntry->GetCount() < MaxStackSize))
					{
						break;
					}

					int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

					auto ReplicatedEntry = FindReplicatedEntry(CurrentEntry->GetItemGuid());

					CurrentEntry->GetCount() += AmountToStack;
					ReplicatedEntry->GetCount() += AmountToStack;

					for (int p = 0; p < CurrentEntry->GetStateValues().Num(); p++)
					{
						if (CurrentEntry->GetStateValues().at(p).GetStateType() == EFortItemEntryState::ShouldShowItemToast)
						{
							CurrentEntry->GetStateValues().at(p).GetIntValue() = bShowItemToast;
						}
					}

					for (int p = 0; p < ReplicatedEntry->GetStateValues().Num(); p++)
					{
						if (ReplicatedEntry->GetStateValues().at(p).GetStateType() == EFortItemEntryState::ShouldShowItemToast)
						{
							ReplicatedEntry->GetStateValues().at(p).GetIntValue() = bShowItemToast;
						}
					}

					ModifiedItemInstances.push_back(CurrentItemInstance);

					GetItemList().MarkItemDirty(CurrentEntry);
					GetItemList().MarkItemDirty(ReplicatedEntry);

					if (OverStack <= 0)
						return std::make_pair(NewItemInstances, ModifiedItemInstances);

					// break;
				}
			}
		}
	}

	Count = OverStack > 0 ? OverStack : Count;

	auto PlayerController = Cast<APlayerController>(GetOwner());

	if (!PlayerController)
		return std::make_pair(NewItemInstances, ModifiedItemInstances);

	if (OverStack > 0 && !bAllowMultipleStacks)
	{
		auto Pawn = PlayerController->GetPawn();

		if (!Pawn)
			return std::make_pair(NewItemInstances, ModifiedItemInstances);

		PickupCreateData CreateData;
		CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(ItemDefinition, Count, -1, MAX_DURABILITY/* level */);
		CreateData.SpawnLocation = Pawn->GetActorLocation();
		CreateData.PawnOwner = Cast<AFortPawn>(Pawn);
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetPlayerValue();
		CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

		AFortPickup::SpawnPickup(CreateData);
		return std::make_pair(NewItemInstances, ModifiedItemInstances);
	}

	auto FortPlayerController = Cast<AFortPlayerController>(PlayerController);
	UFortItem* NewItemInstance = CreateItemInstance(FortPlayerController, ItemDefinition, Count);

	if (NewItemInstance)
	{
		NewItemInstance->GetItemEntry()->CopyFromAnotherItemEntry(ItemEntry);

		if (OverrideCount != -1)
			NewItemInstance->GetItemEntry()->GetCount() = OverrideCount;

		NewItemInstances.push_back(NewItemInstance);

		ItemInstances.Add(NewItemInstance);
		auto ReplicatedEntryIdx = GetItemList().GetReplicatedEntries().Add(*NewItemInstance->GetItemEntry(), FFortItemEntry::GetStructSize());
		// GetItemList().GetReplicatedEntries().AtPtr(ReplicatedEntryIdx, FFortItemEntry::GetStructSize())->GetIsReplicatedCopy() = true;

		if (FortPlayerController && WorldItemDefinition->IsValidLowLevel())
		{
			bool AreGadgetsEnabled = Addresses::ApplyGadgetData && Addresses::RemoveGadgetData && Globals::bEnableAGIDs;
			bool bWasGadget = false;
			
			if (AreGadgetsEnabled)
			{
				if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(WorldItemDefinition))
				{
					if (GadgetItemDefinition->ShouldDropAllItemsOnEquip()) // idk shouldnt this be auto?
					{
						FortPlayerController->DropAllItems({ GadgetItemDefinition }, false, false, Fortnite_Version < 7);
					}

					bool (*ApplyGadgetData)(UFortGadgetItemDefinition* a1, __int64 a2, UFortItem* a3, unsigned __int8 a4) = decltype(ApplyGadgetData)(Addresses::ApplyGadgetData);
					static auto FortInventoryOwnerInterfaceClass = FindObject<UClass>("/Script/FortniteGame.FortInventoryOwnerInterface");
					auto Interface = __int64(FortPlayerController->GetInterfaceAddress(FortInventoryOwnerInterfaceClass));
					bool idktbh = true; // Something to do with durability
					
					bool DidApplyingGadgetSucceed = ApplyGadgetData(GadgetItemDefinition, Interface, NewItemInstance, idktbh);
					LOG_INFO(LogDev, "DidApplyingGadgetSucceed: {}", DidApplyingGadgetSucceed);
					bWasGadget = true;
				}
			}

			if (WorldItemDefinition->ShouldFocusWhenAdded()) // Should we also do this for stacking?
			{
				LOG_INFO(LogDev, "Force focus {}", ItemDefinition->GetFullName());
				FortPlayerController->ServerExecuteInventoryItemHook(FortPlayerController, NewItemInstance->GetItemEntry()->GetItemGuid());
				FortPlayerController->ClientEquipItem(NewItemInstance->GetItemEntry()->GetItemGuid(), true);
			}
		}
		else
		{
			LOG_INFO(LogDev, "Not Valid!");
		}

		if (FortPlayerController && Engine_Version < 420)
		{
			static auto QuickBarsOffset = FortPlayerController->GetOffset("QuickBars", false);
			auto QuickBars = FortPlayerController->Get<AActor*>(QuickBarsOffset);

			if (QuickBars)
			{
				struct
				{
					FGuid                                       Item;                                                     // (Parm, IsPlainOldData)
					EFortQuickBars                                     InQuickBar;                                               // (Parm, ZeroConstructor, IsPlainOldData)
					int                                                Slot;                                                     // (Parm, ZeroConstructor, IsPlainOldData)
				}
				AFortQuickBars_ServerAddItemInternal_Params
				{
					NewItemInstance->GetItemEntry()->GetItemGuid(),
					IsPrimaryQuickbar(ItemDefinition) ? EFortQuickBars::Primary : EFortQuickBars::Secondary,
					-1
				};

				static auto ServerAddItemInternalFn = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.ServerAddItemInternal");
				QuickBars->ProcessEvent(ServerAddItemInternalFn, &AFortQuickBars_ServerAddItemInternal_Params);
			}
		}

		/* if (FortPlayerController && WorldItemDefinition) // Hmm
		{
			auto Pawn = Cast<AFortPlayerPawnAthena>(FortPlayerController->GetMyFortPawn());
			auto GameState = Cast<AFortGameStateAthena>(((AFortGameModeAthena*)GetWorld()->GetGameMode())->GetGameState());

			if (Pawn)
			{
				static auto InventorySpecialActorUniqueIDOffset = WorldItemDefinition->GetOffset("InventorySpecialActorUniqueID");
				auto& InventorySpecialActorUniqueID = WorldItemDefinition->Get<FName>(InventorySpecialActorUniqueIDOffset);

				static auto ItemSpecialActorIDOffset = Pawn->GetOffset("ItemSpecialActorID");
				Pawn->Get<FName>(ItemSpecialActorIDOffset) = InventorySpecialActorUniqueID;

				static auto ItemSpecialActorCategoryIDOffset = Pawn->GetOffset("ItemSpecialActorCategoryID");
				Pawn->Get<FName>(ItemSpecialActorCategoryIDOffset) = InventorySpecialActorUniqueID;

				static auto BecameSpecialActorTimeOffset = Pawn->GetOffset("BecameSpecialActorTime");
				Pawn->Get<float>(BecameSpecialActorTimeOffset) = GameState->GetServerWorldTimeSeconds();
			}
		} */

		if (bShouldUpdate)
			*bShouldUpdate = true;
	}

	return std::make_pair(NewItemInstances, ModifiedItemInstances);
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AFortInventory::AddItem(UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count, int LoadedAmmo, bool bShowItemToast)
{
	if (LoadedAmmo == -1)
	{
		if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDefinition)) // bPreventDefaultPreload ?
			LoadedAmmo = WeaponDef->GetClipSize();
		else
			LoadedAmmo = 0;
	}

	auto ItemEntry = FFortItemEntry::MakeItemEntry(ItemDefinition, Count, LoadedAmmo);
	auto Ret = AddItem(ItemEntry, bShouldUpdate, bShowItemToast);

	if (!bUseFMemoryRealloc)
	{
		FFortItemEntry::FreeItemEntry(ItemEntry);
		VirtualFree(ItemEntry, 0, MEM_RELEASE);
	}

	return Ret;
}

bool AFortInventory::RemoveItem(const FGuid& ItemGuid, bool* bShouldUpdate, int Count, bool bForceRemoval, bool bIgnoreVariables)
{
	if (bShouldUpdate)
		*bShouldUpdate = false;

	auto ItemInstance = FindItemInstance(ItemGuid);
	auto ReplicatedEntry = FindReplicatedEntry(ItemGuid);

	if (!ItemInstance || !ReplicatedEntry)
		return false;

	auto ItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntry->GetItemDefinition());

	if (!ItemDefinition)
		return false;

	int OldCount = Count;

	if (Count < 0) // idk why i have this
	{
		Count = 0;
		bForceRemoval = true;
	}

	auto& ItemInstances = GetItemList().GetItemInstances();
	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	auto NewCount = ReplicatedEntry->GetCount() - Count;

	bool bOverrideChangeStackSize = false;

	if (!bIgnoreVariables && ItemDefinition->ShouldPersistWhenFinalStackEmpty())
	{
		bool bIsFinalStack = true;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances.at(i);

			if (ItemInstance->GetItemEntry()->GetItemDefinition() == ItemDefinition && ItemInstance->GetItemEntry()->GetItemGuid() != ItemGuid)
			{
				bIsFinalStack = false;
				break;
			}
		}

		if (bIsFinalStack)
		{
			NewCount = NewCount < 0 ? 0 : NewCount; // min(NewCount, 0) or something i forgot
			bOverrideChangeStackSize = true;
		}
	}

	if (OldCount != -1 && (NewCount > 0 || bOverrideChangeStackSize))
	{
		ItemInstance->GetItemEntry()->GetCount() = NewCount;
		ReplicatedEntry->GetCount() = NewCount;

		GetItemList().MarkItemDirty(ItemInstance->GetItemEntry());
		GetItemList().MarkItemDirty(ReplicatedEntry);

		return true;
	}

	if (NewCount < 0) // Hm
		return false;

	auto FortPlayerController = Cast<AFortPlayerController>(GetOwner());

	bool bWasGadget = false;

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		if (ItemInstances.at(i)->GetItemEntry()->GetItemGuid() == ItemGuid)
		{
			bool AreGadgetsEnabled = Addresses::ApplyGadgetData && Addresses::RemoveGadgetData && Globals::bEnableAGIDs;

			if (FortPlayerController && AreGadgetsEnabled)
			{
				if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemDefinition))
				{
					LOG_INFO(LogDev, "Unequipping Gadget!");
					GadgetItemDefinition->UnequipGadgetData(FortPlayerController, ItemInstances.at(i));

					bWasGadget = true;

					if (bWasGadget)
					{
						if (Fortnite_Version < 7 && GadgetItemDefinition->ShouldDropAllItemsOnEquip())
						{
							FortPlayerController->AddPickaxeToInventory();
						}
					}
				}
			}

			FFortItemEntry::FreeItemEntry(ItemInstances.at(i)->GetItemEntry()); // Really this is deconstructing it, which frees the arrays inside, we have to do this since Remove doesn't.
			ItemInstances.Remove(i);
			break;
		}
	}

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (ReplicatedEntries.at(i, FFortItemEntry::GetStructSize()).GetItemGuid() == ItemGuid)
		{
			FFortItemEntry::FreeItemEntry(ReplicatedEntries.AtPtr(i, FFortItemEntry::GetStructSize()));
			ReplicatedEntries.Remove(i, FFortItemEntry::GetStructSize());
			break;
		}
	}

	if (FortPlayerController && Engine_Version < 420)
	{
		static auto QuickBarsOffset = FortPlayerController->GetOffset("QuickBars", false);
		auto QuickBars = FortPlayerController->Get<AFortQuickBars*>(QuickBarsOffset);

		if (QuickBars)
		{
			auto ItemDefinitionQuickBars = IsPrimaryQuickbar(ItemDefinition) ? EFortQuickBars::Primary : EFortQuickBars::Secondary;
			auto SlotIndex = QuickBars->GetSlotIndex(ItemGuid, ItemDefinitionQuickBars);

			if (SlotIndex != -1)
			{
				QuickBars->ServerRemoveItemInternal(ItemGuid, false, true);
				QuickBars->EmptySlot(ItemDefinitionQuickBars, SlotIndex);
			}
		}
	}

	// todo remove from weaponlist

	if (bShouldUpdate)
		*bShouldUpdate = true;

	return true;
}

void AFortInventory::SwapItem(const FGuid& ItemGuid, FFortItemEntry* NewItemEntry, int OverrideNewCount, std::pair<FFortItemEntry*, FFortItemEntry*>* outEntries)
{
	auto NewCount = OverrideNewCount == -1 ? NewItemEntry->GetCount() : OverrideNewCount;
	
	auto ItemInstance = FindItemInstance(ItemGuid);

	if (!ItemInstance)
		return;

	/* RemoveItem(ItemGuid, nullptr, ItemInstance->GetItemEntry()->GetCount(), true);
	AddItem(NewItemEntry, nullptr, false, OverrideNewCount);

	return; */

	// IDK WHY THIS DOESNT WORK

	static auto FortItemEntrySize = FFortItemEntry::GetStructSize();

	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();
	
	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		auto& ReplicatedEntry = ReplicatedEntries.At(i, FortItemEntrySize);

		if (ReplicatedEntry.GetItemGuid() == ItemGuid)
		{
			ReplicatedEntry.CopyFromAnotherItemEntry(NewItemEntry);
			ItemInstance->GetItemEntry()->CopyFromAnotherItemEntry(NewItemEntry);

			ReplicatedEntry.GetCount() = NewCount;
			ItemInstance->GetItemEntry()->GetCount() = NewCount;
			
			if (outEntries)
				*outEntries = std::make_pair(ItemInstance->GetItemEntry(), &ReplicatedEntry);
		}
	}
}

void AFortInventory::ModifyCount(UFortItem* ItemInstance, int New, bool bRemove, std::pair<FFortItemEntry*, FFortItemEntry*>* outEntries, bool bUpdate, bool bShowItemToast)
{
	auto ReplicatedEntry = FindReplicatedEntry(ItemInstance->GetItemEntry()->GetItemGuid());

	if (!ReplicatedEntry)
		return;

	if (!bRemove)
	{
		ItemInstance->GetItemEntry()->GetCount() += New;
		ReplicatedEntry->GetCount() += New;

		for (int p = 0; p < ItemInstance->GetItemEntry()->GetStateValues().Num(); p++)
		{
			if (ItemInstance->GetItemEntry()->GetStateValues().at(p).GetStateType() == EFortItemEntryState::ShouldShowItemToast)
			{
				ItemInstance->GetItemEntry()->GetStateValues().at(p).GetIntValue() = bShowItemToast;
			}
		}

		for (int p = 0; p < ReplicatedEntry->GetStateValues().Num(); p++)
		{
			if (ReplicatedEntry->GetStateValues().at(p).GetStateType() == EFortItemEntryState::ShouldShowItemToast)
			{
				ReplicatedEntry->GetStateValues().at(p).GetIntValue() = bShowItemToast;
			}
		}
	}
	else
	{
		ItemInstance->GetItemEntry()->GetCount() -= New;
		ReplicatedEntry->GetCount() -= New;
	}

	if (outEntries)
		*outEntries = { ItemInstance->GetItemEntry(), ReplicatedEntry};

	if (bUpdate || !outEntries)
	{
		GetItemList().MarkItemDirty(ItemInstance->GetItemEntry());
		GetItemList().MarkItemDirty(ReplicatedEntry);
	}
}

UFortItem* AFortInventory::GetPickaxeInstance()
{
	static auto FortWeaponMeleeItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.FortWeaponMeleeItemDefinition");

	auto& ItemInstances = GetItemList().GetItemInstances();

	for (int i = 0; i < ItemInstances.Num(); ++i)
	{
		auto ItemInstance = ItemInstances.At(i);

		if (ItemInstance->GetItemEntry() && ItemInstance->GetItemEntry()->GetItemDefinition() &&
			ItemInstance->GetItemEntry()->GetItemDefinition()->IsA(FortWeaponMeleeItemDefinitionClass)
			)
		{
			return ItemInstance;
		}
	}
	
	return nullptr;
}

UFortItem* AFortInventory::FindItemInstance(UFortItemDefinition* ItemDefinition)
{
	auto& ItemInstances = GetItemList().GetItemInstances();

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		auto ItemInstance = ItemInstances.At(i);

		if (ItemInstance->GetItemEntry()->GetItemDefinition() == ItemDefinition)
			return ItemInstance;
	}

	return nullptr;
}

void AFortInventory::CorrectLoadedAmmo(const FGuid& Guid, int NewAmmoCount)
{
	auto CurrentWeaponInstance = FindItemInstance(Guid);

	if (!CurrentWeaponInstance)
		return;

	auto CurrentWeaponReplicatedEntry = FindReplicatedEntry(Guid);

	if (!CurrentWeaponReplicatedEntry)
		return;

	if (CurrentWeaponReplicatedEntry->GetLoadedAmmo() != NewAmmoCount)
	{
		CurrentWeaponInstance->GetItemEntry()->GetLoadedAmmo() = NewAmmoCount;
		CurrentWeaponReplicatedEntry->GetLoadedAmmo() = NewAmmoCount;

		GetItemList().MarkItemDirty(CurrentWeaponInstance->GetItemEntry());
		GetItemList().MarkItemDirty(CurrentWeaponReplicatedEntry);
	}
}

UFortItem* AFortInventory::FindItemInstance(const FGuid& Guid)
{
	auto& ItemInstances = GetItemList().GetItemInstances();

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		auto ItemInstance = ItemInstances.At(i);

		if (ItemInstance->GetItemEntry()->GetItemGuid() == Guid)
			return ItemInstance;
	}

	return nullptr;
}

FFortItemEntry* AFortInventory::FindReplicatedEntry(const FGuid& Guid)
{
	static auto FortItemEntrySize = FFortItemEntry::GetStructSize();

	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		auto& ReplicatedEntry = ReplicatedEntries.At(i, FortItemEntrySize);

		if (ReplicatedEntry.GetItemGuid() == Guid)
			return &ReplicatedEntry;
	}

	return nullptr;
}

/* UClass* AFortInventory::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortInventory");
	return Class;
} */