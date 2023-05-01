#include "FortLootPackage.h"

#include "DataTable.h"
#include "KismetMathLibrary.h"
#include "FortWeaponItemDefinition.h"
#include "UObjectArray.h"
#include "GameplayTagContainer.h"
#include "FortGameModeAthena.h"

#include <random>

float GetRandomFloatForLooting(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    float random_number = dis(gen);

    return random_number;

    return UKismetMathLibrary::RandomFloatInRange(min, max);
}

static FFortLootTierData* GetLootTierData(std::vector<FFortLootTierData*>& LootTierData, float TotalWeight)
{
    FFortLootTierData* SelectedItem = nullptr;

    for (auto Item : LootTierData)
    {
        TotalWeight += Item->GetWeight();
    }

    float RandomNumber = GetRandomFloatForLooting(0, TotalWeight); // is -1 needed?

    for (auto Item : LootTierData)
    {
        if (RandomNumber <= Item->GetWeight())
        {
            SelectedItem = Item;
            break;
        }

        RandomNumber -= Item->GetWeight();
    }

    if (!SelectedItem)
        return GetLootTierData(LootTierData, TotalWeight);

    return SelectedItem;
}

static FFortLootPackageData* GetLootPackage(std::vector<FFortLootPackageData*>& LootPackages, float TotalWeight)
{
    FFortLootPackageData* SelectedItem = nullptr;

    float RandomNumber = GetRandomFloatForLooting(0, TotalWeight); // is -1 needed?

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
        return GetLootPackage(LootPackages, TotalWeight);

    return SelectedItem;
}

struct FFortGameFeatureLootTableData
{
    TSoftObjectPtr<UDataTable> LootTierData;
    TSoftObjectPtr<UDataTable> LootPackageData; 
};

