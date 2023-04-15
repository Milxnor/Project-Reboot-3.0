#pragma once

#include "FortWorldItemDefinition.h"
#include "FortPlayerController.h"
#include "AttributeSet.h"
#include "SoftObjectPtr.h"

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

	UAttributeSet* GetAttributeSet()
	{
		static auto AttributeSetOffset = this->GetOffset("AttributeSet", false);

		if (AttributeSetOffset == -1)
			return nullptr;

		auto& AttributeSetSoft = this->Get<TSoftObjectPtr<UAttributeSet>>(AttributeSetOffset);

		static auto AttributeClass = FindObject<UClass>("/Script/GameplayAbilities.AttributeSet");
	    return AttributeSetSoft.Get(AttributeClass, true);
	}

	void UnequipGadgetData(AFortPlayerController* PlayerController, UFortItem* Item);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortGadgetItemDefinition");
		return Class;
	}
};