#pragma once

#include "NetSerialization.h"
#include "Class.h"

#include "reboot.h"

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