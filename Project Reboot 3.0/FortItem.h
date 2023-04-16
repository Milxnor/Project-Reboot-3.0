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

	bool& GetIsReplicatedCopy()
	{
		static auto bIsReplicatedCopyOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "bIsReplicatedCopy");
		return *(bool*)(__int64(this) + bIsReplicatedCopyOffset);
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

	void CopyFromAnotherItemEntry(FFortItemEntry* OtherItemEntry, bool bCopyGuid = false)
	{
		auto OldGuid = this->GetItemGuid();

		if (false)
		{
			CopyStruct(this, OtherItemEntry, FFortItemEntry::GetStructSize(), FFortItemEntry::GetStruct());
		}
		else
		{
			this->GetItemDefinition() = OtherItemEntry->GetItemDefinition();
			this->GetCount() = OtherItemEntry->GetCount();
			this->GetLoadedAmmo() = OtherItemEntry->GetLoadedAmmo();
		}

		this->GetItemGuid() = OldGuid;
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
		auto Entry = // (FFortItemEntry*)FMemory::Realloc(0, GetStructSize(), 0); 
			Alloc<FFortItemEntry>(GetStructSize());

		if (!Entry)
			return nullptr;

		Entry->MostRecentArrayReplicationKey = -1;
		Entry->ReplicationID = -1;
		Entry->ReplicationKey = -1;

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
		return GetPtr<FFortItemEntry>(ItemEntryOffset);
	}

	void SetOwningControllerForTemporaryItem(UObject* Controller);
};