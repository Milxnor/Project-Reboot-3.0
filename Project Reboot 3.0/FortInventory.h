#pragma once

#include "Actor.h"
#include "Class.h"

#include "NetSerialization.h"
#include "FortItem.h"
#include "FortItemDefinition.h"

#include "reboot.h"


static bool IsPrimaryQuickbar(UFortItemDefinition* ItemDefinition)
{
	/* if (ItemDefinition->IsA(UFortDecoItemDefinition::StaticClass()))
	{
		if (ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
			return false;
		else
			return true;
	}
	else if (ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()))
		return true; */

	static auto FortWeaponMeleeItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortWeaponMeleeItemDefinition");
	static auto FortEditToolItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortEditToolItemDefinition");
	static auto FortBuildingItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortBuildingItemDefinition");
	static auto FortAmmoItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortAmmoItemDefinition");
	static auto FortResourceItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortResourceItemDefinition");
	static auto FortTrapItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.FortTrapItemDefinition");

	if (!ItemDefinition->IsA(FortWeaponMeleeItemDefinitionClass) && !ItemDefinition->IsA(FortEditToolItemDefinitionClass) &&
		!ItemDefinition->IsA(FortBuildingItemDefinitionClass) && !ItemDefinition->IsA(FortAmmoItemDefinitionClass)
		&& !ItemDefinition->IsA(FortResourceItemDefinitionClass) && !ItemDefinition->IsA(FortTrapItemDefinitionClass))
		return true;

	return false;
}

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

	TArray<FFortItemEntry>& GetReplicatedEntries()
	{
		static auto ReplicatedEntriesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemList", "ReplicatedEntries");
		return *(TArray<FFortItemEntry>*)(__int64(this) + ReplicatedEntriesOffset);
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

	FORCENOINLINE void Update(bool bMarkArrayDirty = true)
	{
		HandleInventoryLocalUpdate();

		if (bMarkArrayDirty)
		{
			GetItemList().MarkArrayDirty();
		}
	}

	std::pair<std::vector<UFortItem*>, std::vector<UFortItem*>> AddItem(UFortItemDefinition* ItemDefinition, bool* bShouldUpdate, int Count = 1, int LoadedAmmo = -1, bool bShouldAddToStateValues = false);
	bool RemoveItem(const FGuid& ItemGuid, bool* bShouldUpdate, int Count);
	void ModifyCount(UFortItem* ItemInstance, int New, bool bRemove = false, std::pair<FFortItemEntry*, FFortItemEntry*>* outEntries = nullptr, bool bUpdate = true);
	
	UFortItem* GetPickaxeInstance();
	UFortItem* FindItemInstance(UFortItemDefinition* ItemDefinition);

	UFortItem* FindItemInstance(const FGuid& Guid);
	FFortItemEntry* FindReplicatedEntry(const FGuid& Guid);
};