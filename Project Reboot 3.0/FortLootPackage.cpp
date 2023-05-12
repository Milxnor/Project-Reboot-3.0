#include "FortLootPackage.h"

#include "DataTable.h"
#include "KismetMathLibrary.h"
#include "FortWeaponItemDefinition.h"
#include "UObjectArray.h"
#include "GameplayTagContainer.h"
#include "FortGameModeAthena.h"

struct FFortGameFeatureLootTableData
{
    TSoftObjectPtr<UDataTable> LootTierData;
    TSoftObjectPtr<UDataTable> LootPackageData;
};

#ifdef EXPERIMENTAL_LOOTING

template <typename RowStructType = uint8>
void CollectDataTablesRows(std::vector<UDataTable*> DataTables, std::map<FName, RowStructType*>* OutMap, std::function<bool(FName, RowStructType*)> Check = []() { return true; })
{
    std::vector<UDataTable*> DataTablesToIterate;

    static auto CompositeDataTableClass = FindObject<UClass>("/Script/Engine.CompositeDataTable");

    for (auto DataTable : DataTables)
    {
        // if (auto CompositeDataTable = Cast<UCompositeDataTable>(DataTable))
        if (DataTable->IsA(CompositeDataTableClass))
        {
            auto CompositeDataTable = DataTable;

            static auto ParentTablesOffset = DataTable->GetOffset("ParentTables");
            auto& ParentTables = DataTable->Get<TArray<UDataTable*>>(ParentTablesOffset);

            for (int i = 0; i < ParentTables.Num(); i++)
            {
                DataTablesToIterate.push_back(ParentTables.at(i));
            }
        }

        DataTablesToIterate.push_back(DataTable);
    }

    for (auto CurrentDataTable : DataTablesToIterate)
    {
        for (auto& CurrentPair : CurrentDataTable->GetRowMap())
        {
            if (Check(CurrentPair.Key(), (RowStructType*)CurrentPair.Value()))
                (*OutMap)[CurrentPair.Key()] = (RowStructType*)CurrentPair.Value();
        }
    }
}

int GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel)
{
    return 0;
}

float GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumberLootDrops)
{
    if (LootTierData->GetLootPackageCategoryMinArray().Num() != LootTierData->GetLootPackageCategoryWeightArray().Num()
        || LootTierData->GetLootPackageCategoryMinArray().Num() != LootTierData->GetLootPackageCategoryMaxArray().Num()
        )
        return 0;

    // return OriginalNumberLootDrops;

    float MinimumLootDrops = 0;

    if (LootTierData->GetLootPackageCategoryMinArray().Num() > 0)
    {
        for (int i = 0; i < LootTierData->GetLootPackageCategoryMinArray().Num(); i++)
        {
            // Fortnite does more here, we need to figure it out.
            MinimumLootDrops += LootTierData->GetLootPackageCategoryMinArray().at(i);
        }
    }

    if (MinimumLootDrops > OriginalNumberLootDrops)
    {
        LOG_INFO(LogLoot, "Requested {} loot drops but minimum drops is {} for loot package {}", OriginalNumberLootDrops, MinimumLootDrops, LootTierData->GetLootPackage().ToString());
        // Fortnite doesn't return here?
    }

    int SumLootPackageCategoryWeightArray = 0;

    if (LootTierData->GetLootPackageCategoryWeightArray().Num() > 0)
    {
        for (int i = 0; i < LootTierData->GetLootPackageCategoryWeightArray().Num(); i++)
        {
            // Fortnite does more here, we need to figure it out.

            if (LootTierData->GetLootPackageCategoryWeightArray().at(i) > 0)
            {
                auto LootPackageCategoryMaxArrayIt = LootTierData->GetLootPackageCategoryMaxArray().at(i);

                float IDK = 0; // TODO

                if (LootPackageCategoryMaxArrayIt < 0 || IDK < LootPackageCategoryMaxArrayIt)
                {
                    SumLootPackageCategoryWeightArray += LootTierData->GetLootPackageCategoryWeightArray().at(i);
                }
            }
        }
    }

    // if (MinimumLootDrops < OriginalNumberLootDrops) // real commeneted one to one
    {
        // IDK

        while (SumLootPackageCategoryWeightArray > 0)
        {
            // HONESTLY IDEK WHAT FORTNITE DOES HERE

            float v29 = (float)rand() * 0.000030518509;

            float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
                + (float)((float)SumLootPackageCategoryWeightArray * v29))
                + 0.5) >> 1;

            // OutLootTierInfo->Hello++;
            MinimumLootDrops++;

            if (MinimumLootDrops >= OriginalNumberLootDrops)
                return MinimumLootDrops;

            SumLootPackageCategoryWeightArray--;
        }

        /* if (MinimumLootDrops < OriginalNumberLootDrops)
        {
            std::cout << std::format("Requested {} loot drops but maximum drops is {} for loot package {}\n", OriginalNumberLootDrops, MinimumLootDrops, LootTierData->LootPackage.ToString());
        } */
    }

    return MinimumLootDrops;
}

