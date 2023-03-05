#include "FortInventory.h"
#include "FortPlayerController.h"

UFortItem* CreateItemInstance(AFortPlayerController* PlayerController, UFortItemDefinition* ItemDefinition, int Count)
{
	UFortItem* NewItemInstance = ItemDefinition->CreateTemporaryItemInstanceBP(Count);

	if (NewItemInstance)
		NewItemInstance->SetOwningControllerForTemporaryItem(PlayerController);

	return NewItemInstance;
}

std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AFortInventory::AddItem(UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count)
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