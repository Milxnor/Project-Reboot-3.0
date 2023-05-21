#pragma once

#include <Windows.h>

#include "NetSerialization.h"
#include "Class.h"
#include "GameplayAbilitySpec.h"

#include "reboot.h"

constexpr inline bool bUseFMemoryRealloc = false; // This is for allocating our own Item entries, I don't know why this doesn't work

enum class EFortItemEntryState : uint8_t // this changes but its fineee
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

#define MAX_DURABILITY 0x3F800000

struct FFortItemEntryStateValue
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.FortItemEntryStateValue");
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

	bool& DoesUpdateStatsOnCollection()
	{
		// added like s8+ or somethingf idsk it was on 10.40 but not 7.40
		static auto bUpdateStatsOnCollectionOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "bUpdateStatsOnCollection");
		return *(bool*)(__int64(this) + bUpdateStatsOnCollectionOffset);
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

	int& GetLevel()
	{
		static auto LevelOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "Level");
		return *(int*)(__int64(this) + LevelOffset);
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

	float& GetDurability()
	{
		static auto DurabilityOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "Durability");
		return *(float*)(__int64(this) + DurabilityOffset);
	}

	FGameplayAbilitySpecHandle& GetGameplayAbilitySpecHandle()
	{
		static auto GameplayAbilitySpecHandleOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GameplayAbilitySpecHandle");
		return *(FGameplayAbilitySpecHandle*)(__int64(this) + GameplayAbilitySpecHandleOffset);
	}

	TArray<float>& GetGenericAttributeValues()
	{
		static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues");
		return *(TArray<float>*)(__int64(this) + GenericAttributeValuesOffset);
	}

	TWeakObjectPtr<class AFortInventory>& GetParentInventory()
	{
		static auto ParentInventoryOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "ParentInventory");
		return *(TWeakObjectPtr<class AFortInventory>*)(__int64(this) + ParentInventoryOffset);
	}

	void CopyFromAnotherItemEntry(FFortItemEntry* OtherItemEntry, bool bCopyGuid = false)
	{
		// We can use FortItemEntryStruct->CopyScriptStruct

		FGuid OldGuid = this->GetItemGuid();

		if (false)
		{
			CopyStruct(this, OtherItemEntry, FFortItemEntry::GetStructSize(), FFortItemEntry::GetStruct());
		}
		else
		{
			this->GetItemDefinition() = OtherItemEntry->GetItemDefinition();
			this->GetCount() = OtherItemEntry->GetCount();
			this->GetLoadedAmmo() = OtherItemEntry->GetLoadedAmmo();
			this->GetItemGuid() = OtherItemEntry->GetItemGuid();
			this->GetLevel() = OtherItemEntry->GetLevel();
		}

		if (!bCopyGuid)
			this->GetItemGuid() = OldGuid;

		static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues", false);

		if (GenericAttributeValuesOffset != -1)
		{			
			// this->GetGenericAttributeValues().CopyFromArray(OtherItemEntry->GetGenericAttributeValues());
		}

		// this->GetStateValues().CopyFromArray(OtherItemEntry->GetStateValues(), FFortItemEntryStateValue::GetStructSize()); // broooooooooooooooooooo

		// should we do this?

		this->MostRecentArrayReplicationKey = -1;
		this->ReplicationID = -1;
		this->ReplicationKey = -1;
	}

	void SetStateValue(EFortItemEntryState StateType, int IntValue);

	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.FortItemEntry");
		return Struct;
	}

	static int GetStructSize()
	{
		static auto StructSize = GetStruct()->GetPropertiesSize();
		return StructSize;
	}

	static FFortItemEntry* MakeItemEntry(UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0, float Durability = MAX_DURABILITY, int Level = 0);

	// We need to find a better way for below... Especially since we can't do either method for season 5 or 6.

	static void FreeItemEntry(FFortItemEntry* Entry)
	{
		if (Addresses::FreeEntry)
		{
			static __int64 (*FreeEntryOriginal)(__int64 Entry) = decltype(FreeEntryOriginal)(Addresses::FreeEntry);
			FreeEntryOriginal(__int64(Entry));
		}
		else
		{
			static auto GenericAttributeValuesOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "GenericAttributeValues", false);

			if (GenericAttributeValuesOffset != -1)
			{
				Entry->GetGenericAttributeValues().FreeGood();
			}

			Entry->GetStateValues().FreeGood();
		}

		// RtlZeroMemory(Entry, FFortItemEntry::GetStructSize());
	}

	static void FreeArrayOfEntries(TArray<FFortItemEntry>& tarray)
	{
		if (Addresses::FreeArrayOfEntries)
		{
			static __int64 (*FreeArrayOfEntriesOriginal)(TArray<FFortItemEntry>& a1) = decltype(FreeArrayOfEntriesOriginal)(Addresses::FreeArrayOfEntries);
			FreeArrayOfEntriesOriginal(tarray);
		}
		else
		{
			if (Addresses::FreeEntry)
			{
				for (int i = 0; i < tarray.size(); i++)
				{
					FreeItemEntry(tarray.AtPtr(i));
				}
			}
			else
			{
				tarray.Free(); // does nothing
			}
		}
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