/*struct UFortLootPackage
{
    int CurrentIdx = 0;
    std::vector<FFortItemEntry> ItemEntries;
}; */

FFortLootTierData* PickLootTierData(const std::vector<UDataTable*>& LTDTables, FName LootTierGroup, int WorldLevel = 0, int ForcedLootTier = -1, FName* OutRowName = nullptr) // Fortnite returns the row name and then finds the tier data again, but I really don't see the point of this.
{
    // This like isn't right, at all.

    float LootTier = ForcedLootTier;

    if (LootTier == -1)
    {
        // LootTier = 0;
    }
    else
    {
        // buncha code im too lazy to reverse
    }

    // IDIakuuyg8712u091fj120gvik

    // if (fabs(LootTier) <= 0.0000000099999999)
      //  return 0;

    std::map<FName, FFortLootTierData*> TierGroupLTDs;

    CollectDataTablesRows<FFortLootTierData>(LTDTables, &TierGroupLTDs, [&](FName RowName, FFortLootTierData* TierData) -> bool {
        if (LootTierGroup == TierData->GetTierGroup() && (LootTier == -1 ? true : LootTier == TierData->GetLootTier()))
        {
            return true;
        }

        return false;
        });

    FFortLootTierData* ChosenRowLootTierData = PickWeightedElement<FName, FFortLootTierData*>(TierGroupLTDs,
        [](FFortLootTierData* LootTierData) -> float { return LootTierData->GetWeight(); }, RandomFloatForLoot, -1,
        true, LootTier == -1 ? 1 : LootTier, OutRowName);

    if (!ChosenRowLootTierData)
        return nullptr;

    return ChosenRowLootTierData;
}

void PickLootDropsFromLootPackage(const std::vector<UDataTable*>& LPTables, const FName& LootPackageName, std::vector<LootDrop>* OutEntries, int LootPackageCategory = -1, bool bPrint = false)
{
    if (!OutEntries)
        return;

    std::map<FName, FFortLootPackageData*> LootPackageIDMap;

    CollectDataTablesRows<FFortLootPackageData>(LPTables, &LootPackageIDMap, [&](FName RowName, FFortLootPackageData* LootPackage) -> bool {
        if (LootPackage->GetLootPackageID() != LootPackageName)
        {
            return false;
        }

        if (LootPackageCategory != -1 && LootPackage->GetLootPackageCategory() != LootPackageCategory) // idk if proper
        {
            return false;
        }

        /* if (WorldLevel >= 0)
        {
            if (LootPackage->MaxWorldLevel >= 0 && WorldLevel > LootPackage->MaxWorldLevel)
                return 0;

            if (LootPackage->MinWorldLevel >= 0 && WorldLevel < LootPackage->MinWorldLevel)
                return 0;
        } */

        return true;
        });

    if (LootPackageIDMap.size() == 0)
    {
        // std::cout << std::format("Loot Package {} has no valid weights.\n", LootPackageName.ToString());
        return;
    }

    FName PickedPackageRowName;
    FFortLootPackageData* PickedPackage = PickWeightedElement<FName, FFortLootPackageData*>(LootPackageIDMap,
        [](FFortLootPackageData* LootPackageData) -> float { return LootPackageData->GetWeight(); }, RandomFloatForLoot,
        -1, true, 1, &PickedPackageRowName, bPrint);

    if (!PickedPackage)
        return;

    if (bPrint)
        LOG_INFO(LogLoot, "PickLootDropsFromLootPackage selected package {} with loot package category {} with weight {} from LootPackageIDMap of size: {}", PickedPackageRowName.ToString(), LootPackageCategory, PickedPackage->GetWeight(), LootPackageIDMap.size());

    if (PickedPackage->GetLootPackageCall().Data.Num() > 1)
    {
        if (PickedPackage->GetCount() > 0)
        {
            int v9 = 0;

            while (v9 < PickedPackage->GetCount())
            {
                int LootPackageCategoryToUseForLPCall = 0; // hmm
                
                PickLootDropsFromLootPackage(LPTables,
                    PickedPackage->GetLootPackageCall().Data.Data ? UKismetStringLibrary::Conv_StringToName(PickedPackage->GetLootPackageCall()) : FName(0),
                    OutEntries, LootPackageCategoryToUseForLPCall, bPrint
                );

                v9++;
            }
        }

        return;
    }

    auto ItemDefinition = PickedPackage->GetItemDefinition().Get(UFortItemDefinition::StaticClass(), true);

    if (!ItemDefinition)
    {
        LOG_INFO(LogLoot, "Loot Package {} does not contain a LootPackageCall or ItemDefinition.", PickedPackage->GetLootPackageID().ToString());
        return;
    }

    int ItemLevel = 0;

    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);
    int LoadedAmmo = WeaponItemDefinition ? WeaponItemDefinition->GetClipSize() : 0; // we shouldnt set loaded ammo here techinally

    if (auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition))
    {
        ItemLevel = 0; // GetItemLevel(WorldItemDefinition->LootLevelData, 0);
    }

    int CountMultiplier = 1;
    int FinalCount = CountMultiplier * PickedPackage->GetCount();

    if (FinalCount > 0)
    {
        int FinalItemLevel = 0;

        if (ItemLevel >= 0)
            FinalItemLevel = ItemLevel;

        while (FinalCount > 0)
        {
            int CurrentCountForEntry = PickedPackage->GetCount(); // Idk calls some itemdefinition vfunc

            OutEntries->push_back(LootDrop(FFortItemEntry::MakeItemEntry(ItemDefinition, CurrentCountForEntry, LoadedAmmo)));

            if (Engine_Version >= 424)
            {
                /*
                
                Alright, so Fortnite literally doesn't reference the first loot package category for chests and floor loot (didnt check rest).
                Usually the first loot package category in our case is ammo, so this is quite weird.
                I have no clue how Fortnite would actually add the ammo.

                Guess what, on the chapter 2 new loot tier groups, like FactionChests, they don't even have a package which has ammo as its loot package call.
                
                */ 

                bool IsWeapon = PickedPackage->GetLootPackageID().ToString().contains(".Weapon.") && WeaponItemDefinition; // ONG?

                if (IsWeapon)
                {
                    auto AmmoData = WeaponItemDefinition->GetAmmoData();

                    if (AmmoData)
                    {
                        int AmmoCount = AmmoData->GetDropCount(); // idk about this one

                        OutEntries->push_back(LootDrop(FFortItemEntry::MakeItemEntry(AmmoData, AmmoCount)));
                    }
                }
            }

            if (bPrint)
            {
                LOG_INFO(LogLoot, "Adding Item: {}", ItemDefinition->GetPathName());
            }

            FinalCount -= CurrentCountForEntry;
        }
    }
}

