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

		AFortPickup::SpawnPickup(ItemDefinition, Pawn->GetActorLocation(), Count, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, -1, Cast<AFortPawn>(Pawn));
		return std::make_pair(NewItemInstances, ModifiedItemInstances);
	}

	auto FortPlayerController = Cast<AFortPlayerController>(PlayerController);
	UFortItem* NewItemInstance = CreateItemInstance(FortPlayerController, ItemDefinition, Count);

	if (NewItemInstance)
	{
		auto OldItemGuid = NewItemInstance->GetItemEntry()->GetItemGuid();

		if (false)
		{
			CopyStruct(NewItemInstance->GetItemEntry(), ItemEntry, FFortItemEntry::GetStructSize(), FFortItemEntry::GetStruct());
		}
		else
		{
			NewItemInstance->GetItemEntry()->GetItemDefinition() = ItemEntry->GetItemDefinition();
			NewItemInstance->GetItemEntry()->GetCount() = ItemEntry->GetCount();
			NewItemInstance->GetItemEntry()->GetLoadedAmmo() = ItemEntry->GetLoadedAmmo();
		}		

		NewItemInstance->GetItemEntry()->GetItemGuid() = OldItemGuid;

		NewItemInstance->GetItemEntry()->MostRecentArrayReplicationKey = -1;
		NewItemInstance->GetItemEntry()->ReplicationID = -1;
		NewItemInstance->GetItemEntry()->ReplicationKey = -1;

		if (OverrideCount != -1)
			NewItemInstance->GetItemEntry()->GetCount() = OverrideCount;

		NewItemInstances.push_back(NewItemInstance);

		static auto FortItemEntryStruct = FindObject(L"/Script/FortniteGame.FortItemEntry");
		static auto FortItemEntrySize = *(int*)(__int64(FortItemEntryStruct) + Offsets::PropertiesSize);

		bool bEnableStateValues = false;

		if (bEnableStateValues)
		{
			FFortItemEntryStateValue* StateValue = (FFortItemEntryStateValue*)FMemory::Realloc(0, FFortItemEntryStateValue::GetStructSize(), 0); 
				// Alloc<FFortItemEntryStateValue>(FFortItemEntryStateValue::GetStructSize());
			StateValue->GetIntValue() = bShowItemToast;
			StateValue->GetStateType() = EFortItemEntryState::ShouldShowItemToast;
			NewItemInstance->GetItemEntry()->GetStateValues().AddPtr(StateValue, FFortItemEntryStateValue::GetStructSize());
		}

		ItemInstances.Add(NewItemInstance);
		GetItemList().GetReplicatedEntries().Add(*NewItemInstance->GetItemEntry(), FortItemEntrySize);

		if (WorldItemDefinition->IsValidLowLevel())
		{
			if (WorldItemDefinition->ShouldFocusWhenAdded()) // Should we also do this for stacking?
			{
				FortPlayerController->ServerExecuteInventoryItemHook(FortPlayerController, NewItemInstance->GetItemEntry()->GetItemGuid());
			}

			bool AreGadgetsEnabled = Addresses::ApplyGadgetData && Addresses::RemoveGadgetData && Globals::bEnableAGIDs;

			if (AreGadgetsEnabled)
			{
				if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(WorldItemDefinition))
				{
					if (GadgetItemDefinition->ShouldDropAllItemsOnEquip()) // idk shouldnt this be auto?
					{
						FortPlayerController->DropAllItems({ GadgetItemDefinition });
					}

					bool (*ApplyGadgetData)(UFortGadgetItemDefinition * a1, __int64 a2, UFortItem* a3, unsigned __int8 a4) = decltype(ApplyGadgetData)(Addresses::ApplyGadgetData);
					static auto FortInventoryOwnerInterfaceClass = FindObject<UClass>("/Script/FortniteGame.FortInventoryOwnerInterface");
					LOG_INFO(LogDev, "Res: {}", ApplyGadgetData(GadgetItemDefinition, __int64(PlayerController->GetInterfaceAddress(FortInventoryOwnerInterfaceClass)), NewItemInstance, true));
				}
			}
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
	// VirtualFree(ItemEntry);
	return Ret;
}

