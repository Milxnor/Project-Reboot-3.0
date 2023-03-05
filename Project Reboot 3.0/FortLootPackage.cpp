#include "FortLootPackage.h"

#include "DataTable.h"
#include "KismetMathLibrary.h"

static FFortLootTierData* GetLootTierData(std::vector<FFortLootTierData*>& LootTierData, bool bPrint)
{
    float TotalWeight = 0;

    for (auto Item : LootTierData)
    {
        TotalWeight += Item->GetWeight();
    }

    float RandomNumber = UKismetMathLibrary::RandomFloatInRange(0, TotalWeight); // is -1 needed?

    FFortLootTierData* SelectedItem = nullptr;

    if (bPrint)
    {
        std::cout << std::format("TotalWeight: {}\n", TotalWeight);
    }

    for (auto Item : LootTierData)
    {
        if (bPrint)
        {
            std::cout << std::format("Rand: {} Weight: {}\n", RandomNumber, Item->GetWeight());
        }

        if (RandomNumber <= Item->GetWeight())
        {
            SelectedItem = Item;
            break;
        }

        RandomNumber -= Item->GetWeight();
    }

    if (!SelectedItem)
        return GetLootTierData(LootTierData, bPrint);

    return SelectedItem;
}

static FFortLootPackageData* GetLootPackage(std::vector<FFortLootPackageData*>& LootPackages)
{
    float TotalWeight = 0;

    for (auto Item : LootPackages)
    {
        TotalWeight += Item->GetWeight();
    }

    float RandomNumber = UKismetMathLibrary::RandomFloatInRange(0, TotalWeight); // is -1 needed?

    FFortLootPackageData* SelectedItem = nullptr;

    for (auto Item : LootPackages)
    {
        if (RandomNumber <= Item->GetWeight())
        {
            SelectedItem = Item;
            break;
        }

        RandomNumber -= Item->GetWeight();
    }

    if (!SelectedItem)
        return GetLootPackage(LootPackages);

    return SelectedItem;
}

