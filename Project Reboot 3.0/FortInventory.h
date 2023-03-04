#pragma once

#include "Actor.h"
#include "Class.h"

#include "NetSerialization.h"
#include "FortItem.h"
#include "FortItemDefinition.h"

#include "reboot.h"

enum class EFortInventoryType : unsigned char
{
	World = 0,
	Account = 1,
	Outpost = 2,
	MAX = 3,
};

struct FFortItemList : public FFastArraySerializer
{
	TArray<UFortItem*>& GetItemInstances()
	{
		static auto ItemInstancesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ItemInstances");
		return *(TArray<UFortItem*>*)(__int64(this) + ItemInstancesOffset);
	}

	TArray<FFastArraySerializerItem>& GetReplicatedEntries()
	{
		static auto ReplicatedEntriesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ReplicatedEntries");
		return *(TArray<FFastArraySerializerItem>*)(__int64(this) + ReplicatedEntriesOffset);
	}
};

class AFortInventory : public AActor
{
public:
	FFortItemList& GetItemList()
	{
		static auto InventoryOffset = GetOffset("Inventory");
		return Get<FFortItemList>(InventoryOffset);
	}

	EFortInventoryType& GetInventoryType()
	{
		static auto InventoryOffset = GetOffset("InventoryType");
		return Get<EFortInventoryType>(InventoryOffset);
	}

	void HandleInventoryLocalUpdate()
	{
		static auto HandleInventoryLocalUpdateFn = FindObject<UFunction>(L"/Script/FortniteGame.FortInventory.HandleInventoryLocalUpdate");
		ProcessEvent(HandleInventoryLocalUpdateFn);
	}

	FORCENOINLINE void Update(bool bMarkArrayDirty = false)
	{
		HandleInventoryLocalUpdate();

		if (bMarkArrayDirty)
		{
			GetItemList().MarkArrayDirty();
		}
	}

	std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AddItem(UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count = 1);

	UFortItem* FindItemInstance(const FGuid& Guid);
};