std::vector<LootDrop> PickLootDrops(FName TierGroupName, bool bPrint, int recursive)
{
    std::vector<LootDrop> LootDrops;

    if (recursive > 10)
        return LootDrops;

    auto GameState = ((AFortGameModeAthena*)GetWorld()->GetGameMode())->GetGameStateAthena();
    static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

    static std::vector<UDataTable*> LTDTables;
    static std::vector<UDataTable*> LPTables;

    static auto CompositeDataTableClass = FindObject<UClass>(L"/Script/Engine.CompositeDataTable");

    static int LastNum1 = 14915;

    auto CurrentPlaylist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GameState->GetCurrentPlaylist();

    if (LastNum1 != Globals::AmountOfListens)
    {
        LastNum1 = Globals::AmountOfListens;

        LTDTables.clear();
        LPTables.clear();

        bool bFoundPlaylistTable = false;

        if (CurrentPlaylist)
        {
            static auto LootTierDataOffset = CurrentPlaylist->GetOffset("LootTierData");
            auto& LootTierDataSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootTierDataOffset);

            static auto LootPackagesOffset = CurrentPlaylist->GetOffset("LootPackages");
            auto& LootPackagesSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootPackagesOffset);

            if (LootTierDataSoft.IsValid() && LootPackagesSoft.IsValid())
            {
                auto LootTierDataStr = LootTierDataSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                auto LootPackagesStr = LootPackagesSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                auto LootTierDataTableIsComposite = LootTierDataStr.contains("Composite");
                auto LootPackageTableIsComposite = LootPackagesStr.contains("Composite");

                auto StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                auto StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                if (StrongLootTierData && StrongLootPackage)
                {
                    LTDTables.push_back(StrongLootTierData);
                    LPTables.push_back(StrongLootPackage);

                    bFoundPlaylistTable = true;
                }
            }
        }

        if (!bFoundPlaylistTable)
        {
            LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
            LPTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
        }

        // LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootTierData_Client.AthenaLootTierData_Client"));
        // LPTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootPackages_Client.AthenaLootPackages_Client"));

        static auto FortGameFeatureDataClass = FindObject<UClass>("/Script/FortniteGame.FortGameFeatureData");

        if (FortGameFeatureDataClass)
        {
            for (int i = 0; i < ChunkedObjects->Num(); i++)
            {
                auto Object = ChunkedObjects->GetObjectByIndex(i);

                if (!Object)
                    continue;

                if (Object->IsA(FortGameFeatureDataClass))
                {
                    auto GameFeatureData = Object;
                    static auto DefaultLootTableDataOffset = GameFeatureData->GetOffset("DefaultLootTableData");

                    if (DefaultLootTableDataOffset != -1)
                    {
                        auto DefaultLootTableData = GameFeatureData->GetPtr<FFortGameFeatureLootTableData>(DefaultLootTableDataOffset);

                        auto LootTierDataTableStr = DefaultLootTableData->LootTierData.SoftObjectPtr.ObjectID.AssetPathName.ToString();

                        auto LootTierDataTableIsComposite = LootTierDataTableStr.contains("Composite");
                        auto LootPackageTableStr = DefaultLootTableData->LootPackageData.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                        auto LootPackageTableIsComposite = LootPackageTableStr.contains("Composite");

                        auto LootTierDataPtr = DefaultLootTableData->LootTierData.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                        auto LootPackagePtr = DefaultLootTableData->LootPackageData.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                        if (LootPackagePtr)
                        {
                            LPTables.push_back(LootPackagePtr);
                        }

                        if (CurrentPlaylist)
                        {
                            static auto PlaylistOverrideLootTableDataOffset = GameFeatureData->GetOffset("PlaylistOverrideLootTableData");
                            auto& PlaylistOverrideLootTableData = GameFeatureData->Get<TMap<FGameplayTag, FFortGameFeatureLootTableData>>(PlaylistOverrideLootTableDataOffset);

                            static auto GameplayTagContainerOffset = CurrentPlaylist->GetOffset("GameplayTagContainer");
                            auto GameplayTagContainer = CurrentPlaylist->GetPtr<FGameplayTagContainer>(GameplayTagContainerOffset);

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); i++)
                            {
                                auto& Tag = GameplayTagContainer->GameplayTags.At(i);

                                for (auto& Value : PlaylistOverrideLootTableData)
                                {
                                    auto CurrentOverrideTag = Value.First;

                                    if (Tag.TagName == CurrentOverrideTag.TagName)
                                    {
                                        auto OverrideLootPackageTableStr = Value.Second.LootPackageData.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                                        auto bOverrideIsComposite = OverrideLootPackageTableStr.contains("Composite");

                                        auto ptr = Value.Second.LootPackageData.Get(bOverrideIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                                        if (ptr)
                                        {
                                            /* if (bOverrideIsComposite)
                                            {
                                                static auto ParentTablesOffset = ptr->GetOffset("ParentTables");

                                                auto ParentTables = ptr->GetPtr<TArray<UDataTable*>>(ParentTablesOffset);

                                                for (int z = 0; z < ParentTables->size(); z++)
                                                {
                                                    auto ParentTable = ParentTables->At(z);

                                                    if (ParentTable)
                                                    {
                                                        LPTables.push_back(ParentTable);
                                                    }
                                                }
                                            } */

                                            LPTables.push_back(ptr);
                                        }
                                    }
                                }
                            }
                        }

                        if (LootTierDataPtr)
                        {
                            LTDTables.push_back(LootTierDataPtr);
                        }

                        if (CurrentPlaylist)
                        {
                            static auto PlaylistOverrideLootTableDataOffset = GameFeatureData->GetOffset("PlaylistOverrideLootTableData");
                            auto& PlaylistOverrideLootTableData = GameFeatureData->Get<TMap<FGameplayTag, FFortGameFeatureLootTableData>>(PlaylistOverrideLootTableDataOffset);

                            static auto GameplayTagContainerOffset = CurrentPlaylist->GetOffset("GameplayTagContainer");
                            auto GameplayTagContainer = CurrentPlaylist->GetPtr<FGameplayTagContainer>(GameplayTagContainerOffset);

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); i++)
                            {
                                auto& Tag = GameplayTagContainer->GameplayTags.At(i);

                                for (auto& Value : PlaylistOverrideLootTableData)
                                {
                                    auto CurrentOverrideTag = Value.First;

                                    if (Tag.TagName == CurrentOverrideTag.TagName)
                                    {
                                        auto OverrideLootTierDataStr = Value.Second.LootTierData.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                                        auto bOverrideIsComposite = OverrideLootTierDataStr.contains("Composite");

                                        auto ptr = Value.Second.LootTierData.Get(bOverrideIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                                        if (ptr)
                                        {
                                            /* if (bOverrideIsComposite)
                                            {
                                                static auto ParentTablesOffset = ptr->GetOffset("ParentTables");

                                                auto ParentTables = ptr->GetPtr<TArray<UDataTable*>>(ParentTablesOffset);

                                                for (int z = 0; z < ParentTables->size(); z++)
                                                {
                                                    auto ParentTable = ParentTables->At(z);

                                                    if (ParentTable)
                                                    {
                                                        LTDTables.push_back(ParentTable);
                                                    }
                                                }
                                            } */

                                            LTDTables.push_back(ptr);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int i = 0; i < LTDTables.size(); i++)
        {
            auto& Table = LTDTables.at(i);

            if (!Table->IsValidLowLevel())
            {
                continue;
            }

            Table->AddToRoot();
            LOG_INFO(LogDev, "[{}] LTD {}", i, Table->GetFullName());
        }

        for (int i = 0; i < LPTables.size(); i++)
        {
            auto& Table = LPTables.at(i);

            if (!Table->IsValidLowLevel())
            {
                continue;
            }

            Table->AddToRoot();
            LOG_INFO(LogDev, "[{}] LP {}", i, Table->GetFullName());
        } 
    }

    if (Fortnite_Version <= 6 || std::floor(Fortnite_Version) == 9) // ahhh
    {
        LTDTables.clear();
        LPTables.clear();

        bool bFoundPlaylistTable = false;

        if (CurrentPlaylist)
        {
            static auto LootTierDataOffset = CurrentPlaylist->GetOffset("LootTierData");
            auto& LootTierDataSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootTierDataOffset);

            static auto LootPackagesOffset = CurrentPlaylist->GetOffset("LootPackages");
            auto& LootPackagesSoft = CurrentPlaylist->Get<TSoftObjectPtr<UDataTable>>(LootPackagesOffset);

            if (LootTierDataSoft.IsValid() && LootPackagesSoft.IsValid())
            {
                auto LootTierDataStr = LootTierDataSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                auto LootPackagesStr = LootPackagesSoft.SoftObjectPtr.ObjectID.AssetPathName.ToString();
                auto LootTierDataTableIsComposite = LootTierDataStr.contains("Composite");
                auto LootPackageTableIsComposite = LootPackagesStr.contains("Composite");

                auto StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                auto StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

                if (StrongLootTierData && StrongLootPackage)
                {
                    LTDTables.push_back(StrongLootTierData);
                    LPTables.push_back(StrongLootPackage);

                    bFoundPlaylistTable = true;
                }
            }
        }

        if (!bFoundPlaylistTable)
        {
            LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
            LPTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
        }
    }

    if (LTDTables.size() <= 0 || LPTables.size() <= 0)
    {
        LOG_WARN(LogLoot, "Empty tables! ({} {})", LTDTables.size(), LPTables.size());
        return LootDrops;
    }

    std::vector<FFortLootTierData*> TierGroupLTDs;
    
    float TotalTierGroupLTDsWeight = 0;

    for (int p = 0; p < LTDTables.size(); p++)
    {
        auto LTD = LTDTables[p];

        // if (bPrint)
            // LOG_INFO(LogLoot, "LTD: {}", !LTD->IsValidLowLevel() ? "BadRead" : LTD->GetFullName());

        if (!LTD->IsValidLowLevel())
        {
            // if (bPrint)
                LOG_INFO(LogLoot, "BadRead!");

            continue;
        }

        auto& LTDRowMap = LTD->GetRowMap();

        for (auto& CurrentLTD : LTDRowMap)
        {
            auto TierData = (FFortLootTierData*)CurrentLTD.Value();

            if (IsBadReadPtr(TierData, 8)) // this shouldn't be needed
                continue;

            if (TierGroupName == TierData->GetTierGroup() && TierData->GetWeight() != 0)
            {
                TotalTierGroupLTDsWeight += TierData->GetWeight();
                TierGroupLTDs.push_back(TierData);
            }
        }
    }

    if (TierGroupLTDs.size() == 0)
    {
        LOG_WARN(LogLoot, "Failed to find any LTD for: {}", TierGroupName.ToString());
        return LootDrops;
    }

    FFortLootTierData* ChosenRowLootTierData = GetLootTierData(TierGroupLTDs, TotalTierGroupLTDsWeight);

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

    static int AmountToAdd = Engine_Version >= 424 ? Engine_Version >= 500 ? 1 : 1 : 0; // fr

    float NumLootPackageDrops = std::floor(ChosenRowLootTierData->GetNumLootPackageDrops() + AmountToAdd);

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

    // if (SumLootPackageCategoryWeightArray > SumLootPackageCategoryMinArray)
        // return PickLootDrops(TierGroupName, bPrint, ++recursive); // hm

    std::vector<FFortLootPackageData*> TierGroupLPs;

    for (int p = 0; p < LPTables.size(); p++)
    {
        auto LP = LPTables[p];

        if (!LP->IsValidLowLevel())
            continue;

        auto& LPRowMap = LP->GetRowMap();

        for (auto& CurrentLP : LPRowMap)
        {
            auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

            if (!LootPackage)
                continue;

            if (LootPackage->GetLootPackageID() == ChosenRowLootTierData->GetLootPackage() && LootPackage->GetWeight() != 0 && LootPackage->GetCount() != 0)
            {
                TierGroupLPs.push_back(LootPackage);
            }
        }
    }

    auto ChosenLootPackageName = ChosenRowLootTierData->GetLootPackage().ToString();

    if (ChosenLootPackageName.contains(".Empty")) // I don't think?
    {
        return PickLootDrops(TierGroupName, bPrint);
        // return LootDrops;
    }

    bool bIsWorldList = ChosenLootPackageName.contains("WorldList");

    if (bPrint)
    {
        LOG_INFO(LogLoot, "NumLootPackageDrops Floored: {}", NumLootPackageDrops);
        LOG_INFO(LogLoot, "NumLootPackageDrops Original: {}", ChosenRowLootTierData->GetNumLootPackageDrops());
        LOG_INFO(LogLoot, "TierGroupLPs.size(): {}", TierGroupLPs.size());
        LOG_INFO(LogLoot, "ChosenLootPackageName: {}", ChosenLootPackageName);

        /* float t = ChosenRowLootTierData->NumLootPackageDrops;

        int b = (int)((t + t) - 0.5) >> 1;
        auto c = ChosenRowLootTierData->NumLootPackageDrops - b;

        b += c >= (rand() * 0.000030518509);

        std::cout << "b: " << b << '\n'; */
    }

    LootDrops.reserve(NumLootPackageDrops);

    for (float i = 0; i < NumLootPackageDrops; i++)
    {
        FFortLootPackageData* TierGroupLP = nullptr;

        if (i >= TierGroupLPs.size())
        {
            break;
            /* auto randomNumberFloat = UKismetMathLibrary::RandomFloatInRange(0, TierGroupLPs.size());
            auto randomNumberFloored = std::floor((int)randomNumberFloat); // idk

            if (bPrint)
                LOG_INFO(LogLoot, "randomNumberFloat: {} randomNumberFloored: {}", randomNumberFloat, randomNumberFloored);

            TierGroupLP = TierGroupLPs.at(randomNumberFloored); */

            TierGroupLP = TierGroupLPs.at(i - NumLootPackageDrops); // Once we fix chapter 2 loot package drops, we can use this
        }
        else
        {
            TierGroupLP = TierGroupLPs.at(i);
        }

        if (!TierGroupLP)
            continue;

        auto& LootPackageCallFStr = TierGroupLP->GetLootPackageCall();
        auto TierGroupLPStr = LootPackageCallFStr.IsValid() ? LootPackageCallFStr.ToString() : "InvalidLootPackageCall.Empty";

        if (bPrint)
            LOG_INFO(LogLoot, "TierGroupLPStr: {}", TierGroupLPStr);

        if (!bIsWorldList && TierGroupLPStr.contains(".Empty"))
        {
            NumLootPackageDrops++;
            continue;
        }

        std::vector<FFortLootPackageData*> lootPackageCalls;
        float lootPackageCallsTotalWeight = 0;

        if (bIsWorldList)
        {
            for (int j = 0; j < TierGroupLPs.size(); j++)
            {
                auto& CurrentLP = TierGroupLPs.at(j);

                if (CurrentLP->GetWeight() != 0)
                {
                    lootPackageCallsTotalWeight += CurrentLP->GetWeight();
                    lootPackageCalls.push_back(CurrentLP);

                    if (bPrint)
                    {
                        LOG_INFO(LogDev, "Adding LootPackage: {}", CurrentLP->GetAnnotation().ToString());
                    }
                }
            }
        }
        else
        {
            for (int p = 0; p < LPTables.size(); p++)
            {
                if (!LPTables[p]->IsValidLowLevel())
                    continue;

                auto& LPRowMap = LPTables[p]->GetRowMap();

                for (auto& CurrentLP : LPRowMap)
                {
                    auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

                    if (LootPackage->GetLootPackageID().ToString() == TierGroupLPStr && LootPackage->GetWeight() != 0 && LootPackage->GetCount() != 0)
                    {
                        lootPackageCallsTotalWeight += LootPackage->GetWeight();
                        lootPackageCalls.push_back(LootPackage);
                    }
                }
            }
        }

        if (bPrint)
            LOG_INFO(LogLoot, "lootPackageCalls.size(): {}", lootPackageCalls.size());

        if (lootPackageCalls.size() == 0)
        {
            // std::cout << "lootPackageCalls.size() == 0!\n";
            NumLootPackageDrops++;
            continue;
        }

        FFortLootPackageData* LootPackageCall = GetLootPackage(lootPackageCalls, lootPackageCallsTotalWeight);

        if (!LootPackageCall) // Should NEVER happen
        {
            LOG_ERROR(LogLoot, "Failed to get any loot package call??");
            NumLootPackageDrops++;
            continue;
        }

        auto ItemDef = LootPackageCall->GetItemDefinition().Get(UFortItemDefinition::StaticClass(), true);

        if (!ItemDef)
        {
            NumLootPackageDrops++;
            continue;
        }

        if (bPrint)
        {
           LOG_INFO(LogLoot, "[{}] {} {} {}", i, lootPackageCalls.size(), TierGroupLPStr, ItemDef->GetName());
        }

        auto WeaponDef = Cast<UFortWeaponItemDefinition>(ItemDef);

        LootDrop lootDrop{};
        lootDrop.ItemDefinition = ItemDef;
        lootDrop.LoadedAmmo = WeaponDef ? WeaponDef->GetClipSize() : 0;
        lootDrop.Count = LootPackageCall->GetCount();

        LootDrops.push_back(lootDrop);
    }

    return LootDrops;
}