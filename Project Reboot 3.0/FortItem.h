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

	UObject*& GetItemDefinition()
	{
		static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortItemEntry", "ItemDefinition");
		return *(UObject**)(__int64(this) + ItemDefinitionOffset);
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