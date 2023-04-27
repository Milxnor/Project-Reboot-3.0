#include <Windows.h>
#include <iostream>

#include "FortGameModeAthena.h"
#include "reboot.h"
#include "finder.h"
#include "hooking.h"
#include "GameSession.h"
#include "FortPlayerControllerAthena.h"
#include "AbilitySystemComponent.h"
#include "FortPlayerPawn.h"
#include "globals.h"
#include "FortInventoryInterface.h"
#include <fstream>
#include "GenericPlatformTime.h"
#include "FortAthenaMutator_GiveItemsAtGamePhaseStep.h"

#include "BuildingFoundation.h"
#include "Map.h"
#include "events.h"
#include "FortKismetLibrary.h"
#include "vehicles.h"
#include "UObjectArray.h"
#include "BuildingTrap.h"
#include "FortAthenaCreativePortal.h"
#include "commands.h"
#include "FortAthenaSupplyDrop.h"
#include "FortMinigame.h"
#include "KismetSystemLibrary.h"
#include "die.h"
#include "InventoryManagementLibrary.h"
#include "FortPlayerPawnAthena.h"
#include "FortWeaponRangedMountedCannon.h"
#include "gui.h"

#include "FortGameplayAbilityAthena_PeriodicItemGrant.h"
#include "vendingmachine.h"
#include "FortOctopusVehicle.h"
#include "FortVolumeManager.h"
#include "FortAthenaMutator_Barrier.h"

#include "PlaysetLevelStreamComponent.h"

enum ENetMode
{
    NM_Standalone,
    NM_DedicatedServer,
    NM_ListenServer,
    NM_Client,
    NM_MAX,
};

constexpr ENetMode NetMode = ENetMode::NM_DedicatedServer;

static ENetMode GetNetModeHook() { return NetMode; }
static ENetMode GetNetModeHook2() { return NetMode; }

static bool ReturnTrueHook() { return true; }
static int Return2Hook() { return 2; }

static bool NoMCPHook() { return Globals::bNoMCP; }
static void CollectGarbageHook() { return; }

static __int64 (*DispatchRequestOriginal)(__int64 a1, __int64* a2, int a3);

static __int64 DispatchRequestHook(__int64 a1, __int64* a2, int a3)
{
    if (Globals::bNoMCP)
        return DispatchRequestOriginal(a1, a2, a3);

    if (Engine_Version >= 423)
        return DispatchRequestOriginal(a1, a2, 3); 

    // LOG_INFO(LogDev, "Dispatch Request!");

    static auto Offset = FindMcpIsDedicatedServerOffset();

    *(int*)(__int64(a2) + Offset) = 3;

    return DispatchRequestOriginal(a1, a2, 3);
}

void (*ApplyHomebaseEffectsOnPlayerSetupOriginal)(
    __int64* GameState,
    __int64 a2,
    __int64 a3,
    __int64 a4,
    UObject* Hero,
    char a6,
    unsigned __int8 a7);

void __fastcall ApplyHomebaseEffectsOnPlayerSetupHook(
    __int64* GameState,
    __int64 a2,
    __int64 a3,
    __int64 a4,
    UObject* Hero,
    char a6,
    unsigned __int8 a7)
{
    LOG_INFO(LogDev, "Old hero: {}", Hero ? Hero->GetFullName() : "InvalidObject");

    UFortItemDefinition* HeroType = FindObject<UFortItemDefinition>("/Game/Athena/Heroes/HID_030_Athena_Commando_M_Halloween.HID_030_Athena_Commando_M_Halloween");

    if (Fortnite_Version == 1.72)
    {
        auto AllHeroTypes = GetAllObjectsOfClass(FindObject<UClass>("/Script/FortniteGame.FortHeroType"));
        std::vector<UFortItemDefinition*> AthenaHeroTypes;

        for (int i = 0; i < AllHeroTypes.size(); i++)
        {
            auto CurrentHeroType = (UFortItemDefinition*)AllHeroTypes.at(i);

            if (CurrentHeroType->GetPathName().starts_with("/Game/Athena/Heroes/"))
                AthenaHeroTypes.push_back(CurrentHeroType);
        }

        if (AthenaHeroTypes.size())
        {
            HeroType = AthenaHeroTypes.at(std::rand() % AthenaHeroTypes.size());
        }
    }

    static auto ItemDefinitionOffset = Hero->GetOffset("ItemDefinition");
    Hero->Get<UFortItemDefinition*>(ItemDefinitionOffset) = HeroType;

    return ApplyHomebaseEffectsOnPlayerSetupOriginal(GameState, a2, a3, a4, Hero, a6, a7);
}

