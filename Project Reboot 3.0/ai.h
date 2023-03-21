#pragma once

#include "reboot.h"
#include "Actor.h"
#include "SoftObjectPath.h"
#include "KismetStringLibrary.h"
#include "GameplayStatics.h"
#include "FortPlayerPawn.h"
#include "FortPlayerController.h"
#include "FortPlayerControllerAthena.h"

using UNavigationSystemV1 = UObject;
using UNavigationSystemConfig = UObject;
using AAthenaNavSystemConfigOverride = UObject;
using UAthenaNavSystem = UObject;
using UAthenaNavSystemConfig = UObject;

enum class EFNavigationSystemRunMode : uint8_t
{
    InvalidMode = 0,
    GameMode = 1,
    EditorMode = 2,
    SimulationMode = 3,
    PIEMode = 4,
    FNavigationSystemRunMode_MAX = 5
};

enum class ENavSystemOverridePolicy : uint8_t
{
    Override = 0,
    Append = 1,
    Skip = 2,
    ENavSystemOverridePolicy_MAX = 3
};

extern inline void (*NavSystemCleanUpOriginal)(UNavigationSystemV1*, uint8) = nullptr;

extern inline void (*AddNavigationSystemToWorldOriginal)(UWorld& WorldOwner, EFNavigationSystemRunMode RunMode, UNavigationSystemConfig* NavigationSystemConfig, char bInitializeForWorld,
    char bOverridePreviousNavSys) = nullptr;

static void SetNavigationSystem(AAthenaNavSystemConfigOverride* NavSystemOverride)
{
    auto WorldSettings = GetWorld()->K2_GetWorldSettings();

    NavSystemOverride->Get<ENavSystemOverridePolicy>("OverridePolicy") = ENavSystemOverridePolicy::Append;

    WorldSettings->Get("NavigationSystemConfigOverride") = NavSystemOverride->Get("NavigationSystemConfig");
    WorldSettings->Get("NavigationSystemConfig")->Get<bool>("bIsOverriden") = true;

    auto NavSystem = (UAthenaNavSystem*)GetWorld()->Get("NavigationSystem");

    NavSystemCleanUpOriginal(NavSystem, 0);

    GetWorld()->Get("NavigationSystem") = nullptr;

    if (!NavSystemOverride->Get("NavigationSystemConfig"))
        return;

    auto& ClassPath = NavSystemOverride->Get("NavigationSystemConfig")->Get<FSoftObjectPath>("NavigationSystemClass");

    auto NewNavSystemClass = FindObject<UClass>(ClassPath.AssetPathName.ToString());

    if (!NewNavSystemClass)
        return;

    LOG_INFO(LogAI, "Setup navigation system.");

    AddNavigationSystemToWorldOriginal(*GetWorld(), EFNavigationSystemRunMode::GameMode, NavSystemOverride->Get("NavigationSystemConfig"), true, false);
}

static void SetupNavConfig()
{
    static auto AthenaNavSystemConfigOverrideClass = FindObject<UClass>("/Script/FortniteGame.AthenaNavSystemConfigOverride");
    auto NavSystemOverride = GetWorld()->SpawnActor<AActor>(AthenaNavSystemConfigOverrideClass);

    if (!NavSystemOverride)
        return;

    static auto AthenaNavSystemConfigClass = FindObject<UClass>("/Script/FortniteGame.AthenaNavSystemConfig");
    auto AthenaNavConfig = (UAthenaNavSystemConfig*)UGameplayStatics::SpawnObject(AthenaNavSystemConfigClass, NavSystemOverride);
    AthenaNavConfig->Get<bool>("bUseBuildingGridAsNavigableSpace") = false;
    AthenaNavConfig->Get<bool>("bUsesStreamedInNavLevel") = true;
    AthenaNavConfig->Get<bool>("bAllowAutoRebuild") = true;
    AthenaNavConfig->Get<bool>("bCreateOnClient") = true; // BITFIELD
    AthenaNavConfig->Get<bool>("bAutoSpawnMissingNavData") = true; // BITFIELD
    AthenaNavConfig->Get<bool>("bSpawnNavDataInNavBoundsLevel") = true; // BITFIELD
    AthenaNavConfig->Get<bool>("bUseNavigationInvokers") = false;
    AthenaNavConfig->Get<FName>("DefaultAgentName") = UKismetStringLibrary::Conv_StringToName(L"Galileo");

    // NavSystemOverride->Get<ENavSystemOverridePolicy>("OverridePolicy") = ENavSystemOverridePolicy::Append;
    NavSystemOverride->Get("NavigationSystemConfig") = AthenaNavConfig;

    SetNavigationSystem(NavSystemOverride);
}

static AFortPlayerPawn* SpawnAIFromCustomizationData(const FVector& Location, UObject* CustomizationData)
{
    static auto PawnClassOffset = CustomizationData->GetOffset("PawnClass");
    auto PawnClass = CustomizationData->Get<UClass*>(PawnClassOffset);

    if (!PawnClass)
    {
        LOG_INFO(LogAI, "Invalid PawnClass for AI!");
        return nullptr;
    }

    auto Pawn = GetWorld()->SpawnActor<AFortPlayerPawn>(PawnClass, Location);

    static auto CharacterCustomizationOffset = CustomizationData->GetOffset("CharacterCustomization");
    auto CharacterCustomization = CustomizationData->Get(CharacterCustomizationOffset);
    auto CharacterCustomizationLoadoutOffset = CharacterCustomization->GetOffset("CustomizationLoadout");
    auto CharacterCustomizationLoadout = CharacterCustomization->GetPtr<FFortAthenaLoadout>(CharacterCustomizationLoadoutOffset);

    ApplyCID(Pawn, CharacterCustomizationLoadout->GetCharacter());

    struct FItemAndCount
    {
        int                                                Count;                                                    // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
        unsigned char                                      UnknownData00[0x4];                                       // 0x0004(0x0004) MISSED OFFSET
        UFortItemDefinition* Item;                                                     // 0x0008(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    };

    static auto StartupInventoryOffset = CustomizationData->GetOffset("StartupInventory");
    auto StartupInventory = CustomizationData->Get(StartupInventoryOffset);
    static auto StartupInventoryItemsOffset = StartupInventory->GetOffset("Items");
    auto& StartupInventoryItems = StartupInventory->Get<TArray<FItemAndCount>>(StartupInventoryItemsOffset);

    auto Controller = Pawn->GetController();
    LOG_INFO(LogDev, "Controller: {} StartupInventoryItems.Num: {}", Controller ? Controller->GetFullName() : "InvalidObject", StartupInventoryItems.Num());

    if (Controller)
    {
        /* static auto InventoryOffset = Controller->GetOffset("Inventory");
        auto Inventory = Controller->Get<AFortInventory*>(InventoryOffset);

        for (int i = 0; i < StartupInventoryItems.Num(); i++)
        {
            auto pair = Inventory->AddItem(StartupInventoryItems.at(i).Item, nullptr, StartupInventoryItems.at(i).Count);

            LOG_INFO(LogDev, "pair.first.size(): {}", pair.first.size());

            if (pair.first.size() > 0)
            {
                if (auto weaponDef = Cast<UFortWeaponItemDefinition>(StartupInventoryItems.at(i).Item))
                    Pawn->EquipWeaponDefinition(weaponDef, pair.first.at(0)->GetItemEntry()->GetItemGuid());
            }
        } */

        // Inventory->Update(); // crashes idk why
    }

    return Pawn;
}