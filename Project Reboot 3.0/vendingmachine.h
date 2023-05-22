#pragma once

#include "reboot.h"
#include "BuildingGameplayActor.h"
#include "GameplayStatics.h"
#include "FortLootPackage.h"
#include "GameplayAbilityTypes.h"
#include "KismetMathLibrary.h"

using ABuildingItemCollectorActor = ABuildingGameplayActor;

struct FCollectorUnitInfo
{
	static std::string GetStructName()
	{
		static std::string StructName = FindObject<UStruct>(L"/Script/FortniteGame.CollectorUnitInfo") ? "/Script/FortniteGame.CollectorUnitInfo" : "/Script/FortniteGame.ColletorUnitInfo"; // nice one fortnite
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

static inline UCurveTable* GetGameData()
{
	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	UCurveTable* FortGameData = nullptr;

	auto CurrentPlaylist = GameState->GetCurrentPlaylist();

	if (CurrentPlaylist)
	{
		static auto GameDataOffset = CurrentPlaylist->GetOffset("GameData");
		FortGameData = CurrentPlaylist ? CurrentPlaylist->GetPtr<TSoftObjectPtr<UCurveTable>>(GameDataOffset)->Get() : nullptr;
	}

	if (!FortGameData)
		FortGameData = FindObject<UCurveTable>(L"/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData"); // uhm so theres one without athena and on newer versions that has it so idk // after i wrote this cokmment idk what i meant

	return FortGameData;
}

static inline void FillItemCollector(ABuildingItemCollectorActor* ItemCollector, FName& LootTierGroup, bool bUseInstanceLootValueOverrides, int LootTier, int recursive = 0)
{
	if (recursive >= 10)
		return;

	auto GameModeAthena = (AFortGameModeAthena*)GetWorld()->GetGameMode();
	auto GameState = Cast<AFortGameStateAthena>(GameModeAthena->GetGameState());

	static auto ItemCollectionsOffset = ItemCollector->GetOffset("ItemCollections");
	auto& ItemCollections = ItemCollector->Get<TArray<FCollectorUnitInfo>>(ItemCollectionsOffset);

	UCurveTable* FortGameData = GetGameData();

	auto WoodName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Wood");
	auto StoneName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Stone");
	auto MetalName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.Cost.Metal");

	static auto StoneItemData = FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	static auto MetalItemData = FindObject<UFortResourceItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

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

	int itemCollectorRecursive = 0;

	for (int ItemCollectorIt = 0; ItemCollectorIt < ItemCollections.Num(); ItemCollectorIt++)
	{
		if (itemCollectorRecursive > 3)
		{
			itemCollectorRecursive = 0;
			continue;
		}

		auto ItemCollection = ItemCollections.AtPtr(ItemCollectorIt, FCollectorUnitInfo::GetPropertiesSize());

		if (ItemCollection->GetOutputItemEntry()->Num() > 0)
		{
			ItemCollection->GetOutputItemEntry()->Free();
			ItemCollection->GetOutputItem() = nullptr;
		}

		constexpr bool bPrint = false;

		std::vector<LootDrop> LootDrops = PickLootDrops(LootTierGroup, GameState->GetWorldLevel(), LootTier, bPrint);

		if (LootDrops.size() == 0)
		{
			// LOG_WARN(LogGame, "Failed to find LootDrops for vending machine loot tier: {}", LootTier);
			ItemCollectorIt--; // retry (?)
			itemCollectorRecursive++;
			continue;
		}

		for (int LootDropIt = 0; LootDropIt < LootDrops.size(); LootDropIt++)
		{
			UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(LootDrops[LootDropIt]->GetItemDefinition());

			if (!WorldItemDefinition)
				continue;

			if (!IsPrimaryQuickbar(WorldItemDefinition)) // i dont think we need this check
				continue;

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
			itemCollectorRecursive++;
			continue;
		}

		for (int LootDropIt = 0; LootDropIt < LootDrops.size(); LootDropIt++)
		{
			auto ItemEntry = LootDrops[LootDropIt].ItemEntry; // FFortItemEntry::MakeItemEntry(LootDrops[LootDropIt]->GetItemDefinition(), LootDrops[LootDropIt]->GetCount(), LootDrops[LootDropIt]->GetLoadedAmmo(), MAX_DURABILITY, LootDrops[LootDropIt]->GetLevel());

			if (!ItemEntry)
				continue;

			ItemCollection->GetOutputItemEntry()->AddPtr(ItemEntry, FFortItemEntry::GetStructSize());
		}

		// The reason I set the curve to 0 is because it will force it to return value, probably not how we are supposed to do it but whatever.

		bool bShouldBeNullTable = true; // Fortnite_Version < 5

		ItemCollection->GetInputCount()->GetCurve().CurveTable = bShouldBeNullTable ? nullptr : FortGameData; // scuffed idc
		ItemCollection->GetInputCount()->GetCurve().RowName = bShouldBeNullTable ? FName(0) : WoodName; // Scuffed idc 
		ItemCollection->GetInputCount()->GetValue() = LootTier == 0 ? CommonPrice 
			: LootTier == 1 ? UncommonPrice
			: LootTier == 2 ? RarePrice
			: LootTier == 3 ? EpicPrice
			: LootTier == 4 ? LegendaryPrice
			: -1;
	}

	static auto bUseInstanceLootValueOverridesOffset = ItemCollector->GetOffset("bUseInstanceLootValueOverrides", false);

	if (bUseInstanceLootValueOverridesOffset != -1)
		ItemCollector->Get<bool>(bUseInstanceLootValueOverridesOffset) = bUseInstanceLootValueOverrides;

	// LOG_INFO(LogDev, "LootTier: {}", LootTier);

	static auto StartingGoalLevelOffset = ItemCollector->GetOffset("StartingGoalLevel");

	if (StartingGoalLevelOffset != -1)
		ItemCollector->Get<int32>(StartingGoalLevelOffset) = LootTier;

	static auto VendingMachineClass = FindObject<UClass>(L"/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");

	if (ItemCollector->IsA(VendingMachineClass))
	{
		static auto OverrideVendingMachineRarityOffset = ItemCollector->GetOffset("OverrideVendingMachineRarity", false);

		if (OverrideVendingMachineRarityOffset != -1)
			ItemCollector->Get<uint8_t>(OverrideVendingMachineRarityOffset) = LootTier;

		static auto OverrideGoalOffset = ItemCollector->GetOffset("OverrideGoal", false);
		
		if (OverrideGoalOffset != -1)
		{
			ItemCollector->Get<int32>(OverrideGoalOffset) = LootTier == 0 ? CommonPrice
				: LootTier == 1 ? UncommonPrice
				: LootTier == 2 ? RarePrice
				: LootTier == 3 ? EpicPrice
				: LootTier == 4 ? LegendaryPrice
				: -1;
		}
	}
}

static inline void FillVendingMachines()
{
	auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
	auto AllVendingMachines = UGameplayStatics::GetAllActorsOfClass(GetWorld(), VendingMachineClass);

	auto OverrideLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaVending"); // ItemCollector->GetLootTierGroupOverride();

	std::map<int, float> ThingAndWeights; // Bro IDK WHat to name it!

	auto RarityWeightsName = UKismetStringLibrary::Conv_StringToName(L"Default.VendingMachine.RarityWeights");

	auto FortGameData = GetGameData();

	float WeightSum = 0;

	for (int i = 0; i < 6; i++)
	{
		auto Weight = UDataTableFunctionLibrary::EvaluateCurveTableRow(FortGameData, RarityWeightsName, i);
		ThingAndWeights[i] = Weight;
		WeightSum += Weight;
	}

	for (int i = 0; i < ThingAndWeights.size(); i++)
	{
		// LOG_INFO(LogDev, "[{}] bruh: {}", i, ThingAndWeights.at(i));
	}

	std::map<int, int> PickedRarities;

	for (int i = 0; i < AllVendingMachines.Num(); i++)
	{
		auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines.at(i);

		if (!VendingMachine)
			continue;

		auto randomFloatGenerator = [&](float Max) -> float { return UKismetMathLibrary::RandomFloatInRange(0, Max); };

		int Out;
		PickWeightedElement<int, float>(ThingAndWeights, [&](float Weight) -> float { return Weight; }, randomFloatGenerator, WeightSum, false, 1, &Out, false, true);

		PickedRarities[Out]++;

		if (Out == 0)
		{
			VendingMachine->K2_DestroyActor();
			continue;
		}

		/*
		
		LOOT LEVELS:

		0 - Common
		1 - Uncommon
		2 - Rare
		3 - Epic
		4 - Legendary

		*/

		FillItemCollector(VendingMachine, OverrideLootTierGroup, true, Out - 1);
	}

	auto AllVendingMachinesNum = AllVendingMachines.Num();

	AllVendingMachines.Free();

	bool bPrintDebug = true;

	if (bPrintDebug)
	{
		LOG_INFO(LogGame, "Destroyed {}/{} vending machines.", PickedRarities[0], AllVendingMachinesNum);
		LOG_INFO(LogGame, "Filled {}/{} vending machines with common items.", PickedRarities[1], AllVendingMachinesNum);
		LOG_INFO(LogGame, "Filled {}/{} vending machines with uncommon items.", PickedRarities[2], AllVendingMachinesNum);
		LOG_INFO(LogGame, "Filled {}/{} vending machines with rare items.", PickedRarities[3], AllVendingMachinesNum);
		LOG_INFO(LogGame, "Filled {}/{} vending machines with epic items.", PickedRarities[4], AllVendingMachinesNum);
		LOG_INFO(LogGame, "Filled {}/{} vending machines with legendary items.", PickedRarities[5], AllVendingMachinesNum);
	}
	else
	{
		LOG_INFO(LogGame, "Filled {} vending machines!", AllVendingMachinesNum);
	}
}