std::vector<LootDrop> PickLootDrops(FName TierGroupName, int ForcedLootTier, bool bPrint, int recursive)
{
    std::vector<LootDrop> LootDrops;

    if (recursive > 6)
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
    
    FName LootTierRowName;
    auto ChosenRowLootTierData = PickLootTierData(LTDTables, TierGroupName, 0, ForcedLootTier, &LootTierRowName);

    if (!ChosenRowLootTierData)
    {
        return LootDrops;
    }
    else if (bPrint)
    {
        LOG_INFO(LogLoot, "Picked loot tier data row {}", LootTierRowName.ToString());
    }

    // auto ChosenLootPackageName = ChosenRowLootTierData->GetLootPackage().ToString();

    // if (ChosenLootPackageName.contains(".Empty")) { return PickLootDropsNew(TierGroupName, bPrint, ++recursive); }

    float NumLootPackageDrops = ChosenRowLootTierData->GetNumLootPackageDrops();

    float NumberLootDrops = 0;

    if (NumLootPackageDrops > 0)
    {
        if (NumLootPackageDrops < 1)
        {
            NumberLootDrops = 1;
        }
        else
        {
            NumberLootDrops = (int)(float)((float)(NumLootPackageDrops + NumLootPackageDrops) - 0.5) >> 1;
            float v20 = NumLootPackageDrops - NumberLootDrops;
            if (v20 > 0.0000099999997)
            {
                NumberLootDrops += v20 >= (rand() * 0.000030518509);
            }
        }
    }

    float AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(ChosenRowLootTierData, NumberLootDrops);

    LootDrops.reserve(AmountOfLootPackageDrops);

    if (AmountOfLootPackageDrops > 0)
    {
        for (int i = 0; i < AmountOfLootPackageDrops; i++)
        {
            if (i >= ChosenRowLootTierData->GetLootPackageCategoryMinArray().Num())
                break;

            for (int j = 0; j < ChosenRowLootTierData->GetLootPackageCategoryMinArray().at(i); j++)
            {
                if (ChosenRowLootTierData->GetLootPackageCategoryMinArray().at(i) < 1)
                    break;

                int LootPackageCategory = i;

                PickLootDropsFromLootPackage(LPTables, ChosenRowLootTierData->GetLootPackage(), &LootDrops, LootPackageCategory, bPrint);
            }
        }
    }

    return LootDrops;
}
#else

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

std::vector<LootDrop> PickLootDrops(FName TierGroupName, bool bPrint, int recursive)
{
    std::vector<LootDrop> LootDrops;

    if (recursive > 6)
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
        return PickLootDrops(TierGroupName, bPrint, ++recursive);
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
                        // LOG_INFO(LogDev, "Adding LootPackage: {}", CurrentLP->GetAnnotation().ToString());
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

        LootDrops.push_back(LootDrop(FFortItemEntry::MakeItemEntry(ItemDef, LootPackageCall->GetCount(), WeaponDef ? WeaponDef->GetClipSize() : 0));
    }

    return LootDrops;
}
#endif
