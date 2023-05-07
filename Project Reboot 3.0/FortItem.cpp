#include "FortItem.h"

#include "FortWeaponItemDefinition.h"

FFortItemEntry* FFortItemEntry::MakeItemEntry(UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, float Durability)
{
	auto Entry = // (FFortItemEntry*)FMemory::Realloc(0, GetStructSize(), 0); 
		Alloc<FFortItemEntry>(GetStructSize());

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