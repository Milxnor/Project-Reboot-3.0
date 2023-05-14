#pragma once

#include "reboot.h"
#include "Actor.h"
#include "SoftObjectPath.h"
#include "KismetStringLibrary.h"
#include "GameplayStatics.h"
#include "FortPlayerPawn.h"
#include "FortAthenaMutator.h"
#include "FortPlayerController.h"
#include "FortGameModeAthena.h"
#include "FortGameStateAthena.h"
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

static bool SetNavigationSystem(AAthenaNavSystemConfigOverride* NavSystemOverride)
{
    UAthenaNavSystem*& NavSystem = (UAthenaNavSystem*&)GetWorld()->Get("NavigationSystem");

    LOG_INFO(LogDev, "NavSystem: {}", NavSystem->IsValidLowLevel() ? NavSystem->GetFullName() : "BadRead");

    if (NavSystem)
    {
        return false;
        NavSystemCleanUpOriginal(NavSystem, 0);
        GetWorld()->Get("NavigationSystem") = nullptr;
    }

    auto WorldSettings = GetWorld()->GetWorldSettings();

    LOG_INFO(LogDev, "WorldSettings: {}", WorldSettings->IsValidLowLevel() ? WorldSettings->GetFullName() : "BadRead");

    if (!WorldSettings)
        return false;

    static auto OverridePolicyOffset = NavSystemOverride->GetOffset("OverridePolicy", false);
    
    if (OverridePolicyOffset != -1)
        NavSystemOverride->Get<ENavSystemOverridePolicy>(OverridePolicyOffset) = ENavSystemOverridePolicy::Append;
    
    static auto NavSystemOverride_NavigationSystemConfigOffset = NavSystemOverride->GetOffset("NavigationSystemConfig");

    WorldSettings->Get("NavigationSystemConfigOverride") = NavSystemOverride->Get(NavSystemOverride_NavigationSystemConfigOffset);

    LOG_INFO(LogDev, "WorldSettings_NavigationSystemConfig: {}", __int64(WorldSettings->Get("NavigationSystemConfig")));

    if (WorldSettings->Get("NavigationSystemConfig"))
        WorldSettings->Get("NavigationSystemConfig")->Get<bool>("bIsOverriden") = true;

    if (!NavSystemOverride->Get("NavigationSystemConfig"))
    {
        LOG_ERROR(LogAI, "No NavigationSystemConfig!");
        return false;
    }

    auto& ClassPath = NavSystemOverride->Get("NavigationSystemConfig")->Get<FSoftObjectPath>("NavigationSystemClass");

    auto NewNavSystemClass = FindObject<UClass>(ClassPath.AssetPathName.ToString());

    if (!NewNavSystemClass)
    {
        LOG_ERROR(LogAI, "No NavigationSystemClass!");
        return false;
    }

    LOG_INFO(LogAI, "Setup navigation system.");

    // if (Fortnite_Version >= 10.40) // idk when they added the fifth arg or does it even matter????
    {
        static void (*AddNavigationSystemToWorldOriginal)(UWorld * WorldOwner, EFNavigationSystemRunMode RunMode, UNavigationSystemConfig * NavigationSystemConfig, char bInitializeForWorld, char bOverridePreviousNavSys) =
            decltype(AddNavigationSystemToWorldOriginal)(Addresses::AddNavigationSystemToWorld);

        AddNavigationSystemToWorldOriginal(GetWorld(), EFNavigationSystemRunMode::GameMode, NavSystemOverride->Get("NavigationSystemConfig"), true, false);
    }

    return true;
}

static void SetupServerBotManager()
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

    static auto FortServerBotManagerClass = FindObject<UClass>(L"/Script/FortniteGame.FortServerBotManagerAthena"); // Is there a BP for this? // GameMode->ServerBotManagerClass

    if (!FortServerBotManagerClass)
        return;

    static auto ServerBotManagerOffset = GameMode->GetOffset("ServerBotManager");
    UObject*& ServerBotManager = GameMode->Get(ServerBotManagerOffset);

    if (!ServerBotManager)
        ServerBotManager = UGameplayStatics::SpawnObject(FortServerBotManagerClass, GetTransientPackage());

    if (ServerBotManager)
    {
        static auto CachedGameModeOffset = ServerBotManager->GetOffset("CachedGameMode");
        ServerBotManager->Get(CachedGameModeOffset) = GameMode;

        static auto CachedGameStateOffset = ServerBotManager->GetOffset("CachedGameState", false);

        if (CachedGameStateOffset != -1)
            ServerBotManager->Get(CachedGameStateOffset) = GameState;

        static auto CachedBotMutatorOffset = ServerBotManager->GetOffset("CachedBotMutator");
        ServerBotManager->Get(CachedBotMutatorOffset) = FindFirstMutator(FindObject<UClass>(L"/Script/FortniteGame.FortAthenaMutator_Bots"));
    }
}

static void SetupAIDirector()
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

    static auto AIDirectorClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaAIDirector"); // Probably wrong class

    if (!AIDirectorClass)
        return;

    static auto AIDirectorOffset = GameMode->GetOffset("AIDirector");
    
    if (!GameMode->Get(AIDirectorOffset))
        GameMode->Get(AIDirectorOffset) = GetWorld()->SpawnActor<AActor>(AIDirectorClass);

    if (GameMode->Get(AIDirectorOffset))
    {
        LOG_INFO(LogAI, "Successfully spawned AIDirector!");

        static auto ActivateFn = FindObject<UFunction>("/Script/FortniteGame.FortAIDirector.Activate");

        if (ActivateFn) // ?
            GameMode->Get(AIDirectorOffset)->ProcessEvent(ActivateFn); // ?
    }
}

static void SetupNavConfig(const FName& AgentName)
{
    static auto AthenaNavSystemConfigOverrideClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaNavSystemConfigOverride");
    auto NavSystemOverride = GetWorld()->SpawnActor<AActor>(AthenaNavSystemConfigOverrideClass);

    if (!NavSystemOverride)
        return;

    static auto AthenaNavSystemConfigClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaNavSystemConfig");
    auto AthenaNavConfig = (UAthenaNavSystemConfig*)UGameplayStatics::SpawnObject(AthenaNavSystemConfigClass, NavSystemOverride);
    AthenaNavConfig->Get<bool>("bUseBuildingGridAsNavigableSpace") = false;

    static auto bUsesStreamedInNavLevelOffset = AthenaNavConfig->GetOffset("bUsesStreamedInNavLevel", false);

    if (bUsesStreamedInNavLevelOffset != -1)
        AthenaNavConfig->Get<bool>(bUsesStreamedInNavLevelOffset) = true;

    AthenaNavConfig->Get<bool>("bAllowAutoRebuild") = true;
    AthenaNavConfig->Get<bool>("bCreateOnClient") = true; // BITFIELD
    AthenaNavConfig->Get<bool>("bAutoSpawnMissingNavData") = true; // BITFIELD
    AthenaNavConfig->Get<bool>("bSpawnNavDataInNavBoundsLevel") = true; // BITFIELD

    static auto bUseNavigationInvokersOffset = AthenaNavConfig->GetOffset("bUseNavigationInvokers", false);
    
    if (bUseNavigationInvokersOffset != -1)
        AthenaNavConfig->Get<bool>(bUseNavigationInvokersOffset) = false;

    static auto DefaultAgentNameOffset = AthenaNavConfig->GetOffset("DefaultAgentName", false);

    if (DefaultAgentNameOffset != -1)
        AthenaNavConfig->Get<FName>(DefaultAgentNameOffset) = AgentName;

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