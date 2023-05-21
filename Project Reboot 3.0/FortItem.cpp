#include "FortItem.h"

#include "FortWeaponItemDefinition.h"
#include "AbilitySystemComponent.h"

void FFortItemEntry::SetStateValue(EFortItemEntryState StateType, int IntValue)
{
	for (int i = 0; i < GetStateValues().Num(); i++)
	{
		if (GetStateValues().at(i).GetStateType() == StateType)
		{
			GetStateValues().at(i).GetIntValue() = IntValue;
			return;
		}
	}

	auto idx = GetStateValues().AddUninitialized2(FFortItemEntryStateValue::GetStructSize()); // AddUninitialized?

	GetStateValues().AtPtr(idx, FFortItemEntryStateValue::GetStructSize())->GetIntValue() = IntValue;
	GetStateValues().AtPtr(idx, FFortItemEntryStateValue::GetStructSize())->GetStateType() = StateType;
	GetStateValues().AtPtr(idx, FFortItemEntryStateValue::GetStructSize())->GetNameValue() = FName(0);

	// idk some parentinventory stuff here

	// ItemEntry->bIsDirty = true;
}

FFortItemEntry* FFortItemEntry::MakeItemEntry(UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, float Durability, int Level)
{
	auto Entry = Alloc<FFortItemEntry>(GetStructSize(), bUseFMemoryRealloc);

	if (!Entry)
		return nullptr;

	if (LoadedAmmo == -1)
	{
		if (auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDefinition)) // bPreventDefaultPreload ?
			LoadedAmmo = WeaponDef->GetClipSize();
		else
			LoadedAmmo = 0;
	}

	Entry->MostRecentArrayReplicationKey = -1; // idk if we need to set this
	Entry->ReplicationID = -1;
	Entry->ReplicationKey = -1;

	Entry->GetItemDefinition() = ItemDefinition;
	Entry->GetCount() = Count;
	Entry->GetLoadedAmmo() = LoadedAmmo;
	Entry->GetDurability() = Durability;
	Entry->GetGameplayAbilitySpecHandle() = FGameplayAbilitySpecHandle(-1);
	Entry->GetParentInventory().ObjectIndex = -1;
	Entry->GetLevel() = Level;
	// We want to add StateValues.Add(DurabilityInitialized); orwnatefc erwgearf yk
	// CoCreateGuid((GUID*)&Entry->GetItemGuid());
	// Entry->DoesUpdateStatsOnCollection() = true; // I think fortnite does this?

	return Entry;
}

void UFortItem::SetOwningControllerForTemporaryItem(UObject* Controller)
{
	static auto SOCFTIFn = FindObject<UFunction>(L"/Script/FortniteGame.FortItem.SetOwningControllerForTemporaryItem");
	this->ProcessEvent(SOCFTIFn, &Controller);
}