#pragma once

#include "reboot.h"
#include "Actor.h"
#include "SoftObjectPath.h"
#include "KismetStringLibrary.h"
#include "GameplayStatics.h"

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

    LOG_INFO(LogAI, "Setup navigation system.\n");

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
    AthenaNavConfig->Get<FName>("DefaultAgentName") = UKismetStringLibrary::Conv_StringToName(L"MANG");

    // NavSystemOverride->Get<ENavSystemOverridePolicy>("OverridePolicy") = ENavSystemOverridePolicy::Append;
    NavSystemOverride->Get("NavigationSystemConfig") = AthenaNavConfig;

    SetNavigationSystem(NavSystemOverride);
}