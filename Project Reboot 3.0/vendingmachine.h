#pragma once

#include "reboot.h"
#include "BuildingGameplayActor.h"
#include "GameplayStatics.h"
#include "FortLootPackage.h"

using ABuildingItemCollectorActor = ABuildingGameplayActor;

struct FCollectorUnitInfo
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.CollectorUnitInfo");
		return Struct;
	}

	static int GetPropertiesSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	TArray<FFortItemEntry>* GetOutputItemEntry()
	{
		static auto OutputItemEntryOffset = FindOffsetStruct("/Script/FortniteGame.CollectorUnitInfo", "OutputItemEntry");
		return (TArray<FFortItemEntry>*)(__int64(this) + OutputItemEntryOffset);
	}

	UFortWorldItemDefinition*& GetOutputItem()
	{
		static auto OutputItemOffset = FindOffsetStruct("/Script/FortniteGame.CollectorUnitInfo", "OutputItem");
		return *(UFortWorldItemDefinition**)(__int64(this) + OutputItemOffset);
	}
};

static inline void FillItemCollector(ABuildingItemCollectorActor* ItemCollector, FName& LootTierGroup, bool bUseInstanceLootValueOverrides, int recursive = 0)
{
	if (recursive >= 10)
		return;

	static auto ItemCollectionsOffset = ItemCollector->GetOffset("ItemCollections");
	auto& ItemCollections = ItemCollector->Get<TArray<FCollectorUnitInfo>>(ItemCollectionsOffset);

	uint8_t RarityToUse = -1;

	for (int ItemCollectorIt = 0; ItemCollectorIt < ItemCollections.Num(); ItemCollectorIt++)
	{
		auto ItemCollection = ItemCollections.AtPtr(ItemCollectorIt, FCollectorUnitInfo::GetPropertiesSize());

		if (ItemCollection->GetOutputItemEntry()->Num() > 0)
		{
			ItemCollection->GetOutputItemEntry()->Free();
			ItemCollection->GetOutputItem() = nullptr;
		}

		constexpr bool bPrint = false;

		std::vector<LootDrop> LootDrops = PickLootDrops(LootTierGroup, bPrint);

		int tries = 0;

		while (LootDrops.size() == 0)
		{
			tries++;
			LootDrops = PickLootDrops(LootTierGroup, bPrint);

			if (tries >= 10)
				break;
		}

		if (LootDrops.size() == 0)
			continue;


		for (int LootDropIt = 0; LootDropIt < LootDrops.size(); LootDropIt++)
		{
			auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootDrops[LootDropIt].ItemDefinition);

			if (WorldItemDefinition && IsPrimaryQuickbar(WorldItemDefinition)) // nice
			{
				static auto RarityOffset = WorldItemDefinition->GetOffset("Rarity");

				if (RarityToUse == -1)
					RarityToUse = WorldItemDefinition->Get<uint8_t>(RarityOffset);

				if (WorldItemDefinition->Get<uint8_t>(RarityOffset) == RarityToUse)
				{
					bool bItemAlreadyInCollector = false;

					for (int ItemCollectorIt2 = 0; ItemCollectorIt2 < ItemCollections.Num(); ItemCollectorIt2++)
					{
						auto ItemCollection2 = ItemCollections.AtPtr(ItemCollectorIt2, FCollectorUnitInfo::GetPropertiesSize());

						if (ItemCollection2->GetOutputItem() == WorldItemDefinition)
						{
							bItemAlreadyInCollector = true;
							break;
						}
					}

					if (bItemAlreadyInCollector)
						break;

					ItemCollection->GetOutputItem() = WorldItemDefinition;
				}

				break;
			}
		}

		if (!ItemCollection->GetOutputItem())
		{
			ItemCollectorIt--; // retry
			continue;
		}

		for (int LootDropIt = 0; LootDropIt < LootDrops.size(); LootDropIt++)
		{
			auto ItemEntry = FFortItemEntry::MakeItemEntry(LootDrops[LootDropIt].ItemDefinition, LootDrops[LootDropIt].Count, LootDrops[LootDropIt].LoadedAmmo);
			ItemCollection->GetOutputItemEntry()->Add(*ItemEntry, FFortItemEntry::GetStructSize());
		}
	}

	static auto bUseInstanceLootValueOverridesOffset = ItemCollector->GetOffset("bUseInstanceLootValueOverrides");
	ItemCollector->Get<bool>(bUseInstanceLootValueOverridesOffset) = bUseInstanceLootValueOverrides;

	static auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");

	if (ItemCollector->IsA(VendingMachineClass))
	{
		static auto OverrideVendingMachineRarityOffset = ItemCollector->GetOffset("OverrideVendingMachineRarity");
		ItemCollector->Get<uint8_t>(OverrideVendingMachineRarityOffset) = RarityToUse;
	}
}

static inline void FillVendingMachines()
{
	static auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
	auto AllVendingMachines = UGameplayStatics::GetAllActorsOfClass(GetWorld(), VendingMachineClass);

	auto OverrideLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaVending"); // ItemCollector->GetLootTierGroupOverride();

	for (int i = 0; i < AllVendingMachines.Num(); i++)
	{
		auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines.at(i);

		if (!VendingMachine)
			continue;

		FillItemCollector(VendingMachine, OverrideLootTierGroup, true);
	}

	AllVendingMachines.Free();
}