DWORD WINAPI Main(LPVOID)
{
    InitLogger();

    std::cin.tie(0);
    std::cout.tie(0);
    std::ios_base::sync_with_stdio(false);

    auto MH_InitCode = MH_Initialize();

    if (MH_InitCode != MH_OK)
    {
        LOG_ERROR(LogInit, "Failed to initialize MinHook {}!", MH_StatusToString(MH_InitCode));
        return 1;
    }

    LOG_INFO(LogInit, "Initializing Project Reboot!");

    Addresses::SetupVersion();

    Offsets::FindAll(); // We have to do this before because FindCantBuild uses FortAIController.CreateBuildingActor
    Offsets::Print();

    Addresses::FindAll();
    // Addresses::Print();
    Addresses::Init();
    Addresses::Print();

    LOG_INFO(LogDev, "Fortnite_CL: {}", Fortnite_CL);
    LOG_INFO(LogDev, "Version: {}", Fortnite_Version);

    CreateThread(0, 0, GuiThread, 0, 0, 0);

    while (SecondsUntilTravel > 0)
    {
        SecondsUntilTravel -= 1;

        Sleep(1000);
    }

    bSwitchedInitialLevel = true;

    static auto GameModeDefault = FindObject<AFortGameModeAthena>(L"/Script/FortniteGame.Default__FortGameModeAthena");
    static auto FortPlayerControllerZoneDefault = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerControllerZone");
    static auto FortPlayerControllerAthenaDefault = FindObject<AFortPlayerControllerAthena>(L"/Script/FortniteGame.Default__FortPlayerControllerAthena"); // FindObject<UClass>(L"/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");
    static auto FortPlayerPawnAthenaDefault = FindObject<AFortPlayerPawn>(L"/Script/FortniteGame.Default__FortPlayerPawnAthena"); // FindObject<AFortPlayerPawn>(L"/Game/Athena/PlayerPawn_Athena.Default__PlayerPawn_Athena_C");
    static auto FortAbilitySystemComponentAthenaDefault = FindObject<UObject>(L"/Script/FortniteGame.Default__FortAbilitySystemComponentAthena");
    static auto FortPlayerStateAthenaDefault = FindObject<AFortPlayerStateAthena>(L"/Script/FortniteGame.Default__FortPlayerStateAthena");
    static auto FortKismetLibraryDefault = FindObject<UFortKismetLibrary>(L"/Script/FortniteGame.Default__FortKismetLibrary");
    static auto AthenaMarkerComponentDefault = FindObject<UAthenaMarkerComponent>(L"/Script/FortniteGame.Default__AthenaMarkerComponent");
    static auto FortWeaponDefault = FindObject<AFortWeapon>(L"/Script/FortniteGame.Default__FortWeapon");
    static auto FortOctopusVehicleDefault = FindObject<AFortOctopusVehicle>("/Script/FortniteGame.Default__FortOctopusVehicle");

    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNetPackageMap VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNetTraffic VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNet VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogBuilding VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortQuest VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector NoLogging", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAbilitySystem VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogDataTable VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFort VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogGameMode VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortCustomization VeryVerbose", nullptr);

    Hooking::MinHook::Hook((PVOID)Addresses::NoMCP, (PVOID)NoMCPHook, nullptr);
    Hooking::MinHook::Hook((PVOID)Addresses::GetNetMode, (PVOID)GetNetModeHook, nullptr);
    Hooking::MinHook::Hook((PVOID)Addresses::DispatchRequest, (PVOID)DispatchRequestHook, (PVOID*)&DispatchRequestOriginal);

    GSRandSeed = FGenericPlatformTime::Cycles();
    ReplicationRandStream = FRandomStream(FGenericPlatformTime::Cycles());

    Hooking::MinHook::Hook((PVOID)Addresses::KickPlayer, (PVOID)AGameSession::KickPlayerHook, (PVOID*)&AGameSession::KickPlayerOriginal);

    LOG_INFO(LogDev, "Built on {} {}", __DATE__, __TIME__);
    LOG_INFO(LogDev, "[bNoMCP] {}", Globals::bNoMCP);
    LOG_INFO(LogDev, "[bGoingToPlayEvent] {}", Globals::bGoingToPlayEvent);
    LOG_INFO(LogDev, "Size: 0x{:x}", sizeof(TMap<FName, void*>));

    Hooking::MinHook::Hook((PVOID)Addresses::ActorGetNetMode, (PVOID)GetNetModeHook2, nullptr);

    LOG_INFO(LogDev, "FindGIsServer: 0x{:x}", FindGIsServer() - __int64(GetModuleHandleW(0)));
    LOG_INFO(LogDev, "FindGIsClient: 0x{:x}", FindGIsClient() - __int64(GetModuleHandleW(0)));

    bool bUseRemovePlayer = false;
    bool bUseSwitchLevel = false;

    Hooking::MinHook::Hook(FindObject<ABuildingFoundation>("/Script/FortniteGame.Default__BuildingFoundation"),
        FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationTransform"),
        ABuildingFoundation::SetDynamicFoundationTransformHook, (PVOID*)&ABuildingFoundation::SetDynamicFoundationTransformOriginal, false, true);

    Hooking::MinHook::Hook(FindObject<ABuildingFoundation>("/Script/FortniteGame.Default__BuildingFoundation"),
        FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationEnabled"),
        ABuildingFoundation::SetDynamicFoundationEnabledHook, (PVOID*)&ABuildingFoundation::SetDynamicFoundationEnabledOriginal, false, true);

    if (bUseSwitchLevel)
    {
        static auto SwitchLevel = FindObject<UFunction>(L"/Script/Engine.PlayerController.SwitchLevel");
        FString Level = Engine_Version < 424
            ? L"Athena_Terrain" : Engine_Version >= 500 ? Engine_Version >= 501
            ? L"Asteria_Terrain"
            : Globals::bCreative ? L"Creative_NoApollo_Terrain"
            : L"Artemis_Terrain"
            : Globals::bCreative ? L"Creative_NoApollo_Terrain"
            : L"Apollo_Terrain";

        GetLocalPlayerController()->ProcessEvent(SwitchLevel, &Level);

        if (FindGIsServer())
            *(bool*)FindGIsServer() = true;

        if (FindGIsClient())
            *(bool*)FindGIsClient() = false;
    }
    else
    {
        if (FindGIsServer())
            *(bool*)FindGIsServer() = true;

        if (FindGIsClient())
            *(bool*)FindGIsClient() = false;

        if (!bUseRemovePlayer)
        {
            auto& LocalPlayers = GetLocalPlayers();

            if (LocalPlayers.Num() && LocalPlayers.Data)
            {
                LocalPlayers.Remove(0);
            }
        }
        else if (bUseRemovePlayer)
        {
            UGameplayStatics::RemovePlayer((APlayerController*)GetLocalPlayerController(), true);
        }

        FString LevelB = Engine_Version < 424
            ? L"open Athena_Terrain" : Engine_Version >= 500 ? Engine_Version >= 501
            ? L"open Asteria_Terrain"
            : Globals::bCreative ? L"open Creative_NoApollo_Terrain"
            : L"open Artemis_Terrain"
            : Globals::bCreative ? L"open Creative_NoApollo_Terrain"
            : L"open Apollo_Terrain";

        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), LevelB, nullptr);
    }

    LOG_INFO(LogPlayer, "Switched level.");

    if (Fortnite_Version == 17.30)
    {
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E07910), (PVOID)Return2Hook, nullptr);
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DED12C), (PVOID)ReturnTrueHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DED158), (PVOID)ReturnTrueHook, nullptr);
    }

    if (bUseSwitchLevel)
    {
        if (!bUseRemovePlayer)
        {
            auto& LocalPlayers = GetLocalPlayers();

            if (LocalPlayers.Num() && LocalPlayers.Data)
            {
                LocalPlayers.Remove(0);
            }
        }
        else if (bUseRemovePlayer)
        {
            UGameplayStatics::RemovePlayer((APlayerController*)GetLocalPlayerController(), true);
        }
    }

    auto AddressesToNull = Addresses::GetFunctionsToNull();

    auto ServerCheatAllIndex = GetFunctionIdxOrPtr(FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.ServerCheatAll"));

    if (ServerCheatAllIndex)
        AddressesToNull.push_back(__int64(FortPlayerControllerAthenaDefault->VFTable[ServerCheatAllIndex / 8]));

    for (auto func : AddressesToNull)
    {
        if (func == 0)
            continue;

        LOG_INFO(LogDev, "Nulling 0x{:x}", func - __int64(GetModuleHandleW(0)));

        DWORD dwProtection;
        VirtualProtect((PVOID)func, 1, PAGE_EXECUTE_READWRITE, &dwProtection);

        *(uint8_t*)func = 0xC3;

        DWORD dwTemp;
        VirtualProtect((PVOID)func, 1, dwProtection, &dwTemp);
    }

    if (Fortnite_Version != 22.4)
    {
        auto matchmaking = Memcury::Scanner::FindPattern("83 BD ? ? ? ? 01 7F 18 49 8D 4D D8 48 8B D6 E8 ? ? ? ? 48").Get();

        matchmaking = matchmaking ? matchmaking : Memcury::Scanner::FindPattern("83 7D 88 01 7F 0D 48 8B CE E8").Get();

        bool bMatchmakingSupported = false;

        bMatchmakingSupported = matchmaking && Engine_Version >= 420;
        int idx = 0;

        if (bMatchmakingSupported) // now check if it leads to the right place and where the jg is at
        {
            for (int i = 0; i < 9; i++)
            {
                auto byte = (uint8_t*)(matchmaking + i);

                if (IsBadReadPtr(byte))
                    continue;

                // std::cout << std::format("[{}] 0x{:x}\n", i, (int)*byte);

                if (*byte == 0x7F)
                {
                    bMatchmakingSupported = true;
                    idx = i;
                    break;
                }

                bMatchmakingSupported = false;
            }
        }

        std::cout << "Matchmaking will " << (bMatchmakingSupported ? "be supported\n" : "not be supported\n");

        if (bMatchmakingSupported)
        {
            std::cout << "idx: " << idx << '\n';

            auto before = (uint8_t*)(matchmaking + idx);

            std::cout << "before byte: " << (int)*before << '\n';

            *before = 0x74;
        }
    }

    // return false;

    // UNetDriver::ReplicationDriverOffset = FindOffsetStruct("/Script/Engine.NetDriver", "ReplicationDriver"); // NetDriver->GetOffset("ReplicationDriver");

    // Globals::bAbilitiesEnabled = Engine_Version < 500;

    if (Engine_Version < 420)
    {
        auto ApplyHomebaseEffectsOnPlayerSetupAddr = Memcury::Scanner::FindPattern("40 55 53 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 00 4C 8B").Get();

        Hooking::MinHook::Hook((PVOID)ApplyHomebaseEffectsOnPlayerSetupAddr, ApplyHomebaseEffectsOnPlayerSetupHook, (PVOID*)&ApplyHomebaseEffectsOnPlayerSetupOriginal);
    }

    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameMode.ReadyToStartMatch"), AFortGameModeAthena::Athena_ReadyToStartMatchHook,
       (PVOID*)&AFortGameModeAthena::Athena_ReadyToStartMatchOriginal, false, false, true);

    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnFor"),
        AGameModeBase::SpawnDefaultPawnForHook, nullptr, false);
    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameModeBase.HandleStartingNewPlayer"), AFortGameModeAthena::Athena_HandleStartingNewPlayerHook,
        (PVOID*)&AFortGameModeAthena::Athena_HandleStartingNewPlayerOriginal, false);

    static auto ControllerServerAttemptInteractFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.ServerAttemptInteract");

    if (ControllerServerAttemptInteractFn)
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, ControllerServerAttemptInteractFn, AFortPlayerController::ServerAttemptInteractHook,
            (PVOID*)&AFortPlayerController::ServerAttemptInteractOriginal, false, true);
    }
    else
    {
        Hooking::MinHook::Hook(FindObject("/Script/FortniteGame.Default__FortControllerComponent_Interaction"),
            FindObject<UFunction>("/Script/FortniteGame.FortControllerComponent_Interaction.ServerAttemptInteract"),
            AFortPlayerController::ServerAttemptInteractHook, (PVOID*)&AFortPlayerController::ServerAttemptInteractOriginal, false, true);
    }

    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/Engine.PlayerController.ServerAcknowledgePossession"),
        AFortPlayerControllerAthena::ServerAcknowledgePossessionHook, nullptr, false);

    if (Engine_Version >= 424)
    {
        static auto ServerRestartPlayerFn = FindObject<UFunction>(L"/Script/Engine.PlayerController.ServerRestartPlayer");
        auto ZoneServerRestartPlayer = FortPlayerControllerZoneDefault->VFTable[GetFunctionIdxOrPtr(ServerRestartPlayerFn) / 8];

        LOG_INFO(LogDev, "ZoneServerRestartPlayer: 0x{:x}", __int64(ZoneServerRestartPlayer) - __int64(GetModuleHandleW(0)));

        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, ServerRestartPlayerFn,
            // ZoneServerRestartPlayer,
            AFortPlayerControllerAthena::ServerRestartPlayerHook,
            nullptr, false);
    }

    Hooking::MinHook::Hook(FortWeaponDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortWeapon.ServerReleaseWeaponAbility"),
        AFortWeapon::ServerReleaseWeaponAbilityHook, (PVOID*)&AFortWeapon::ServerReleaseWeaponAbilityOriginal, false, true);

    auto OnPlayImpactFXStringRef = Memcury::Scanner::FindStringRef(L"OnPlayImpactFX", true, 0);
    __int64 OnPlayImpactFXAddr = 0;

    if (OnPlayImpactFXStringRef.Get())
    {
        auto OnPlayImpactFXFunctionPtr = OnPlayImpactFXStringRef.ScanFor({ 0x48, 0x8D, 0x0D }).RelativeOffset(3).GetAs<void*>();
        auto OnPlayImpactFXPtrRef = Memcury::Scanner::FindPointerRef(OnPlayImpactFXFunctionPtr).Get();

        for (int i = 0; i < 2000; i++)
        {
            if (*(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i) == 0x48 && *(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i + 2) == 0x5C)
            {
                OnPlayImpactFXAddr = OnPlayImpactFXPtrRef - i;
                break;
            }

            if (*(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i) == 0x4C && *(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i + 1) == 0x8B && *(uint8_t*)(uint8_t*)(OnPlayImpactFXPtrRef - i + 2) == 0xDC)
            {
                OnPlayImpactFXAddr = OnPlayImpactFXPtrRef - i;
                break;
            }
        }
    }

    LOG_INFO(LogDev, "OnPlayImpactFX: 0x{:x}", OnPlayImpactFXAddr - __int64(GetModuleHandleW(0)));
    Hooking::MinHook::Hook((PVOID)OnPlayImpactFXAddr, AFortWeapon::OnPlayImpactFXHook, (PVOID*)&AFortWeapon::OnPlayImpactFXOriginal);

    /* Hooking::MinHook::Hook(FindObject<AFortVolumeManager>("/Script/FortniteGame.Default__FortVolumeManager"), FindObject<UFunction>(L"/Script/FortniteGame.FortVolumeManager.SpawnVolume"),
        AFortVolumeManager::SpawnVolumeHook, (PVOID*)&AFortVolumeManager::SpawnVolumeOriginal, false);
    Hooking::MinHook::Hook((PVOID)GetFunctionIdxOrPtr(FindObject<UFunction>("/Script/FortniteGame.PlaysetLevelStreamComponent.SetPlayset"), true), 
        UPlaysetLevelStreamComponent::SetPlaysetHook, (PVOID*)&UPlaysetLevelStreamComponent::SetPlaysetOriginal); */

    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerDropAllItems"),
        AFortPlayerController::ServerDropAllItemsHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault,
        FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerSpawnInventoryDrop") 
        ? FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerSpawnInventoryDrop") : FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerAttemptInventoryDrop"),
        AFortPlayerController::ServerAttemptInventoryDropHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerCheat"),
        ServerCheatHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerExecuteInventoryItem"),
       AFortPlayerController::ServerExecuteInventoryItemHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerPlayEmoteItem"),
       AFortPlayerController::ServerPlayEmoteItemHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerRepairBuildingActor"),
        AFortPlayerController::ServerRepairBuildingActorHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerCreateBuildingActor"), 
        AFortPlayerController::ServerCreateBuildingActorHook, (PVOID*)&AFortPlayerController::ServerCreateBuildingActorOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerBeginEditingBuildingActor"),
        AFortPlayerController::ServerBeginEditingBuildingActorHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerEditBuildingActor"),
        AFortPlayerController::ServerEditBuildingActorHook, (PVOID*)&AFortPlayerController::ServerEditBuildingActorOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerEndEditingBuildingActor"),
        AFortPlayerController::ServerEndEditingBuildingActorHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerLoadingScreenDropped"),
        AFortPlayerController::ServerLoadingScreenDroppedHook, (PVOID*)&AFortPlayerController::ServerLoadingScreenDroppedOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerReadyToStartMatch"),
        AFortPlayerControllerAthena::ServerReadyToStartMatchHook, (PVOID*)&AFortPlayerControllerAthena::ServerReadyToStartMatchOriginal, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange"),
        AFortPlayerControllerAthena::ServerRequestSeatChangeHook, (PVOID*)&AFortPlayerControllerAthena::ServerRequestSeatChangeOriginal, false);
    if (false)
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerGameplay.StartGhostMode"), // (Milxnor) TODO: This changes to a component in later seasons.
            AFortPlayerControllerAthena::StartGhostModeHook, (PVOID*)&AFortPlayerControllerAthena::StartGhostModeOriginal, false, true);
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerGameplay.EndGhostMode"),
            AFortPlayerControllerAthena::EndGhostModeHook, (PVOID*)&AFortPlayerControllerAthena::EndGhostModeOriginal, false);
    }
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem"),
        AFortPlayerControllerAthena::ServerGiveCreativeItemHook, nullptr, true);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerPlaySquadQuickChatMessage"),
        AFortPlayerControllerAthena::ServerPlaySquadQuickChatMessageHook, nullptr, false);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland"),
        AFortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIslandHook, nullptr, false);

    // Hooking::MinHook::Hook(FortPlayerStateAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.ServerSetInAircraft"),
        // AFortPlayerStateAthena::ServerSetInAircraftHook, (PVOID*)&AFortPlayerStateAthena::ServerSetInAircraftOriginal, false, true); // We could use second method but eh

    if (false && FortOctopusVehicleDefault) // hooking broken on 19.10 i cant figure it out for the life of me
    {
        static auto ServerUpdateTowhookFn = FindObject<UFunction>("/Script/FortniteGame.FortOctopusVehicle.ServerUpdateTowhook");
        Hooking::MinHook::Hook(FortOctopusVehicleDefault, ServerUpdateTowhookFn, AFortOctopusVehicle::ServerUpdateTowhookHook, nullptr, false);
    }

    Hooking::MinHook::Hook(FindObject<AFortWeaponRangedMountedCannon>(L"/Script/FortniteGame.Default__FortWeaponRangedMountedCannon"),
        FindObject<UFunction>("/Script/FortniteGame.FortWeaponRangedMountedCannon.ServerFireActorInCannon"), AFortWeaponRangedMountedCannon::ServerFireActorInCannonHook, nullptr, false);

    static auto NetMulticast_Athena_BatchedDamageCuesFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues") : FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.NetMulticast_Athena_BatchedDamageCues");

    Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState"),
        AFortPlayerPawn::ServerSendZiplineStateHook, nullptr, false);
    Hooking::MinHook::Hook((PVOID)GetFunctionIdxOrPtr(FindObject<UFunction>("/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle"), true), AFortPlayerPawn::ServerOnExitVehicleHook, (PVOID*)&AFortPlayerPawn::ServerOnExitVehicleOriginal);

    bool bNativeHookRemoveFortItemFromPlayer = false;

    if (bNativeHookRemoveFortItemFromPlayer)
    {
        Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveFortItemFromPlayer"),
            UFortKismetLibrary::K2_RemoveFortItemFromPlayerHook1, nullptr, false);
    }

    static auto FortGameplayAbilityAthena_PeriodicItemGrantDefault = FindObject<UFortGameplayAbilityAthena_PeriodicItemGrant>("/Script/FortniteGame.Default__FortGameplayAbilityAthena_PeriodicItemGrant");

    Hooking::MinHook::Hook(FortGameplayAbilityAthena_PeriodicItemGrantDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant.StartItemAwardTimers"),
        UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersHook, (PVOID*)&UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersOriginal, false, true);

    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_Barrier>(L"/Script/FortniteGame.Default__FortAthenaMutator_Barrier"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_Barrier.OnGamePhaseStepChanged"),
        AFortAthenaMutator_Barrier::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_Barrier::OnGamePhaseStepChangedOriginal, false, true);
    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_Disco>(L"/Script/FortniteGame.Default__FortAthenaMutator_Disco"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_Disco.OnGamePhaseStepChanged"),
        AFortAthenaMutator_Disco::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_Disco::OnGamePhaseStepChangedOriginal, false, true);
    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_GiveItemsAtGamePhaseStep>(L"/Script/FortniteGame.Default__FortAthenaMutator_GiveItemsAtGamePhaseStep"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_GiveItemsAtGamePhaseStep.OnGamePhaseStepChanged"),
        AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChangedOriginal, false, true);

    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_GiveItemToPlayer"),
        UFortKismetLibrary::K2_GiveItemToPlayerHook, (PVOID*)&UFortKismetLibrary::K2_GiveItemToPlayerOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_GiveBuildingResource"),
        UFortKismetLibrary::K2_GiveBuildingResourceHook, (PVOID*)&UFortKismetLibrary::K2_GiveBuildingResourceOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner"),
        UFortKismetLibrary::GiveItemToInventoryOwnerHook, (PVOID*)&UFortKismetLibrary::GiveItemToInventoryOwnerOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayerByGuid"),
        UFortKismetLibrary::K2_RemoveItemFromPlayerByGuidHook, (PVOID*)&UFortKismetLibrary::K2_RemoveItemFromPlayerByGuidOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer"),
        UFortKismetLibrary::K2_RemoveItemFromPlayerHook, (PVOID*)&UFortKismetLibrary::K2_RemoveItemFromPlayerOriginal, false, true);

    Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, NetMulticast_Athena_BatchedDamageCuesFn,
        AFortPawn::NetMulticast_Athena_BatchedDamageCuesHook, (PVOID*)&AFortPawn::NetMulticast_Athena_BatchedDamageCuesOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.MovingEmoteStopped"),
        AFortPawn::MovingEmoteStoppedHook, (PVOID*)&AFortPawn::MovingEmoteStoppedOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.OnCapsuleBeginOverlap") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.OnCapsuleBeginOverlap") : FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.OnCapsuleBeginOverlap"),
        AFortPlayerPawnAthena::OnCapsuleBeginOverlapHook, (PVOID*)&AFortPlayerPawnAthena::OnCapsuleBeginOverlapOriginal, false, true);

    if (!bNativeHookRemoveFortItemFromPlayer)
    {
        Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveFortItemFromPlayer"),
            UFortKismetLibrary::K2_RemoveFortItemFromPlayerHook, (PVOID*)&UFortKismetLibrary::K2_RemoveFortItemFromPlayerOriginal, false, true);
    }

    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorld"),
        UFortKismetLibrary::K2_SpawnPickupInWorldHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithLootTier"),
        UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithClass"),
        UFortKismetLibrary::K2_SpawnPickupInWorldWithClassHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldWithClassOriginal, false, true);

    if (Addresses::FreeArrayOfEntries || Addresses::FreeEntry)
    {
        Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.PickLootDrops"),
            UFortKismetLibrary::PickLootDropsHook, (PVOID*)&UFortKismetLibrary::PickLootDropsOriginal, false, true);
    }

    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.CreateTossAmmoPickupForWeaponItemDefinitionAtLocation"),
        UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocationHook, (PVOID*)&UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.SpawnInstancedPickupInWorld"),
        UFortKismetLibrary::SpawnInstancedPickupInWorldHook, (PVOID*)&UFortKismetLibrary::SpawnInstancedPickupInWorldOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.SpawnItemVariantPickupInWorld"),
        UFortKismetLibrary::SpawnItemVariantPickupInWorldHook, (PVOID*)&UFortKismetLibrary::SpawnItemVariantPickupInWorldOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.PickLootDropsWithNamedWeights"),
        UFortKismetLibrary::PickLootDropsWithNamedWeightsHook, (PVOID*)&UFortKismetLibrary::PickLootDropsWithNamedWeightsOriginal, false, true);

    // TODO Add RemoveItemFromInventoryOwner

    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.SpawnToyInstance"),
        AFortPlayerController::SpawnToyInstanceHook, (PVOID*)&AFortPlayerController::SpawnToyInstanceOriginal, false, true);
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.DropSpecificItem"),
        AFortPlayerController::DropSpecificItemHook, (PVOID*)&AFortPlayerController::DropSpecificItemOriginal, false, true);

    static auto FortAthenaSupplyDropDefault = FindObject(L"/Script/FortniteGame.Default__FortAthenaSupplyDrop");

    Hooking::MinHook::Hook(FortAthenaSupplyDropDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaSupplyDrop.SpawnPickup"),
        AFortAthenaSupplyDrop::SpawnPickupHook, (PVOID*)&AFortAthenaSupplyDrop::SpawnPickupOriginal, false, true);
    Hooking::MinHook::Hook(FortAthenaSupplyDropDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaSupplyDrop.SpawnGameModePickup"),
        AFortAthenaSupplyDrop::SpawnGameModePickupHook, (PVOID*)&AFortAthenaSupplyDrop::SpawnGameModePickupOriginal, false, true);
    Hooking::MinHook::Hook(FortAthenaSupplyDropDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaSupplyDrop.SpawnPickupFromItemEntry"),
        AFortAthenaSupplyDrop::SpawnPickupFromItemEntryHook, (PVOID*)&AFortAthenaSupplyDrop::SpawnPickupFromItemEntryOriginal, false, true);

    static auto FortAthenaCreativePortalDefault = FindObject(L"/Script/FortniteGame.Default__FortAthenaCreativePortal");

    Hooking::MinHook::Hook(FortAthenaCreativePortalDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume"),
        AFortAthenaCreativePortal::TeleportPlayerToLinkedVolumeHook, (PVOID*)&AFortAthenaCreativePortal::TeleportPlayerToLinkedVolumeOriginal, false, true);
    Hooking::MinHook::Hook(FortAthenaCreativePortalDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaCreativePortal.TeleportPlayer"),
        AFortAthenaCreativePortal::TeleportPlayerHook, (PVOID*)&AFortAthenaCreativePortal::TeleportPlayerOriginal, false, true);

    static auto FortMinigameDefault = FindObject(L"/Script/FortniteGame.Default__FortMinigame");

    Hooking::MinHook::Hook(FortMinigameDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortMinigame.ClearPlayerInventory"),
        AFortMinigame::ClearPlayerInventoryHook, (PVOID*)&AFortMinigame::ClearPlayerInventoryOriginal, false, true);

    static auto InventoryManagementLibraryDefault = FindObject<UInventoryManagementLibrary>(L"/Script/FortniteGame.Default__InventoryManagementLibrary");

    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.AddItem"),
        UInventoryManagementLibrary::AddItemHook, (PVOID*)&UInventoryManagementLibrary::AddItemOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.AddItems"),
        UInventoryManagementLibrary::AddItemsHook, (PVOID*)&UInventoryManagementLibrary::AddItemsOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.GiveItemEntryToInventoryOwner"),
        UInventoryManagementLibrary::GiveItemEntryToInventoryOwnerHook, (PVOID*)&UInventoryManagementLibrary::GiveItemEntryToInventoryOwnerOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.RemoveItem"),
        UInventoryManagementLibrary::RemoveItemHook, (PVOID*)&UInventoryManagementLibrary::RemoveItemOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.RemoveItems"),
        UInventoryManagementLibrary::RemoveItemsHook, (PVOID*)&UInventoryManagementLibrary::RemoveItemsOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.SwapItem"),
        UInventoryManagementLibrary::SwapItemHook, (PVOID*)&UInventoryManagementLibrary::SwapItemOriginal, false, true);
    Hooking::MinHook::Hook(InventoryManagementLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.InventoryManagementLibrary.SwapItems"),
        UInventoryManagementLibrary::SwapItemsHook, (PVOID*)&UInventoryManagementLibrary::SwapItemsOriginal, false, true);

    static auto ServerHandlePickupInfoFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupInfo");

    if (ServerHandlePickupInfoFn)
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, ServerHandlePickupInfoFn, AFortPlayerPawn::ServerHandlePickupInfoHook, nullptr, false);
    }
    else
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup"),
            AFortPlayerPawn::ServerHandlePickupHook, nullptr, false);
    }

    static auto PredictionKeyStruct = FindObject<UStruct>(L"/Script/GameplayAbilities.PredictionKey");
    static auto PredictionKeySize = PredictionKeyStruct->GetPropertiesSize();

    {
        int InternalServerTryActivateAbilityIndex = 0;

        if (Engine_Version > 420)
        {
            static auto OnRep_ReplicatedAnimMontageFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.OnRep_ReplicatedAnimMontage");
            InternalServerTryActivateAbilityIndex = (GetFunctionIdxOrPtr(OnRep_ReplicatedAnimMontageFn) - 8) / 8;
        }
        else
        {
            static auto ServerTryActivateAbilityWithEventDataFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ServerTryActivateAbilityWithEventData");
            auto ServerTryActivateAbilityWithEventDataNativeAddr = __int64(FortAbilitySystemComponentAthenaDefault->VFTable[GetFunctionIdxOrPtr(ServerTryActivateAbilityWithEventDataFn) / 8]);

            for (int i = 0; i < 400; i++)
            {
                if ((*(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i) == 0xFF && *(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i + 1) == 0x90) || // call qword ptr
                    (*(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i) == 0xFF && *(uint8_t*)(ServerTryActivateAbilityWithEventDataNativeAddr + i + 1) == 0x93)) // call qword ptr
                {
                    InternalServerTryActivateAbilityIndex = GetIndexFromVirtualFunctionCall(ServerTryActivateAbilityWithEventDataNativeAddr + i) / 8;
                    break;
                }
            }
        }

        LOG_INFO(LogDev, "InternalServerTryActivateAbilityIndex: 0x{:x}", InternalServerTryActivateAbilityIndex);

        VirtualSwap(FortAbilitySystemComponentAthenaDefault->VFTable, InternalServerTryActivateAbilityIndex, UAbilitySystemComponent::InternalServerTryActivateAbilityHook);
    }

    // if (Engine_Version >= 424)
    {
        static auto FortControllerComponent_AircraftDefault = FindObject<AActor>(L"/Script/FortniteGame.Default__FortControllerComponent_Aircraft");
        static auto ServerAttemptAircraftJumpFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerAttemptAircraftJump")
            : FindObject<UFunction>(L"/Script/FortniteGame.FortControllerComponent_Aircraft.ServerAttemptAircraftJump");

        Hooking::MinHook::Hook(FortControllerComponent_AircraftDefault ? FortControllerComponent_AircraftDefault : FortPlayerControllerAthenaDefault, ServerAttemptAircraftJumpFn,
            AFortPlayerController::ServerAttemptAircraftJumpHook, (PVOID*)&AFortPlayerController::ServerAttemptAircraftJumpOriginal, false);
    }

    // if (false)
    {
        if (Fortnite_Version >= 8.3 && Engine_Version < 424) // I can't remember, so ServerAddMapMarker existed on like 8.0 or 8.1 or 8.2 but it didn't have the same params.
        {
            Hooking::MinHook::Hook(AthenaMarkerComponentDefault, FindObject<UFunction>(L"/Script/FortniteGame.AthenaMarkerComponent.ServerAddMapMarker"),
                UAthenaMarkerComponent::ServerAddMapMarkerHook, nullptr, false);
            Hooking::MinHook::Hook(AthenaMarkerComponentDefault, FindObject<UFunction>(L"/Script/FortniteGame.AthenaMarkerComponent.ServerRemoveMapMarker"),
                UAthenaMarkerComponent::ServerRemoveMapMarkerHook, nullptr, false);
        }
    }

    Hooking::MinHook::Hook((PVOID)Addresses::GetPlayerViewpoint, (PVOID)AFortPlayerControllerAthena::GetPlayerViewPointHook, (PVOID*)&AFortPlayerControllerAthena::GetPlayerViewPointOriginal);

    Hooking::MinHook::Hook((PVOID)Addresses::TickFlush, (PVOID)UNetDriver::TickFlushHook, (PVOID*)&UNetDriver::TickFlushOriginal);
    Hooking::MinHook::Hook((PVOID)Addresses::OnDamageServer, (PVOID)ABuildingActor::OnDamageServerHook, (PVOID*)&ABuildingActor::OnDamageServerOriginal);
    
    Hooking::MinHook::Hook((PVOID)Addresses::GetMaxTickRate, GetMaxTickRateHook);
    // Hooking::MinHook::Hook((PVOID)Addresses::CollectGarbage, (PVOID)CollectGarbageHook, nullptr);
    Hooking::MinHook::Hook((PVOID)Addresses::PickTeam, (PVOID)AFortGameModeAthena::Athena_PickTeamHook);
    Hooking::MinHook::Hook((PVOID)Addresses::CompletePickupAnimation, (PVOID)AFortPickup::CompletePickupAnimationHook, (PVOID*)&AFortPickup::CompletePickupAnimationOriginal);
    Hooking::MinHook::Hook((PVOID)Addresses::CanActivateAbility, ReturnTrueHook); // ahhh wtf

    // if (Fortnite_Version >= 2.5)
    if (Engine_Version >= 419)
    {
        auto ServerRemoveInventoryItemFunctionCallRef = Memcury::Scanner::FindPointerRef((PVOID)FindFunctionCall(L"ServerRemoveInventoryItem",
            Fortnite_Version >= 16 ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C }), 0, true);

        LOG_INFO(LogDev, "ServerRemoveInventoryItemFunctionCallRef: 0x{:x}", ServerRemoveInventoryItemFunctionCallRef.Get() - __int64(GetModuleHandleW(0)));

        uint64 ServerRemoveInventoryItemFunctionCallBeginFunctionAddr = 0;

        for (int i = 0; i < 400; i++)
        {
            if (*(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i) == 0x48 && *(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i + 1) == 0x89 && *(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i + 2) == 0x5C)
            {
                ServerRemoveInventoryItemFunctionCallBeginFunctionAddr = ServerRemoveInventoryItemFunctionCallRef.Get() - i;
                break;
            }

            if (*(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i) == 0x48 && *(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i + 1) == 0x83 && *(uint8_t*)(uint8_t*)(ServerRemoveInventoryItemFunctionCallRef.Get() - i + 2) == 0xEC)
            {
                ServerRemoveInventoryItemFunctionCallBeginFunctionAddr = ServerRemoveInventoryItemFunctionCallRef.Get() - i;
                break;
            }
        }

        Hooking::MinHook::Hook(
            Memcury::Scanner(ServerRemoveInventoryItemFunctionCallBeginFunctionAddr).GetAs<PVOID>(),
            UFortInventoryInterface::RemoveInventoryItemHook
        );
    }
   
    // if (Fortnite_Version >= 13)
    Hooking::MinHook::Hook((PVOID)Addresses::SetZoneToIndex, (PVOID)SetZoneToIndexHook, (PVOID*)&SetZoneToIndexOriginal);
    Hooking::MinHook::Hook((PVOID)Addresses::EnterAircraft, (PVOID)AFortPlayerControllerAthena::EnterAircraftHook, (PVOID*)&AFortPlayerControllerAthena::EnterAircraftOriginal);

