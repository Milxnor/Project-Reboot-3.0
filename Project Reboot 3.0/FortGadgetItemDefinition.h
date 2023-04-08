#pragma once

#include "FortWorldItemDefinition.h"

class UFortGadgetItemDefinition : public UFortWorldItemDefinition
{
public:

	bool ShouldDropAllItemsOnEquip()
	{
		static auto bDropAllOnEquipOffset = GetOffset("bDropAllOnEquip", false);

		if (bDropAllOnEquipOffset == -1)
			return false;

		static auto bDropAllOnEquipFieldMask = GetFieldMask(GetProperty("bDropAllOnEquip"));
		return ReadBitfieldValue(bDropAllOnEquipOffset, bDropAllOnEquipFieldMask);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortGadgetItemDefinition");
		return Class;
	}
};