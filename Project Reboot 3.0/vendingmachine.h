#pragma once

#include "reboot.h"
#include "BuildingGameplayActor.h"
#include "GameplayStatics.h"
#include "FortLootPackage.h"
#include "GameplayAbilityTypes.h"

using ABuildingItemCollectorActor = ABuildingGameplayActor;

struct FCollectorUnitInfo
{
	static std::string GetStructName()
	{
		static std::string StructName = FindObject<UStruct>("/Script/FortniteGame.CollectorUnitInfo") ? "/Script/FortniteGame.CollectorUnitInfo" : "/Script/FortniteGame.ColletorUnitInfo"; // nice one fortnite
		return StructName;
	}

	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(GetStructName());
		return Struct;
	}

	static int GetPropertiesSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	FScalableFloat* GetInputCount()
	{
		static auto InputCountOffset = FindOffsetStruct(GetStructName(), "InputCount");
		return (FScalableFloat*)(__int64(this) + InputCountOffset);
	}

	TArray<FFortItemEntry>* GetOutputItemEntry()
	{
		static auto OutputItemEntryOffset = FindOffsetStruct(GetStructName(), "OutputItemEntry");
		return (TArray<FFortItemEntry>*)(__int64(this) + OutputItemEntryOffset);
	}

	UFortWorldItemDefinition*& GetInputItem()
	{
		static auto InputItemOffset = FindOffsetStruct(GetStructName(), "InputItem");
		return *(UFortWorldItemDefinition**)(__int64(this) + InputItemOffset);
	}

	UFortWorldItemDefinition*& GetOutputItem()
	{
		static auto OutputItemOffset = FindOffsetStruct(GetStructName(), "OutputItem");
		return *(UFortWorldItemDefinition**)(__int64(this) + OutputItemOffset);
	}
};