#ifndef PROD
    Hooking::MinHook::Hook((PVOID)Addresses::ProcessEvent, ProcessEventHook, (PVOID*)&UObject::ProcessEventOriginal);
#endif

    AddVehicleHook();

    if (Fortnite_Version > 1.8 || Fortnite_Version == 1.11)
    {
        LOG_INFO(LogDev, "ahh!");
        LOG_INFO(LogDev, "Test: 0x{:x}", FindFunctionCall(L"ClientOnPawnDied") - __int64(GetModuleHandleW(0)));
        Hooking::MinHook::Hook((PVOID)FindFunctionCall(L"ClientOnPawnDied"), AFortPlayerController::ClientOnPawnDiedHook, (PVOID*)&AFortPlayerController::ClientOnPawnDiedOriginal);
    }

    LOG_INFO(LogDev, "PredictionKeySize: 0x{:x} {}", PredictionKeySize, PredictionKeySize);

    static auto GameplayEventDataSize = FindObject<UStruct>("/Script/GameplayAbilities.GameplayEventData")->GetPropertiesSize();
    LOG_INFO(LogDev, "GameplayEventDataSize: 0x{:x} {}", GameplayEventDataSize, GameplayEventDataSize);

    {
        int increaseOffset = 0x10;

        if (Engine_Version >= 424 && std::floor(Fortnite_Version) < 18) // checked on 11.31, 12.41, 14.60, 15.10, 16.40, 17.30 and 18.40
            increaseOffset += 0x8;

        auto MoveSoundStimulusBroadcastIntervalOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerPawn", "MoveSoundStimulusBroadcastInterval");
        MemberOffsets::FortPlayerPawn::CorrectTags = MoveSoundStimulusBroadcastIntervalOffset + increaseOffset;
        LOG_INFO(LogDev, "CorrectTags: 0x{:x}", MemberOffsets::FortPlayerPawn::CorrectTags);
        MemberOffsets::FortPlayerState::PawnDeathLocation = FindOffsetStruct("/Script/FortniteGame.FortPlayerState", "PawnDeathLocation", false);

        MemberOffsets::FortPlayerPawnAthena::LastFallDistance = FindOffsetStruct("/Script/FortniteGame.FortPlayerPawnAthena", "LastFallDistance", false);

        MemberOffsets::FortPlayerStateAthena::DeathInfo = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "DeathInfo");
        MemberOffsets::FortPlayerStateAthena::KillScore = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "KillScore");
        MemberOffsets::FortPlayerStateAthena::TeamKillScore = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "TeamKillScore");

        MemberOffsets::DeathInfo::bDBNO = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "bDBNO");
        MemberOffsets::DeathInfo::DeathCause = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "DeathCause");
        MemberOffsets::DeathInfo::bInitialized = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "bInitialized", false);
        MemberOffsets::DeathInfo::Distance = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "Distance", false);
        MemberOffsets::DeathInfo::DeathTags = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "DeathTags", false);
        MemberOffsets::DeathInfo::DeathLocation = FindOffsetStruct("/Script/FortniteGame.DeathInfo", "DeathLocation", false);

        MemberOffsets::DeathReport::Tags = FindOffsetStruct("/Script/FortniteGame.FortPlayerDeathReport", "Tags");
        MemberOffsets::DeathReport::KillerPawn = FindOffsetStruct("/Script/FortniteGame.FortPlayerDeathReport", "KillerPawn");
        MemberOffsets::DeathReport::KillerPlayerState = FindOffsetStruct("/Script/FortniteGame.FortPlayerDeathReport", "KillerPlayerState");
        MemberOffsets::DeathReport::DamageCauser = FindOffsetStruct("/Script/FortniteGame.FortPlayerDeathReport", "DamageCauser");
    }

    srand(time(0));

    LOG_INFO(LogHook, "Finished!");

    while (true)
    {
        Sleep(10000);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

