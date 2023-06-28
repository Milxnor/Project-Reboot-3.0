#include "FortLootPackage.h"

#include "DataTable.h"
#include "KismetMathLibrary.h"
#include "FortWeaponItemDefinition.h"
#include "UObjectArray.h"
#include "GameplayTagContainer.h"
#include "FortGameModeAthena.h"
#include "FortLootLevel.h"

struct FFortGameFeatureLootTableData
{
    TSoftObjectPtr<UDataTable> LootTierData;
    TSoftObjectPtr<UDataTable> LootPackageData;
};

#define LOOTING_MAP_TYPE std::map // uhhh // TODO (Milxnor) switch bad to map

template <typename RowStructType = uint8>
void CollectDataTablesRows(const std::vector<UDataTable*>& DataTables, LOOTING_MAP_TYPE<FName, RowStructType*>* OutMap, std::function<bool(FName, RowStructType*)> Check = []() { return true; })
{
    std::vector<UDataTable*> DataTablesToIterate;

    static auto CompositeDataTableClass = FindObject<UClass>(L"/Script/Engine.CompositeDataTable");

    for (UDataTable* DataTable : DataTables)
    {
        if (!Addresses::LoadAsset && !DataTable->IsValidLowLevel())
        {
            continue; // Remove from vector?
        }

        // if (auto CompositeDataTable = Cast<UCompositeDataTable>(DataTable))
        if (DataTable->IsA(CompositeDataTableClass))
        {
            auto CompositeDataTable = DataTable;

            static auto ParentTablesOffset = CompositeDataTable->GetOffset("ParentTables");
            auto& ParentTables = CompositeDataTable->Get<TArray<UDataTable*>>(ParentTablesOffset);

            for (int i = 0; i < ParentTables.Num(); ++i)
            {
                DataTablesToIterate.push_back(ParentTables.at(i));
            }
        }

        DataTablesToIterate.push_back(DataTable);
    }

    for (auto CurrentDataTable : DataTablesToIterate)
    {
        for (TPair<FName, uint8_t*>& CurrentPair : CurrentDataTable->GetRowMap())
        {
            if (Check(CurrentPair.Key(), (RowStructType*)CurrentPair.Value()))
            {
                // LOG_INFO(LogDev, "Setting key with {} comp {} num: {} then iterating through map!", CurrentPair.Key().ToString(), CurrentPair.Key().ComparisonIndex.Value, CurrentPair.Key().Number);
                (*OutMap)[CurrentPair.Key()] = (RowStructType*)CurrentPair.Value();

                /* for (auto PairInOutMap : *OutMap)
                {
                    // LOG_INFO(LogDev, "Current Row Key {} comp {} num: {}!", PairInOutMap.first.ToString(), PairInOutMap.first.ComparisonIndex.Value, PairInOutMap.first.Number);
                } */
            }
        }
    }
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
        for (int i = 0; i < LootTierData->GetLootPackageCategoryMinArray().Num(); ++i)
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
        for (int i = 0; i < LootTierData->GetLootPackageCategoryWeightArray().Num(); ++i)
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

            float v29 = (float)rand() * 0.000030518509f;

            float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
                + (float)((float)SumLootPackageCategoryWeightArray * v29))
                + 0.5f) >> 1;

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

FFortLootTierData* PickLootTierData(const std::vector<UDataTable*>& LTDTables, FName LootTierGroup, int ForcedLootTier = -1, FName* OutRowName = nullptr) // Fortnite returns the row name and then finds the tier data again, but I really don't see the point of this.
{
    // This like isn't right, at all.

    float LootTier = ForcedLootTier;

    if (LootTier == -1)
    {
        // LootTier = ??
    }
    else
    {
        // buncha code im too lazy to reverse
    }

    // if (fabs(LootTier) <= 0.0000000099999999f)
      //  return 0;

    int Multiplier = LootTier == -1 ? 1 : LootTier; // Idk i think we need to fill out the code above for this to work properly maybe

    LOOTING_MAP_TYPE<FName, FFortLootTierData*> TierGroupLTDs;

    CollectDataTablesRows<FFortLootTierData>(LTDTables, &TierGroupLTDs, [&](FName RowName, FFortLootTierData* TierData) -> bool {
        if (LootTierGroup == TierData->GetTierGroup())
        {
            if ((LootTier == -1 ? true : LootTier == TierData->GetLootTier()))
            {
                return true;
            }
        }

        return false;
        });

    // LOG_INFO(LogDev, "TierGroupLTDs.size(): {}", TierGroupLTDs.size());

    FFortLootTierData* ChosenRowLootTierData = PickWeightedElement<FName, FFortLootTierData*>(TierGroupLTDs,
        [](FFortLootTierData* LootTierData) -> float { return LootTierData->GetWeight(); }, RandomFloatForLoot, -1,
        true, Multiplier, OutRowName);

    return ChosenRowLootTierData;
}