bool AFortInventory::RemoveItem(const FGuid& ItemGuid, bool* bShouldUpdate, int Count, bool bForceRemoval)
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

	if (Count < 0)
	{
		Count = 0;
		bForceRemoval = true;
	}

	auto NewCount = ReplicatedEntry->GetCount() - Count;

	auto& ItemInstances = GetItemList().GetItemInstances();
	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	bool bOverrideChangeStackSize = false;

	if (ItemDefinition->ShouldPersistWhenFinalStackEmpty() && !bForceRemoval)
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

	if (NewCount > 0 || bOverrideChangeStackSize)
	{
		ItemInstance->GetItemEntry()->GetCount() = NewCount;
		ReplicatedEntry->GetCount() = NewCount;

		GetItemList().MarkItemDirty(ItemInstance->GetItemEntry());
		GetItemList().MarkItemDirty(ReplicatedEntry);

		return true;
	}

	if (NewCount < 0) // Hm
		return false;

	static auto FortItemEntryStruct = FindObject<UStruct>(L"/Script/FortniteGame.FortItemEntry");
	static auto FortItemEntrySize = FortItemEntryStruct->GetPropertiesSize();

	auto FortPlayerController = Cast<AFortPlayerController>(GetOwner());

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		if (ItemInstances.at(i)->GetItemEntry()->GetItemGuid() == ItemGuid)
		{
			bool AreGadgetsEnabled = Addresses::ApplyGadgetData && Addresses::RemoveGadgetData && Globals::bEnableAGIDs;

			if (AreGadgetsEnabled)
			{
				if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(ItemDefinition))
				{
					GadgetItemDefinition->UnequipGadgetData(FortPlayerController, ItemInstances.at(i));
				}
			}

			ItemInstance->GetItemEntry()->GetStateValues().FreeReal();
			ItemInstances.Remove(i);
			break;
		}
	}

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (ReplicatedEntries.at(i, FortItemEntrySize).GetItemGuid() == ItemGuid)
		{
			ReplicatedEntries.at(i, FortItemEntrySize).GetStateValues().FreeReal();
			ReplicatedEntries.Remove(i, FortItemEntrySize);
			break;
		}
	}

	if (FortPlayerController && Engine_Version < 420)
	{
		static auto QuickBarsOffset = FortPlayerController->GetOffset("QuickBars", false);
		auto QuickBars = FortPlayerController->Get<AFortQuickBars*>(QuickBarsOffset);

		if (QuickBars)
		{
			auto SlotIndex = QuickBars->GetSlotIndex(ItemGuid);

			if (SlotIndex != -1)
			{
				QuickBars->ServerRemoveItemInternal(ItemGuid, false, true);
				QuickBars->EmptySlot(IsPrimaryQuickbar(ItemDefinition) ? EFortQuickBars::Primary : EFortQuickBars::Secondary, SlotIndex);
			}
		}
	}

	// todo remove from weaponlist

	if (bShouldUpdate)
		*bShouldUpdate = true;

	return true;
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
	static auto FortWeaponMeleeItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortWeaponMeleeItemDefinition");

	auto& ItemInstances = GetItemList().GetItemInstances();

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		auto ItemInstance = ItemInstances.At(i);

		if (ItemInstance->GetItemEntry()->GetItemDefinition()->IsA(FortWeaponMeleeItemDefinitionClass))
			return ItemInstance;
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
	static auto FortItemEntryStruct = FindObject<UStruct>(L"/Script/FortniteGame.FortItemEntry");
	static auto FortItemEntrySize = FortItemEntryStruct->GetPropertiesSize();

	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		auto& ReplicatedEntry = ReplicatedEntries.At(i, FortItemEntrySize);

		if (ReplicatedEntry.GetItemGuid() == Guid)
			return &ReplicatedEntry;
	}

	return nullptr;
}