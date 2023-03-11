#include "FortInventory.h"
#include "FortPlayerController.h"

UFortItem* CreateItemInstance(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count)
{
	UFortItem* NewItemInstance = ItemDefinition->CreateTemporaryItemInstanceBP(Count);

	if (NewItemInstance)
		NewItemInstance->SetOwningControllerForTemporaryItem(PlayerController);

	return NewItemInstance;
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AFortInventory::AddItem(UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count, int LoadedAmmo, bool bShouldAddToStateValues)
{
	if (bShouldUpdate)
		*bShouldUpdate = false;

	std::vector<UFortItem*> NewItemInstances;
	std::vector<UFortItem*> ModifiedItemInstances;

	auto PlayerController = Cast<AFortPlayerController>(GetOwner());

	UFortItem* NewItemInstance = CreateItemInstance(PlayerController, ItemDefinition, Count);

	if (NewItemInstance)
	{
		NewItemInstances.push_back(NewItemInstance);

		// NewItemInstance->GetItemEntry()->GetItemDefinition() = ItemDefinition;

		static auto FortItemEntryStruct = FindObject(L"/Script/FortniteGame.FortItemEntry");
		static auto FortItemEntrySize = *(int*)(__int64(FortItemEntryStruct) + Offsets::PropertiesSize);

		// LOG_INFO(LogDev, "FortItemEntryStruct {}", __int64(FortItemEntryStruct));
		// LOG_INFO(LogDev, "FortItemEntrySize {}", __int64(FortItemEntrySize));

		GetItemList().GetItemInstances().Add(NewItemInstance);
		GetItemList().GetReplicatedEntries().Add(*NewItemInstance->GetItemEntry(), FortItemEntrySize);

		if (bShouldUpdate)
			*bShouldUpdate = true;
	}

	return std::make_pair(NewItemInstances, ModifiedItemInstances);
}

bool AFortInventory::RemoveItem(const FGuid& ItemGuid, bool* bShouldUpdate, int Count)
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

	auto NewCount = ReplicatedEntry->GetCount() - Count;

	if (NewCount > 0)
	{
		ItemInstance->GetItemEntry()->GetCount() = NewCount;
		ReplicatedEntry->GetCount() = NewCount;

		GetItemList().MarkItemDirty(ItemInstance->GetItemEntry());
		GetItemList().MarkItemDirty(ReplicatedEntry);

		return true;
	}

	static auto FortItemEntryStruct = FindObject(L"/Script/FortniteGame.FortItemEntry");
	static auto FortItemEntrySize = *(int*)(__int64(FortItemEntryStruct) + Offsets::PropertiesSize);

	auto& ItemInstances = GetItemList().GetItemInstances();
	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	for (int i = 0; i < ItemInstances.Num(); i++)
	{
		if (ItemInstances.at(i)->GetItemEntry()->GetItemGuid() == ItemGuid)
		{
			ItemInstances.Remove(i);
			break;
		}
	}

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		if (ReplicatedEntries.at(i).GetItemGuid() == ItemGuid)
		{
			ReplicatedEntries.Remove(i, FortItemEntrySize);
			break;
		}
	}

	// todo remove from weaponlist

	if (bShouldUpdate)
		*bShouldUpdate = true;

	return true;
}

void AFortInventory::ModifyCount(UFortItem* ItemInstance, int New, bool bRemove, std::pair<FFortItemEntry*, FFortItemEntry*>* outEntries, bool bUpdate)
{
	auto ReplicatedEntry = FindReplicatedEntry(ItemInstance->GetItemEntry()->GetItemGuid());

	if (!ReplicatedEntry)
		return;

	if (!bRemove)
	{
		ItemInstance->GetItemEntry()->GetCount() += New;
		ReplicatedEntry->GetCount() += New;
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
	auto& ReplicatedEntries = GetItemList().GetReplicatedEntries();

	for (int i = 0; i < ReplicatedEntries.Num(); i++)
	{
		auto& ReplicatedEntry = ReplicatedEntries.At(i);

		if (ReplicatedEntry.GetItemGuid() == Guid)
			return &ReplicatedEntry;
	}

	return nullptr;
}