void PickLootDropsFromLootPackage(const std::vector<UDataTable*>& LPTables, const FName& LootPackageName, std::vector<LootDrop>* OutEntries, int LootPackageCategory = -1, int WorldLevel = 0, bool bPrint = false, bool bCombineDrops = true)
{
    if (!OutEntries)
        return;

    LOOTING_MAP_TYPE<FName, FFortLootPackageData*> LootPackageIDMap;

    CollectDataTablesRows<FFortLootPackageData>(LPTables, &LootPackageIDMap, [&](FName RowName, FFortLootPackageData* LootPackage) -> bool {
        if (LootPackage->GetLootPackageID() != LootPackageName)
        {
            return false;
        }

        if (LootPackageCategory != -1 && LootPackage->GetLootPackageCategory() != LootPackageCategory) // idk if proper
        {
            return false;
        }

        if (WorldLevel >= 0)
        {
            if (LootPackage->GetMaxWorldLevel() >= 0 && WorldLevel > LootPackage->GetMaxWorldLevel())
                return 0;

            if (LootPackage->GetMinWorldLevel() >= 0 && WorldLevel < LootPackage->GetMinWorldLevel())
                return 0;
        }

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
                    OutEntries, LootPackageCategoryToUseForLPCall, WorldLevel, bPrint
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

    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);
    int LoadedAmmo = WeaponItemDefinition ? WeaponItemDefinition->GetClipSize() : 0; // we shouldnt set loaded ammo here techinally

    auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);

    if (!WorldItemDefinition) // hahahah not proper!!
        return;

    int ItemLevel = UFortLootLevel::GetItemLevel(WorldItemDefinition->GetLootLevelData(), WorldLevel);

    int CountMultiplier = 1;
    int FinalCount = CountMultiplier * PickedPackage->GetCount();

    if (FinalCount > 0)
    {
        int FinalItemLevel = 0;

        if (ItemLevel >= 0)
            FinalItemLevel = ItemLevel;

        while (FinalCount > 0)
        {
            int MaxStackSize = ItemDefinition->GetMaxStackSize();

            int CurrentCountForEntry = MaxStackSize;

            if (FinalCount <= MaxStackSize)
                CurrentCountForEntry = FinalCount;

            if (CurrentCountForEntry <= 0)
                CurrentCountForEntry = 0;

            auto ActualItemLevel = WorldItemDefinition->PickLevel(FinalItemLevel);

            bool bHasCombined = false;

            if (bCombineDrops)
            {
                for (auto& CurrentLootDrop : *OutEntries)
                {
                    if (CurrentLootDrop->GetItemDefinition() == ItemDefinition)
                    {
                        int NewCount = CurrentLootDrop->GetCount() + CurrentCountForEntry;

                        if (NewCount <= ItemDefinition->GetMaxStackSize())
                        {
                            bHasCombined = true;
                            CurrentLootDrop->GetCount() = NewCount;
                        }
                    }
                }
            }

            if (!bHasCombined)
            {
                OutEntries->push_back(LootDrop(FFortItemEntry::MakeItemEntry(ItemDefinition, CurrentCountForEntry, LoadedAmmo, MAX_DURABILITY, ActualItemLevel)));
            }

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

// #define brudda

std::vector<LootDrop> PickLootDrops(FName TierGroupName, int WorldLevel, int ForcedLootTier, bool bPrint, int recursive, bool bCombineDrops)
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

                UDataTable* StrongLootTierData = nullptr;
                UDataTable* StrongLootPackage = nullptr;

                if (!Addresses::LoadAsset)
                {
                    StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                    StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                }
                else
                {
                    StrongLootTierData = (UDataTable*)Assets::LoadAsset(LootTierDataSoft.SoftObjectPtr.ObjectID.AssetPathName);
                    StrongLootPackage = (UDataTable*)Assets::LoadAsset(LootPackagesSoft.SoftObjectPtr.ObjectID.AssetPathName);
                }

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
            if (Addresses::LoadAsset)
            {
                LTDTables.push_back((UDataTable*)Assets::LoadAsset(UKismetStringLibrary::Conv_StringToName(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client")));
                LPTables.push_back((UDataTable*)Assets::LoadAsset(UKismetStringLibrary::Conv_StringToName(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client")));
            }
            else
            {
                LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
                LPTables.push_back(LoadObject<UDataTable>(L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
            }
        }

        // LTDTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootTierData_Client.AthenaLootTierData_Client"));
        // LPTables.push_back(LoadObject<UDataTable>(L"/Game/Athena/Playlists/Playground/AthenaLootPackages_Client.AthenaLootPackages_Client"));

        static auto FortGameFeatureDataClass = FindObject<UClass>(L"/Script/FortniteGame.FortGameFeatureData");

        if (FortGameFeatureDataClass)
        {
            for (int i = 0; i < ChunkedObjects->Num(); ++i)
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

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); ++i)
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

                            for (int i = 0; i < GameplayTagContainer->GameplayTags.Num(); ++i)
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

        for (int i = 0; i < LTDTables.size(); ++i)
        {
            auto& Table = LTDTables.at(i);

            if (!Table->IsValidLowLevel())
            {
                continue;
            }

            Table->AddToRoot();
            LOG_INFO(LogDev, "[{}] LTD {}", i, Table->GetFullName());
        }

        for (int i = 0; i < LPTables.size(); ++i)
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

    if (!Addresses::LoadAsset)
    {
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

                    UDataTable* StrongLootTierData = nullptr;
                    UDataTable* StrongLootPackage = nullptr;

                    StrongLootTierData = LootTierDataSoft.Get(LootTierDataTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);
                    StrongLootPackage = LootPackagesSoft.Get(LootPackageTableIsComposite ? CompositeDataTableClass : UDataTable::StaticClass(), true);

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

    }

    if (LTDTables.size() <= 0 || LPTables.size() <= 0)
    {
        LOG_WARN(LogLoot, "Empty tables! ({} {})", LTDTables.size(), LPTables.size());
        return LootDrops;
    }

    FName LootTierRowName;
    auto ChosenRowLootTierData = PickLootTierData(LTDTables, TierGroupName, ForcedLootTier, &LootTierRowName);

    if (!ChosenRowLootTierData)
    {
        LOG_INFO(LogLoot, "Failed to find LootTierData row for {} with loot tier {}", TierGroupName.ToString(), ForcedLootTier);
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
            NumberLootDrops = (int)(float)((float)(NumLootPackageDrops + NumLootPackageDrops) - 0.5f) >> 1;
            float v20 = NumLootPackageDrops - NumberLootDrops;
            if (v20 > 0.0000099999997f)
            {
                NumberLootDrops += v20 >= (rand() * 0.000030518509f);
            }
        }
    }

    float AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(ChosenRowLootTierData, NumberLootDrops);

    LootDrops.reserve(AmountOfLootPackageDrops);

    if (AmountOfLootPackageDrops > 0)
    {
        for (int i = 0; i < AmountOfLootPackageDrops; ++i)
        {
            if (i >= ChosenRowLootTierData->GetLootPackageCategoryMinArray().Num())
                break;

            for (int j = 0; j < ChosenRowLootTierData->GetLootPackageCategoryMinArray().at(i); ++j)
            {
                if (ChosenRowLootTierData->GetLootPackageCategoryMinArray().at(i) < 1)
                    break;

                int LootPackageCategory = i;

                PickLootDropsFromLootPackage(LPTables, ChosenRowLootTierData->GetLootPackage(), &LootDrops, LootPackageCategory, WorldLevel, bPrint);
            }
        }
    }

    return LootDrops;
}