std::vector<std::pair<UFortItemDefinition*, int>> PickLootDrops(FName TierGroupName, bool bPrint, int recursive)
{
    std::vector<std::pair<UFortItemDefinition*, int>> LootDrops;

    static std::vector<UDataTable*> LTDTables;
    static std::vector<UDataTable*> LPTables;

    static bool bHasFoundTables = false;

    if (!bHasFoundTables)
    {
        bHasFoundTables = true;

        LTDTables.push_back(FindObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
        LPTables.push_back(FindObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
    }

    std::vector<FFortLootTierData*> TierGroupLTDs;

    for (int p = 0; p < LTDTables.size(); p++)
    {
        auto LTD = LTDTables[p];
        auto& LTDRowMap = LTD->GetRowMap();

        auto LTDRowMapNum = LTDRowMap.Pairs.Elements.Num();

        // auto TierGroupNameStr = TierGroupName.ToString();

        for (int i = 0; i < LTDRowMapNum; i++)
        {
            auto& CurrentLTD = LTDRowMap.Pairs.Elements[i].ElementData.Value;
            auto TierData = (FFortLootTierData*)CurrentLTD.Value();

            // auto TierDataGroupStr = TierData->TierGroup.ToString();

            // std::cout << "TierData->TierGroup.ToString(): " << TierDataGroupStr << '\n';

            if (TierGroupName == TierData->GetTierGroup() /* TierDataGroupStr == TierGroupNameStr */ && TierData->GetWeight() != 0)
            {
                TierGroupLTDs.push_back(TierData);
            }
        }
    }

    if (TierGroupLTDs.size() == 0)
    {
        // std::cout << "Failed to find LTD!\n";
        std::cout << "Failed to find any LTD for: " << TierGroupName.ToString() << '\n';
        return LootDrops;
    }

    if (bPrint)
    {
        std::cout << "TierGroupLTDs.size(): " << TierGroupLTDs.size() << '\n';
    }

    FFortLootTierData* ChosenRowLootTierData = GetLootTierData(TierGroupLTDs, bPrint);

    if (!ChosenRowLootTierData) // Should NEVER happen
        return LootDrops;

    if (ChosenRowLootTierData->GetNumLootPackageDrops() <= 0)
        return PickLootDrops(TierGroupName, bPrint, ++recursive); // hm

    auto& LootPackageCategoryMinArray = ChosenRowLootTierData->GetLootPackageCategoryMinArray();
    auto& LootPackageCategoryWeightArray = ChosenRowLootTierData->GetLootPackageCategoryWeightArray();
    auto& LootPackageCategoryMaxArray = ChosenRowLootTierData->GetLootPackageCategoryMaxArray();

    if (LootPackageCategoryMinArray.ArrayNum != LootPackageCategoryWeightArray.ArrayNum ||
        LootPackageCategoryMinArray.ArrayNum != LootPackageCategoryMaxArray.ArrayNum)
        return PickLootDrops(TierGroupName, bPrint, ++recursive); // hm

    int MinimumLootDrops = 0;
    float NumLootPackageDrops = std::floor(ChosenRowLootTierData->GetNumLootPackageDrops());

    if (LootPackageCategoryMinArray.ArrayNum)
    {
        for (int i = 0; i < LootPackageCategoryMinArray.ArrayNum; i++)
        {
            if (LootPackageCategoryMinArray.at(i) > 0)
            {
                MinimumLootDrops += LootPackageCategoryMinArray.at(i);
            }
        }
    }

    if (MinimumLootDrops > NumLootPackageDrops)
    {

    }

    int SumLootPackageCategoryWeightArray = 0;

    for (int i = 0; i < LootPackageCategoryWeightArray.Num(); i++)
    {
        auto CategoryWeight = LootPackageCategoryWeightArray.at(i);

        if (CategoryWeight > 0)
        {
            auto CategoryMaxArray = LootPackageCategoryMaxArray.at(i);

            if (CategoryMaxArray < 0)
            {
                SumLootPackageCategoryWeightArray += CategoryWeight;
            }
        }
    }

    int SumLootPackageCategoryMinArray = 0;

    for (int i = 0; i < LootPackageCategoryMinArray.Num(); i++)
    {
        auto CategoryWeight = LootPackageCategoryMinArray.at(i);

        if (CategoryWeight > 0)
        {
            auto CategoryMaxArray = LootPackageCategoryMaxArray.at(i);

            if (CategoryMaxArray < 0)
            {
                SumLootPackageCategoryMinArray += CategoryWeight;
            }
        }
    }

    if (SumLootPackageCategoryWeightArray > SumLootPackageCategoryMinArray)
        return PickLootDrops(TierGroupName, bPrint, ++recursive); // hm

    std::vector<FFortLootPackageData*> TierGroupLPs;

    for (int p = 0; p < LPTables.size(); p++)
    {
        auto LP = LPTables[p];
        auto& LPRowMap = LP->GetRowMap();

        for (int i = 0; i < LPRowMap.Pairs.Elements.Num(); i++)
        {
            auto& CurrentLP = LPRowMap.Pairs.Elements[i].ElementData.Value;
            auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

            if (!LootPackage)
                continue;

            if (LootPackage->GetLootPackageID() == ChosenRowLootTierData->GetLootPackage() && LootPackage->GetWeight() != 0)
            {
                TierGroupLPs.push_back(LootPackage);
            }
        }
    }

    auto ChosenLootPackageName = ChosenRowLootTierData->GetLootPackage().ToString();

    if (ChosenLootPackageName.contains(".Empty"))
    {
        return PickLootDrops(TierGroupName, bPrint);
        // return LootDrops;
    }

    bool bIsWorldList = ChosenLootPackageName.contains("WorldList");

    if (bPrint)
    {
        std::cout << "NumLootPackageDrops Floored: " << NumLootPackageDrops << '\n';
        std::cout << "NumLootPackageDrops Original: " << ChosenRowLootTierData->GetNumLootPackageDrops() << '\n';
        std::cout << "TierGroupLPs.size(): " << TierGroupLPs.size() << '\n';
        std::cout << "ChosenLootPackageName: " << ChosenLootPackageName << '\n';

        /* float t = ChosenRowLootTierData->NumLootPackageDrops;

        int b = (int)((t + t) - 0.5) >> 1;
        auto c = ChosenRowLootTierData->NumLootPackageDrops - b;

        b += c >= (rand() * 0.000030518509);

        std::cout << "b: " << b << '\n'; */
    }

    LootDrops.reserve(NumLootPackageDrops);

    for (float i = 0; i < NumLootPackageDrops; i++)
    {
        if (i >= TierGroupLPs.size())
            break;

        auto TierGroupLP = TierGroupLPs.at(i);
        auto TierGroupLPStr = TierGroupLP->GetLootPackageCall().ToString();

        if (TierGroupLPStr.contains(".Empty"))
        {
            NumLootPackageDrops++;
            continue;
        }

        std::vector<FFortLootPackageData*> lootPackageCalls;

        if (bIsWorldList)
        {
            for (int j = 0; j < TierGroupLPs.size(); j++)
            {
                auto& CurrentLP = TierGroupLPs.at(j);

                if (CurrentLP->GetWeight() != 0)
                    lootPackageCalls.push_back(CurrentLP);
            }
        }
        else
        {
            for (int p = 0; p < LPTables.size(); p++)
            {
                auto LPRowMap = LPTables[p]->GetRowMap();

                for (int j = 0; j < LPRowMap.Pairs.Elements.Num(); j++)
                {
                    auto& CurrentLP = LPRowMap.Pairs.Elements[j].ElementData.Value;

                    auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

                    if (LootPackage->GetLootPackageID().ToString() == TierGroupLPStr && LootPackage->GetWeight() != 0)
                    {
                        lootPackageCalls.push_back(LootPackage);
                    }
                }
            }
        }

        if (lootPackageCalls.size() == 0)
        {
            // std::cout << "lootPackageCalls.size() == 0!\n";
            NumLootPackageDrops++; // ??
            continue;
        }

        FFortLootPackageData* LootPackageCall = GetLootPackage(lootPackageCalls);

        if (!LootPackageCall) // Should NEVER happen
            continue;

        auto ItemDef = LootPackageCall->GetItemDefinition().Get();

        if (!ItemDef)
        {
            NumLootPackageDrops++; // ??
            continue;
        }

        if (bPrint)
        {
            std::cout << std::format("[{}] {} {} {}\n", i, lootPackageCalls.size(), TierGroupLPStr, ItemDef->GetName());
        }

        LootDrops.push_back({ ItemDef, LootPackageCall->GetCount() });
    }

    return LootDrops;
}