static inline void FillItemCollector(ABuildingItemCollectorActor* ItemCollector, FName& LootTierGroup, bool bUseInstanceLootValueOverrides, bool bEnsureRarity = false, int recursive = 0)
{
	if (recursive >= 10)
		return;

	auto GameModeAthena = (AFortGameModeAthena*)GetWorld()->GetGameMode();
	auto GameState = Cast<AFortGameStateAthena>(GameModeAthena->GetGameState());

	static auto ItemCollectionsOffset = ItemCollector->GetOffset("ItemCollections");
	auto& ItemCollections = ItemCollector->Get<TArray<FCollectorUnitInfo>>(ItemCollectionsOffset);

	auto CurrentPlaylist = GameState->GetCurrentPlaylist();
	UCurveTable* FortGameData = nullptr;

	static auto GameDataOffset = CurrentPlaylist->GetOffset("GameData");
	FortGameData = CurrentPlaylist ? CurrentPlaylist->Get<TSoftObjectPtr<UCurveTable>>(GameDataOffset).Get() : nullptr;

	if (!FortGameData)
		FortGameData = FindObject<UCurveTable>("/Game/Athena/Balance/AthenaGameData.AthenaGameData"); // uhm so theres one without athena and on newer versions that has it so idk

	auto WoodName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Wood");
	auto StoneName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Stone");
	auto MetalName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Metal");

	static auto StoneItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	static auto MetalItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

	uint8_t RarityToUse = 69;

	// TODO: Pull prices from datatables.

	bool bLowerPrices = Fortnite_Version >= 5.20;

	static int CommonPrice = bLowerPrices ? 75 : 100;
	static int UncommonPrice = bLowerPrices ? 150 : 200;
	static int RarePrice = bLowerPrices ? 225 : 300;
	static int EpicPrice = bLowerPrices ? 300 : 400;
	static int LegendaryPrice = bLowerPrices ? 375 : 500;

	if (Fortnite_Version >= 8.10)
	{
		CommonPrice = 0;
		UncommonPrice = 0;
		RarePrice = 0;
		EpicPrice = 0;
		LegendaryPrice = 0;
	}

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

			if (!WorldItemDefinition)
				continue;

			if (!IsPrimaryQuickbar(WorldItemDefinition))
				continue;

			if (bEnsureRarity)
			{
				static auto RarityOffset = WorldItemDefinition->GetOffset("Rarity");

				if (RarityToUse == 69)
					RarityToUse = WorldItemDefinition->Get<uint8_t>(RarityOffset);

				if (WorldItemDefinition->Get<uint8_t>(RarityOffset) != RarityToUse)
					continue;
			}

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

			break;
		}

		if (!ItemCollection->GetOutputItem())
		{
			ItemCollectorIt--; // retry
			continue;
		}

		for (int LootDropIt = 0; LootDropIt < LootDrops.size(); LootDropIt++)
		{
			auto ItemEntry = FFortItemEntry::MakeItemEntry(LootDrops[LootDropIt].ItemDefinition, LootDrops[LootDropIt].Count, LootDrops[LootDropIt].LoadedAmmo);

			if (!ItemEntry)
				continue;

			ItemCollection->GetOutputItemEntry()->AddPtr(ItemEntry, FFortItemEntry::GetStructSize());
		}

		// The reason I set the curve to 0 is because it will force it to return value, probably not how we are supposed to do it but whatever.
		ItemCollection->GetInputCount()->GetCurve().CurveTable = Fortnite_Version < 5 ? nullptr : FortGameData; // scuffed idc
		ItemCollection->GetInputCount()->GetCurve().RowName = Fortnite_Version < 5 ? FName(0) : WoodName; // Scuffed idc 
		ItemCollection->GetInputCount()->GetValue() = RarityToUse == 0 ? CommonPrice 
			: RarityToUse == 1 ? UncommonPrice 
			: RarityToUse == 2 ? RarePrice 
			: RarityToUse == 3 ? EpicPrice
			: RarityToUse == 4 ? LegendaryPrice
			: -1;
	}

	static auto bUseInstanceLootValueOverridesOffset = ItemCollector->GetOffset("bUseInstanceLootValueOverrides", false);

	if (bUseInstanceLootValueOverridesOffset != -1)
		ItemCollector->Get<bool>(bUseInstanceLootValueOverridesOffset) = bUseInstanceLootValueOverrides;

	LOG_INFO(LogDev, "RarityToUse: {}", (int)RarityToUse);

	static auto StartingGoalLevelOffset = ItemCollector->GetOffset("StartingGoalLevel");

	if (StartingGoalLevelOffset != -1)
		ItemCollector->Get<int32>(StartingGoalLevelOffset) = (int)RarityToUse;

	static auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");

	if (ItemCollector->IsA(VendingMachineClass))
	{
		static auto OverrideVendingMachineRarityOffset = ItemCollector->GetOffset("OverrideVendingMachineRarity", false);

		if (OverrideVendingMachineRarityOffset != -1)
			ItemCollector->Get<uint8_t>(OverrideVendingMachineRarityOffset) = RarityToUse;

		static auto OverrideGoalOffset = ItemCollector->GetOffset("OverrideGoal", false);
		
		if (OverrideGoalOffset != -1)
		{
			ItemCollector->Get<int32>(OverrideGoalOffset) = RarityToUse == 0 ? CommonPrice
				: RarityToUse == 1 ? UncommonPrice
				: RarityToUse == 2 ? RarePrice
				: RarityToUse == 3 ? EpicPrice
				: RarityToUse == 4 ? LegendaryPrice
				: -1;
		}
	}
}

static inline void FillVendingMachines()
{
	auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
	auto AllVendingMachines = UGameplayStatics::GetAllActorsOfClass(GetWorld(), VendingMachineClass);

	auto OverrideLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaVending"); // ItemCollector->GetLootTierGroupOverride();

	for (int i = 0; i < AllVendingMachines.Num(); i++)
	{
		auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines.at(i);

		if (!VendingMachine)
			continue;

		FillItemCollector(VendingMachine, OverrideLootTierGroup, true, true);
	}

	AllVendingMachines.Free();
}