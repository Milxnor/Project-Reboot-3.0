#pragma once

#include "NetSerialization.h"
#include "Class.h"

#include "reboot.h"

enum class EFortItemEntryState : uint8_t // idk if this changes
{
	NoneState = 0,
	NewItemCount = 1,
	ShouldShowItemToast = 2,
	DurabilityInitialized = 3,
	DoNotShowSpawnParticles = 4,
	FromRecoveredBackpack = 5,
	FromGift = 6,
	PendingUpgradeCriteriaProgress = 7,
	OwnerBuildingHandle = 8,
	FromDroppedPickup = 9,
	JustCrafted = 10,
	CraftAndSlotTarget = 11,
	GenericAttributeValueSet = 12,
	PickupInstigatorHandle = 13,
	CreativeUserPrefabHasContent = 14,
	EFortItemEntryState_MAX = 15
};

struct FFortItemEntryStateValue
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.FortItemEntryStateValue");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	int& GetIntValue()
	{
		static auto IntValueOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntryStateValue", "IntValue");
		return *(int*)(__int64(this) + IntValueOffset);
	}

	EFortItemEntryState& GetStateType()
	{
		static auto StateTypeOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntryStateValue", "StateType");
		return *(EFortItemEntryState*)(__int64(this) + StateTypeOffset);
	}

	FName& GetNameValue()
	{
		static auto NameValueOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntryStateValue", "NameValue");
		return *(FName*)(__int64(this) + NameValueOffset);
	}
};

struct FFortItemEntry : FFastArraySerializerItem
{
	FGuid& GetItemGuid()
	{
		static auto ItemGuidOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "ItemGuid");
		return *(FGuid*)(__int64(this) + ItemGuidOffset);
	}

	class UFortItemDefinition*& GetItemDefinition()
	{
		static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "ItemDefinition");
		return *(class UFortItemDefinition**)(__int64(this) + ItemDefinitionOffset);
	}

	int& GetCount()
	{
		static auto CountOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "Count");
		return *(int*)(__int64(this) + CountOffset);
	}

	TArray<FFortItemEntryStateValue>& GetStateValues()
	{
		static auto StateValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "StateValues");
		return *(TArray<FFortItemEntryStateValue>*)(__int64(this) + StateValuesOffset);
	}

	int& GetLoadedAmmo()
	{
		static auto LoadedAmmoOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "LoadedAmmo");
		return *(int*)(__int64(this) + LoadedAmmoOffset);
	}

	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.FortItemEntry");
		return Struct;
	}

	static int GetStructSize()
	{
		static auto StructSize = GetStruct()->GetPropertiesSize();
		return StructSize;
	}

	static FFortItemEntry* MakeItemEntry(UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0)
	{
		auto Entry = Alloc<FFortItemEntry>(GetStructSize());

		if (!Entry)
			return nullptr;

		Entry->GetItemDefinition() = ItemDefinition;
		Entry->GetCount() = Count;
		Entry->GetLoadedAmmo() = LoadedAmmo;

		return Entry;
	}
};

class UFortItem : public UObject
{
public:
	FFortItemEntry* GetItemEntry()
	{
		static auto ItemEntryOffset = this->GetOffset("ItemEntry");
		return &Get<FFortItemEntry>(ItemEntryOffset);
	}

	void SetOwningControllerForTemporaryItem(UObject* Controller);
};