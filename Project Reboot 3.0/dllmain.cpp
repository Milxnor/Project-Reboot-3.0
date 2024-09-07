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
#include "FortGameStateAthena.h"
#include "BuildingGameplayActorSpawnMachine.h"
#include "BuildingWeapons.h"

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
#include "FortAthenaVehicleSpawner.h"
#include "FortGameSessionDedicatedAthena.h"
#include "FortAIEncounterInfo.h"
#include "FortServerBotManagerAthena.h"
#include "botnames.h"

enum class EMeshNetworkNodeType : uint8_t
{
    Root = 0,
    Inner = 1,
    Edge = 2,
    Client = 3,
    Unknown = 4,
    EMeshNetworkNodeType_MAX = 5
};

enum ENetMode
{
    NM_Standalone,
    NM_DedicatedServer,
    NM_ListenServer,
    NM_Client,
    NM_MAX,
};

static EMeshNetworkNodeType GetMeshNetworkNodeTypeHook(__int64 a1)
{
    LOG_INFO(LogDev, "GetMeshNetworkNodeTypeHook Ret: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));
    return EMeshNetworkNodeType::Edge;
}

constexpr ENetMode NetMode = ENetMode::NM_DedicatedServer;

static ENetMode GetNetModeHook() { return NetMode; }
static ENetMode GetNetModeHook2() { return NetMode; }

static bool ReturnTrueHook() { return true; }
static bool ReturnFalseHook() { return false; }
static int Return2Hook() { return 2; }

static bool NoMCPHook() { return Globals::bNoMCP; }
static void CollectGarbageHook() { return; }

static __int64 (*ConstructEmptyQueryInfoOriginal)(FEncounterEnvironmentQueryInfo* a1);

static __int64 ConstructEmptyQueryInfoHook(FEncounterEnvironmentQueryInfo* a1)
{
    auto ret = ConstructEmptyQueryInfoOriginal(a1);

    a1->GetEnvironmentQuery() = FindObject<UEnvQuery>(L"/Game/Athena/Deimos/AIDirector/EQS/Deimos_EQS_RiftSlots.Deimos_EQS_RiftSlots");
    a1->IsDirectional() = true;

    LOG_INFO(LogDev, "ConstructEmptyQueryInfoHook!");

    return ret;
}

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

static bool (*CanCreateInCurrentContextOriginal)(UObject* Template);

bool CanCreateInCurrentContextHook(UObject* Template)
{
    auto originalRet = CanCreateInCurrentContextOriginal(Template);

    if (!originalRet)
    {
        LOG_INFO(LogDev, "CanCreateInCurrentContextHook false but returning true for {}!", Template->IsValidLowLevel() ? Template->GetPathName() : "BadRead");
    }

    return true;
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
    if (!Hero)
        return ApplyHomebaseEffectsOnPlayerSetupOriginal(GameState, a2, a3, a4, Hero, a6, a7);

    LOG_INFO(LogDev, "Old hero: {}", Hero ? Hero->GetFullName() : "InvalidObject");

    UFortItemDefinition* HeroType = FindObject<UFortItemDefinition>(L"/Game/Athena/Heroes/HID_030_Athena_Commando_M_Halloween.HID_030_Athena_Commando_M_Halloween");

    if (Fortnite_Version == 1.72 || Fortnite_Version == 1.8)
    {
        auto AllHeroTypes = GetAllObjectsOfClass(FindObject<UClass>(L"/Script/FortniteGame.FortHeroType"));
        std::vector<UFortItemDefinition*> AthenaHeroTypes;

        for (int i = 0; i < AllHeroTypes.size(); i++)
        {
            auto CurrentHeroType = (UFortItemDefinition*)AllHeroTypes.at(i);

            if (CurrentHeroType->GetPathName().starts_with("/Game/Athena/Heroes/"))
                AthenaHeroTypes.push_back(CurrentHeroType);
        }

        if (AthenaHeroTypes.size() > 0)
        {
            HeroType = AthenaHeroTypes.at(std::rand() % AthenaHeroTypes.size() /* - 1 */);
        }
    }

    static auto ItemDefinitionOffset = Hero->GetOffset("ItemDefinition");
    Hero->Get<UFortItemDefinition*>(ItemDefinitionOffset) = HeroType;

    return ApplyHomebaseEffectsOnPlayerSetupOriginal(GameState, a2, a3, a4, Hero, a6, a7);
}

/*

static unsigned __int8 (*SpecialEventScript_ActivatePhaseOriginal)(UObject* SpecialEventScript, int NewPhase);

unsigned __int8 SpecialEventScript_ActivatePhaseHook(UObject* SpecialEventScript, int NewPhase)
{
    LOG_INFO(LogDev, "SpecialEventScript_ActivatePhaseHook {}!", NewPhase);

    static auto ReplicatedActivePhaseIndexOffset = SpecialEventScript->GetOffset("ReplicatedActivePhaseIndex");
    SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = NewPhase;

    static auto OnRep_ReplicatedActivePhaseIndexFn = FindObject<UFunction>("/Script/SpecialEventGameplayRuntime.SpecialEventScript.OnRep_ReplicatedActivePhaseIndex");
    SpecialEventScript->ProcessEvent(OnRep_ReplicatedActivePhaseIndexFn);

    return SpecialEventScript_ActivatePhaseOriginal(SpecialEventScript, NewPhase);
}

*/

static void (*ActivatePhaseAtIndexOriginal)(UObject* SpecialEventScript, int Index);

void ActivatePhaseAtIndexHook(UObject* SpecialEventScript, int Index)
{
    LOG_INFO(LogDev, "ActivatePhaseAtIndexHook {}!", Index);

    static auto ReplicatedActivePhaseIndexOffset = SpecialEventScript->GetOffset("ReplicatedActivePhaseIndex");
    SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = Index;

    static auto OnRep_ReplicatedActivePhaseIndexFn = FindObject<UFunction>("/Script/SpecialEventGameplayRuntime.SpecialEventScript.OnRep_ReplicatedActivePhaseIndex");
    SpecialEventScript->ProcessEvent(OnRep_ReplicatedActivePhaseIndexFn);

    auto& ClientConnections = GetWorld()->GetNetDriver()->GetClientConnections();

    // Rift Tour
    if (Fortnite_Version == 17.30)
    {
        auto EventPlaylist = GetEventPlaylist();

        struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ Cast<AFortGameStateAthena>(GetWorld()->GetGameState()), EventPlaylist };

        {
            if (Index == 0) // Start of Event
            {
                auto AllSatelliteDishes = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FindObject<UClass>("/Game/Athena/Apollo/Environments/BuildingActors/Complex/Props/Apollo_Complex_Satellite_Dish_Top_01.Apollo_Complex_Satellite_Dish_Top_01_C"));

                for (int i = 0; i < AllSatelliteDishes.Num(); i++)
                {
                    AllSatelliteDishes.At(i)->K2_DestroyActor();
                }

                auto AllSpecialEventMutators = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FindObject<UClass>("/Script/SpecialEventGameplayRuntime.FortAthenaMutator_SpecialEvent"));

                for (int i = 0; i < AllSpecialEventMutators.Num(); i++)
                {
                    auto CurrentSpecialEventMutator = AllSpecialEventMutators.At(i);

                    auto ScriptActor = CurrentSpecialEventMutator->Get<AActor*>(CurrentSpecialEventMutator->GetOffset("ScriptActor"));

                    if (!ScriptActor)
                    {
                        ScriptActor = Cast<AActor>(GetEventScripting());
                        LOG_INFO(LogEvent, "ScriptActor set for mutator: {}", CurrentSpecialEventMutator->GetName());
                    }
                }
            }
            if (Index == 2) // Slide
            {
                auto Script = FindObject<UObject>(L"/Buffet/Levels/Buffet_Part_4.Buffet_Part_4.PersistentLevel.BP_Buffet_PhaseScripting_Paint_4");
                auto SplineActor = Script->Get<AActor*>(Script->GetOffset("SplineActor"));
                auto PawnLocation = Script->Get<AActor*>(Script->GetOffset("PawnLocation")) = SplineActor;

                auto AllWrapsSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FindObject<UClass>(L"/Script/SpecialEventGameplayRuntime.FortSpecialRelevancyActorSpawner"));

                for (int i = 0; i < AllWrapsSpawners.Num(); i++)
                {
                    auto CurrentWrapSpawner = AllWrapsSpawners.At(i);

                    if (CurrentWrapSpawner)
                    {
                        auto WrapSpawnerLocation = CurrentWrapSpawner->GetActorLocation();
                        auto WrapSpawnerRotation = CurrentWrapSpawner->GetActorRotation();
                        UClass* ClassToSpawn = nullptr;

                        if (CurrentWrapSpawner->GetName().contains("WrapChangePickup"))
                        {
                            ClassToSpawn = FindObject<UClass>(L"/Buffet/Gameplay/Blueprints/WrapWorldPrototype/BP_Buffet_Paint_WrapChangePickup.BP_Buffet_Paint_WrapChangePickup_C");
                        }
                        else if (CurrentWrapSpawner->GetName().contains("Paint_Pickup"))
                        {
                            ClassToSpawn = FindObject<UClass>(L"/Buffet/Gameplay/Blueprints/WrapWorldPrototype/BP_Buffet_Paint_Pickup.BP_Buffet_Paint_Pickup_C");
                        }
                        auto SpawnedWrap = GetWorld()->SpawnActor<AActor>(ClassToSpawn, WrapSpawnerLocation, WrapSpawnerRotation.Quaternion(), FVector(1, 1, 1), 
                            CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AlwaysSpawn, true, nullptr));

                        if (SpawnedWrap)
                        {
                            LOG_INFO(LogDev, "Spawner Spawned a Wrap: {}", SpawnedWrap->GetName());
                        }
                        else
                        {
                            LOG_INFO(LogDev, "Spawner Failed to spawn a Wrap {}!", CurrentWrapSpawner->GetName());
                        }
                    }
                    else
                    {
                        LOG_INFO(LogDev, "Spawner Invalid!");
                    }
                }

                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentController = ClientConnections.At(i)->GetPlayerController();
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();

                    if (!CurrentPawn)
                        continue;

                    auto PlayerComponent = CurrentPawn->AddComponentByClass(FindObject<UClass>("/Buffet/Gameplay/Blueprints/WrapWorldPrototype/BP_Buffet_Paint_PlayerComponent.BP_Buffet_Paint_PlayerComponent_C")); // WHY DOESnT IT WORK AUTO
                    auto MovementComponent = CurrentPawn->AddComponentByClass(FindObject<UClass>("/Buffet/Gameplay/Blueprints/WrapWorldPrototype/BP_Buffet_Paint_MovementComponent.BP_Buffet_Paint_MovementComponent_C"));

                    PlayerComponent->Get<AActor*>(PlayerComponent->GetOffset("OwningPlayerController")) = CurrentController;

                    struct
                    {
                        AActor* Pawn;
                        AActor* ReturnValue;
                    }wrapmanagerparams{};
                    wrapmanagerparams.Pawn = CurrentPawn;

                    Script->ProcessEvent(Script->FindFunction("GetWrapManagerForPlayer"), &wrapmanagerparams);

                    MovementComponent->Get<bool>(MovementComponent->GetOffset("bIsMovingAlongSpline")) = true;
                    MovementComponent->Get<float>(MovementComponent->GetOffset("ReplicatedTotalSplineTime")) = 59.793846f; // TotalSplineTime
                    MovementComponent->Get<AActor*>(MovementComponent->GetOffset("TargetSplineActor")) = SplineActor;
                    MovementComponent->ProcessEvent(MovementComponent->FindFunction("SetSplineActor"), &SplineActor);
                    bool IsMoving = true;
                    MovementComponent->ProcessEvent(MovementComponent->FindFunction("SetIsMovingAlongSpline"), &IsMoving);
                    MovementComponent->Get<float>(MovementComponent->GetOffset("ReplicatedSplineInterpStrength")) = 1.3f; // SplineInterpStrength
                    auto GameMode = (AFortGameMode*)GetWorld()->GetGameMode();
                    auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());
                    MovementComponent->Get<float>(MovementComponent->GetOffset("StartServerWorldTime")) = GameState->GetServerWorldTimeSeconds();

                    MovementComponent->ProcessEvent(MovementComponent->FindFunction("OnRep_TargetSplineActor"));

                    PlayerComponent->Get<AActor*>(PlayerComponent->GetOffset("WrapManager")) = wrapmanagerparams.ReturnValue;

                    PlayerComponent->Get<UObject*>(PlayerComponent->GetOffset("MovementComponent")) = MovementComponent;

                    int StasisMode = 3;
                    CurrentPawn->ProcessEvent(CurrentPawn->FindFunction("SetStasisMode"), &StasisMode); // ??
                }
            }
            if (Index == 3)
            {
                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();
                    auto CurrentController = ClientConnections.At(i)->GetPlayerController();
                    int StasisMode = 0;

                    CurrentPawn->ProcessEvent(CurrentPawn->FindFunction("SetStasisMode"), &StasisMode);
                }
            }
            if (Index == 4) // Storm King
            {
                auto StormScript = FindObject<AActor>("/Buffet/Levels/Buffet_Part_6.Buffet_Part_6:PersistentLevel.BP_Buffet_PhaseScripting_4");

                auto DefaultPlane = FindObject("/Buffet/Gameplay/Blueprints/WolfMother/BP_PlanePrototype.Default__BP_PlanePrototype_C");

                auto WID = Cast<UFortWorldItemDefinition>(FindObject("WID_Buffet_BeatMatchingBoomBox", nullptr, ANY_PACKAGE)); // Storm King weapon thing

                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();
                    auto CurrentController = (AFortPlayerControllerAthena*)ClientConnections.At(i)->GetPlayerController();

                    auto WorldInventory = CurrentController->GetWorldInventory();

                    bool bShouldUpdate = false;
                    WorldInventory->AddItem(WID, &bShouldUpdate, 1);

                    if (bShouldUpdate)
                        WorldInventory->Update();

                    SendMessageToConsole(CurrentController, L"Gave WID_Buffet_BeatMatchingBoomBox!");
                }

                auto AllWeps = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FindObject<UClass>(L"/Buffet/Gameplay/Blueprints/WolfMother/BeatmatchingWeapon/B_Buffet_BeatMatchingWeaponPrototype.B_Buffet_BeatMatchingWeaponPrototype_C"));

                for (int i = 0; i < AllWeps.Num(); i++)
                {
                    auto CurrentWep = AllWeps.At(i);

                    if (CurrentWep != nullptr)
                    {
                        CurrentWep->Get<AActor*>(CurrentWep->GetOffset("Phase4ScriptingRef")) = StormScript;
                    }
                }
            }
            if (Index == 6) // Ariana
            {
                auto ReflectScript = FindObject(L"/Buffet/Levels/Buffet_Reflect.Buffet_Reflect:PersistentLevel.BP_Buffet_PhaseScripting_Stars_2");

                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();

                    CurrentPawn->AddComponentByClass(FindObject<UClass>("/Buffet/Gameplay/Blueprints/Stars/BP_Buffet_Stars_PlayerComponent.BP_Buffet_Stars_PlayerComponent_C"));
                    CurrentPawn->AddComponentByClass(FindObject<UClass>("/BuffetPlaylist/Playlist/BuffetCharacterMovementMutatorComponent.BuffetCharacterMovementMutatorComponent_C"));
                }
            }
            if (Index == 8) // Bubbles
            {
                //auto BubbleScript = FindObject("/Buffet/Levels/Buffet_Bubbles.Buffet_Bubbles:PersistentLevel.BP_Buffet_PhaseScripting_Bubble_4");
                //auto BubbleSpline = FindObject("/Buffet/Levels/Buffet_Bubbles.Buffet_Bubbles:PersistentLevel.BP_Buffet_SmallBubblePath_2.Spline");

                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();
                    auto ComptoRemove = CurrentPawn->GetComponentByClass(FindObject<UClass>("/Buffet/Gameplay/Blueprints/Stars/BP_Buffet_Stars_PlayerComponent.BP_Buffet_Stars_PlayerComponent_C"));

                    auto BubbleMovementComponent = CurrentPawn->AddComponentByClass(FindObject<UClass>("/Buffet/Gameplay/Blueprints/Bubble/BP_BubblePlayerMovementComponent.BP_BubblePlayerMovementComponent_C"));

                    /*
                    struct
                    {
                        UObject* InSplineComponent;
                        float StartDistance;

                    }SetSplineComponent;
                    SetSplineComponent.StartDistance = 0;
                    SetSplineComponent.InSplineComponent = BubbleSpline;

                    // Works but too fast and player not in correct place on the spline.
                    BubbleMovementComponent->ProcessEvent(BubbleMovementComponent->FindFunction("SetSplineComponent"), &SetSplineComponent);
                    BubbleMovementComponent->Get<UObject*>(BubbleMovementComponent->GetOffset("SplineComponent")) = BubbleSpline;
                    BubbleMovementComponent->Get<bool>(BubbleMovementComponent->GetOffset("bIsMovingAlongSpline")) = true;
                    auto GameMode = (AFortGameMode*)GetWorld()->GetGameMode();
                    auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());
                    BubbleMovementComponent->Get<float>(BubbleMovementComponent->GetOffset("StartServerWorldTime")) = GameState->GetServerWorldTimeSeconds();
                    bool IsMoving = true;
                    BubbleMovementComponent->ProcessEvent(BubbleMovementComponent->FindFunction("SetIsMovingAlongSpline"), &IsMoving);
                    */

                    CurrentPawn->ProcessEvent(CurrentPawn->FindFunction("K2_DestroyComponent"), &ComptoRemove);
                }
            }
            if (Index == 10) // Escher
            {
                for (int i = 0; i < ClientConnections.Num(); i++)
                {
                    auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();

                    CurrentPawn->TeleportTo(FVector(36.34f, 1044.07f, 504.50f), CurrentPawn->GetActorRotation());
                }
            }
        }
    }

    if (Fortnite_Version == 18.40)
    {
        if (Index == 0)
        {
            for (int i = 0; i < ClientConnections.Num(); i++)
            {
                auto CurrentController = (AFortPlayerControllerAthena*)ClientConnections.At(i)->GetPlayerController();

                static auto StormEffectClass = FindObject<UClass>(L"/Game/Athena/SafeZone/GE_OutsideSafeZoneDamage.GE_OutsideSafeZoneDamage_C");
                auto PlayerState = CurrentController->GetPlayerStateAthena();
                PlayerState->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(StormEffectClass, 1, PlayerState->GetAbilitySystemComponent());
            }

            auto AssultWID = Cast<UFortWorldItemDefinition>(FindObject("WID_Guava_Assault_AutoHigh_Athena_SR_Ore_T03", nullptr, ANY_PACKAGE));
            auto RevolveWID = Cast<UFortWorldItemDefinition>(FindObject("WID_Guava_Pistol_SixShooter_Athena_SR_Ore_T03", nullptr, ANY_PACKAGE));

            for (int i = 0; i < ClientConnections.Num(); i++)
            {
                auto CurrentPawn = ClientConnections.At(i)->GetPlayerController()->GetPawn();
                auto CurrentController = (AFortPlayerControllerAthena*)ClientConnections.At(i)->GetPlayerController();

                auto WorldInventory = CurrentController->GetWorldInventory();

                bool bShouldUpdate = true;
                WorldInventory->AddItem(AssultWID, &bShouldUpdate, 1);
                WorldInventory->AddItem(RevolveWID, &bShouldUpdate, 1);


                if (bShouldUpdate)
                    WorldInventory->Update();

                SendMessageToConsole(CurrentController, L"Gave Event Weapons!");
            }

            auto AllSatelliteDishes = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FindObject<UClass>("/Game/Athena/Apollo/Environments/BuildingActors/Complex/Props/Apollo_Complex_Satellite_Dish_Top_01.Apollo_Complex_Satellite_Dish_Top_01_C"));

            for (int i = 0; i < AllSatelliteDishes.Num(); i++)
            {
                AllSatelliteDishes.At(i)->K2_DestroyActor();
            }
        }
    }

    return ActivatePhaseAtIndexOriginal(SpecialEventScript, Index);
}

static void (*AttemptFinishPhaseOriginal)(AActor* SpecialEventScript, FGameplayTag PhaseTag);
void AttemptFinishPhaseHook(AActor* SpecialEventScript, FGameplayTag PhaseTag)
{
    LOG_INFO(LogDev, "SpecialEventScript::AttemptFinishPhase PhaseTag: {}", PhaseTag.TagName.ToString());

    static auto ReplicatedActivePhaseIndexOffset = SpecialEventScript->GetOffset("ReplicatedActivePhaseIndex"); // Sets what Map/Phase were replicating to clients.
    auto LastTag = PhaseTag.TagName.ToString();

    static auto OnRep_ReplicatedActivePhaseIndexFn = FindObject<UFunction>("/Script/SpecialEventGameplayRuntime.SpecialEventScript.OnRep_ReplicatedActivePhaseIndex");

    auto& Connections = GetWorld()->GetNetDriver()->GetClientConnections();

    if (LastTag == "Kiwi.Phase.Islands") // PrisonBlocks (Event Start)
    {
        SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = 1;

        for (int i = 0; i < Connections.Num(); i++)
        {
            auto CurrentPawn = Cast<AFortPlayerPawn>(Connections.At(i)->GetPlayerController()->GetPawn());

            CurrentPawn->TeleportTo(FVector(34640.0f, 32350.0f, 100600.0f), CurrentPawn->GetActorRotation());

            CurrentPawn->ServerChoosePart(EFortCustomPartType::Backpack, FindObject("/Kiwi/Gameplay/Blueprints/Backpack/CP_Backpack_Kiwi.CP_Backpack_Kiwi"));
        }
    }
    if (LastTag == "Kiwi.Phase.Prison") // Kevin
    {
        SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = 2;
    }
    if (LastTag == "Kiwi.Phase.ObservationRoom") // Hangar
    {
        SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = 3;
    }
    if (LastTag == "Kiwi.Phase.Hanger") // Ending
    {
        SpecialEventScript->Get<int32>(ReplicatedActivePhaseIndexOffset) = 4;
    }
    SpecialEventScript->ProcessEvent(OnRep_ReplicatedActivePhaseIndexFn);

    return AttemptFinishPhaseOriginal(SpecialEventScript, PhaseTag);
}

// Used to teleport the pawn in some events, not used in all parts of rift tour for some reason.
static void (*TeleportPlayerPawnOriginal)(UObject* Context, FFrame& Stack, void* Ret);
void TeleportPlayerPawnHook(UObject* Context, FFrame& Stack, void* Ret)
{
    LOG_INFO(LogEvent, "Teleported PlayerPawn!");

    UObject* WorldContextObject;
    AFortPlayerPawn* PlayerPawn;                                                      //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    FVector                DestLocation;                                                //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    FRotator               DestRotation;                                                    //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
    bool                   bIgnoreCollision;                                             //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    bool                   bIgnoreSupplementalKillVolumeSweep;                           //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

    Stack.StepCompiledIn(&WorldContextObject);
    Stack.StepCompiledIn(&PlayerPawn);
    Stack.StepCompiledIn(&DestLocation);
    Stack.StepCompiledIn(&DestRotation);
    Stack.StepCompiledIn(&bIgnoreCollision);
    Stack.StepCompiledIn(&bIgnoreSupplementalKillVolumeSweep);

    PlayerPawn->TeleportTo(DestLocation, DestRotation);

    return TeleportPlayerPawnOriginal(Context, Stack, Ret);
}

/*

// Storm King Spawning, I might put this in its own class soon.
bool bStormKingSpawned = false;
static inline void (*DADBroOnGamephaseStepChangedOriginal)(UObject* Context, FFrame& Stack, void* Ret);
void DADBroOnGamephaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret)
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!GameState)
        return DADBroOnGamephaseStepChangedOriginal(Context, Stack, Ret);

    FRotator Rotation;
    Context->ProcessEvent(Context->FindFunction("GetDesiredDadBroRotation"), &Rotation);

    if (GameState->GetGamePhaseStep() == EAthenaGamePhaseStep::StormShrinking && bStormKingSpawned == false)
    {
        auto StormKing = GetWorld()->SpawnActor<AActor>(FindObject<UClass>("/Game/Athena/DADBRO/DADBRO_Pawn.DADBRO_Pawn_C"), FVector{ 5150.0f, 4900.0f, -100.0f }, Rotation.Quaternion(), FVector{ 1,1,1 }, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AlwaysSpawn));

        if (StormKing)
        {
            bStormKingSpawned = true;
            Context->Get<AActor*>(Context->GetOffset("DadBroPawn")) = StormKing;
            Context->ProcessEvent(Context->FindFunction("OnRep_DadBroPawn"));
        }
        else
        {
            LOG_INFO(LogDev, "Failed to spawn StormKing!");
        }
    }

    return DADBroOnGamephaseStepChangedOriginal(Context, Stack, Ret);
}
*/

static void (*StartEventAtIndexOriginal)(UObject* Context, FFrame& Stack, void* Ret);
void StartEventAtIndexHook(UObject* Context, FFrame& Stack, void* Ret)
{
    int32 InStartingIndex;
    Stack.StepCompiledIn(&InStartingIndex);

    if (Fortnite_Version == 17.50)
    {
        auto Scripting = FindObject<UObject>("/Kiwi/Levels/Kiwi_P.Kiwi_P:PersistentLevel.BP_Kiwi_Master_Scripting_2");
        auto EventPlaylist = GetEventPlaylist();

        struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ Cast<AFortGameStateAthena>(GetWorld()->GetGameState()), EventPlaylist };
        if (EventPlaylist)
        {
            static auto GameplayTagContainerOffset = EventPlaylist->GetOffset("GameplayTagContainer");
            OnReadyParams.PlaylistContextTags = EventPlaylist->Get<FGameplayTagContainer>(GameplayTagContainerOffset);
        }
        else
        {
            OnReadyParams.PlaylistContextTags = FGameplayTagContainer();
        }

        Scripting->ProcessEvent(Scripting->FindFunction("OnReady_F1A32853487CB7603278E6847A5F2625"), &OnReadyParams);
        Context->ProcessEvent(Context->FindFunction("LoadKiwiAssets"), &OnReadyParams);
        Context->ProcessEvent(Context->FindFunction("BP_OnScriptReady"), &OnReadyParams);
        Scripting->ProcessEvent(Scripting->FindFunction("startevent"), &InStartingIndex);
    }

    return StartEventAtIndexOriginal(Context, Stack, Ret);
}

static __int64 (*FlowStep_SetPhaseToActiveOriginal)(AActor* SpecialEventPhase);

__int64 FlowStep_SetPhaseToActiveHook(AActor* SpecialEventPhase)
{
    LOG_INFO(LogDev, "FlowStep_SetPhaseToActiveHook!");

    auto ret = FlowStep_SetPhaseToActiveOriginal(SpecialEventPhase); // idk if three actually is a ret

    static auto OnRep_PhaseState = FindObject<UFunction>(L"/Script/SpecialEventGameplayRuntime.SpecialEventPhase.OnRep_PhaseState");
    SpecialEventPhase->ProcessEvent(OnRep_PhaseState);

    SpecialEventPhase->ForceNetUpdate();

    return ret;
}

UObject* GetGoalManagerHook(UObject* WorldContextObject)
{
    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
    static auto AIGoalManagerOffset = GameMode->GetOffset("AIGoalManager");

    LOG_INFO(LogDev, "WHAT A BOZO GetGoalManagerHook RET: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

    return GameMode->Get(AIGoalManagerOffset);
}

UObject* GetAIDirectorHook()
{
    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
    static auto AIDirectorOffset = GameMode->GetOffset("AIDirector");

    LOG_INFO(LogDev, "WHAT A BOZO GetAIDirectorHook RET: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

    return GameMode->Get(AIDirectorOffset);
}

void ChangeLevels()
{
    constexpr bool bUseRemovePlayer = false;
    constexpr bool bUseSwitchLevel = false;
    constexpr bool bShouldRemoveLocalPlayer = true;

    LOG_INFO(LogDev, "FindGIsClient(): 0x{:x}", FindGIsClient() - __int64(GetModuleHandleW(0)));

    // auto bruh = std::wstring(CustomMapName.begin(), CustomMapName.end());
    // auto bruhh = (L"open " + bruh);

    FString LevelB = /* bUseCustomMap ? bruhh.c_str() : */ (Engine_Version < 424
        ? L"open Athena_Terrain" : Engine_Version >= 500 ? Engine_Version >= 501
        ? L"open Asteria_Terrain"
        : Globals::bCreative ? L"open Creative_NoApollo_Terrain"
        : L"open Artemis_Terrain"
        : Globals::bCreative ? L"open Creative_NoApollo_Terrain"
        : L"open Apollo_Terrain");

    FString Level = /* bUseCustomMap ? bruh.c_str() : */ (Engine_Version < 424
        ? L"Athena_Terrain" : Engine_Version >= 500 ? Engine_Version >= 501
        ? L"Asteria_Terrain"
        : Globals::bCreative ? L"Creative_NoApollo_Terrain"
        : L"Artemis_Terrain"
        : Globals::bCreative ? L"Creative_NoApollo_Terrain"
        : L"Apollo_Terrain");

    LOG_INFO(LogDev, "Using {}.", bUseSwitchLevel ? Level.ToString() : LevelB.ToString());

    auto LocalPC = GetLocalPlayerController();

    LOG_INFO(LogDev, "Got PC: {}", __int64(LocalPC));

    if (bUseSwitchLevel)
    {
        static auto SwitchLevelFn = FindObject<UFunction>(L"/Script/Engine.PlayerController.SwitchLevel");

        LocalPC->ProcessEvent(SwitchLevelFn, &Level);

        if (FindGIsServer())
        {
            *(bool*)FindGIsServer() = true;
        }

        if (FindGIsClient())
        {
            *(bool*)FindGIsClient() = false;
        }
    }
    else
    {
        if (Fortnite_Version != 18.10)
        {
            if (FindGIsServer())
            {
                *(bool*)FindGIsServer() = true;
            }

            if (FindGIsClient())
            {
                *(bool*)FindGIsClient() = false;
            }
        }

        if (bShouldRemoveLocalPlayer)
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
                UGameplayStatics::RemovePlayer((APlayerController*)LocalPC, true);
            }
        }

        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), LevelB, nullptr);
    }

    LOG_INFO(LogPlayer, "Switched level.");

    if (bUseSwitchLevel && bShouldRemoveLocalPlayer)
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
}

void ApplyNullAndRetTrues()
{
    static auto FortPlayerControllerAthenaDefault = FindObject<AFortPlayerControllerAthena>(L"/Script/FortniteGame.Default__FortPlayerControllerAthena"); // FindObject<UClass>(L"/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");

    auto AddressesToNull = Addresses::GetFunctionsToNull();
    const auto AddressesToReturnTrue = Addresses::GetFunctionsToReturnTrue();

    auto ServerCheatAllIndex = GetFunctionIdxOrPtr(FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerCheatAll"));

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

    for (auto func : AddressesToReturnTrue)
    {
        if (func == 0)
            continue;

        LOG_INFO(LogDev, "Forcing return true on 0x{:x}", func - __int64(GetModuleHandleW(0)));

        MH_CreateHook((PVOID)func, ReturnTrueHook, nullptr);
        MH_EnableHook((PVOID)func);
    }
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

    std::cout << std::format("Base Address: 0x{:x}\n", __int64(GetModuleHandleW(0)));

    LOG_INFO(LogInit, "Initializing Project Reboot!");
    LOG_INFO(LogDev, "Built on {} {}", __DATE__, __TIME__);

    Addresses::SetupVersion();

    NumToSubtractFromSquadId = Engine_Version >= 424 ? 2 : Engine_Version >= 423 ? 3 : 0; // TODO: check this
    NumElementsPerChunk = std::floor(Fortnite_Version) >= 5 && Fortnite_Version <= 6 ? 0x10400 : 0x10000; // Idk what version tbh

    Offsets::FindAll(); // We have to do this before because FindCantBuild uses FortAIController.CreateBuildingActor
    Offsets::Print();

    Addresses::FindAll();
    Addresses::Init();
    Addresses::Print();

    bEnableRebooting = Addresses::RebootingDelegate && Addresses::FinishResurrection && Addresses::GetSquadIdForCurrentPlayer && false;

    LOG_INFO(LogDev, "Fortnite_CL: {}", Fortnite_CL);
    LOG_INFO(LogDev, "Fortnite_Version: {}", Fortnite_Version);
    LOG_INFO(LogDev, "Engine_Version: {}", Engine_Version);

#ifdef ABOVE_S20
    if (Fortnite_Version < 20)
    {
        MessageBoxA(0, "Please undefine ABOVE_S20", "Project Reboot 3.0", MB_ICONERROR);
        return 0;
    }
#else
    if (Fortnite_Version > 20)
    {
        MessageBoxA(0, "Please define ABOVE_S20 (compile it yourself and change inc.h)", "Project Reboot 3.0", MB_ICONERROR);
        return 0;
    }
#endif

    CreateThread(0, 0, GuiThread, 0, 0, 0);

    while (SecondsUntilTravel > 0)
    {
        SecondsUntilTravel -= 1;

        Sleep(1000);
    }

    bSwitchedInitialLevel = true;

    // Globals::bAutoRestart = IsRestartingSupported();

    static auto GameModeDefault = FindObject<AFortGameModeAthena>(L"/Script/FortniteGame.Default__FortGameModeAthena");
    static auto FortPlayerControllerZoneDefault = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerControllerZone");
    static auto FortPlayerControllerDefault = FindObject<AFortPlayerController>(L"/Script/FortniteGame.Default__FortPlayerController");
    static auto FortPlayerPawnAthenaDefault = FindObject<AFortPlayerPawn>(L"/Script/FortniteGame.Default__FortPlayerPawnAthena"); // FindObject<AFortPlayerPawn>(L"/Game/Athena/PlayerPawn_Athena.Default__PlayerPawn_Athena_C");
    static auto FortAbilitySystemComponentAthenaDefault = FindObject<UObject>(L"/Script/FortniteGame.Default__FortAbilitySystemComponentAthena");
    static auto FortPlayerStateAthenaDefault = FindObject<AFortPlayerStateAthena>(L"/Script/FortniteGame.Default__FortPlayerStateAthena");
    static auto FortKismetLibraryDefault = FindObject<UFortKismetLibrary>(L"/Script/FortniteGame.Default__FortKismetLibrary");
    static auto AthenaMarkerComponentDefault = FindObject<UAthenaMarkerComponent>(L"/Script/FortniteGame.Default__AthenaMarkerComponent");
    static auto FortWeaponDefault = FindObject<AFortWeapon>(L"/Script/FortniteGame.Default__FortWeapon");
    static auto FortOctopusVehicleDefault = FindObject<AFortOctopusVehicle>(L"/Script/FortniteGame.Default__FortOctopusVehicle");
    static auto FortPlayerControllerAthenaDefault = FindObject<AFortPlayerControllerAthena>(L"/Script/FortniteGame.Default__FortPlayerControllerAthena"); // FindObject<UClass>(L"/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");

    if (Fortnite_Version >= 20 || Fortnite_Version == 12.00)
        ApplyNullAndRetTrues();

    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNetPackageMap VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNetTraffic VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNet VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogNavigation VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortMission VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAIGoalSelection VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortWorld VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortPlayerRegistration VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogBuilding VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortTeams VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAI VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAIDirector VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortQuest VeryVerbose", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector NoLogging", nullptr);
    // UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAbilitySystem VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogDataTable VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogMeshNetwork VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogEQS VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFort VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogGameMode VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogSpecialEvent VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogPlayerController VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogSpecialEventPhase VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortCustomization VeryVerbose", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogSpecialEventScriptMeshActor VeryVerbose", nullptr);

    Hooking::MinHook::Hook((PVOID)Addresses::NoMCP, (PVOID)NoMCPHook, nullptr);
    Hooking::MinHook::Hook((PVOID)Addresses::GetNetMode, (PVOID)GetNetModeHook, nullptr);
    Hooking::MinHook::Hook((PVOID)Addresses::DispatchRequest, (PVOID)DispatchRequestHook, (PVOID*)&DispatchRequestOriginal);

    GSRandSeed = FGenericPlatformTime::Cycles();
    ReplicationRandStream = FRandomStream(FGenericPlatformTime::Cycles());

    Hooking::MinHook::Hook((PVOID)Addresses::KickPlayer, (PVOID)AGameSession::KickPlayerHook, (PVOID*)&AGameSession::KickPlayerOriginal);

    LOG_INFO(LogDev, "Size: 0x{:x}", sizeof(TMap<FName, void*>));

    Hooking::MinHook::Hook((PVOID)Addresses::ActorGetNetMode, (PVOID)GetNetModeHook2, nullptr);

    if (Fortnite_Version > 13 && // ermm
        Fortnite_Version < 20
        ) 
    {
        Hooking::MinHook::Hook(FindObject<ABuildingFoundation>(L"/Script/FortniteGame.Default__BuildingFoundation"),
            FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationTransform"),
            ABuildingFoundation::SetDynamicFoundationTransformHook, (PVOID*)&ABuildingFoundation::SetDynamicFoundationTransformOriginal, false, true);

        Hooking::MinHook::Hook(FindObject<ABuildingFoundation>(L"/Script/FortniteGame.Default__BuildingFoundation"),
            FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.SetDynamicFoundationEnabled"),
            ABuildingFoundation::SetDynamicFoundationEnabledHook, (PVOID*)&ABuildingFoundation::SetDynamicFoundationEnabledOriginal, false, true);
    }


    /*
    if (Fortnite_Version == 6.21) // ur trolling
    {
        std::string AIDirectorFuncName = "/Script/Engine.PlayerController.FOV"; // "/Script/Engine.PlayerController.ClientVoiceHandshakeComplete";
        std::string GoalManagerFuncName = "/Script/Engine.PlayerController.EnableCheats";

        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0xAADD50), (PVOID)ConstructEmptyQueryInfoHook, (PVOID*)&ConstructEmptyQueryInfoOriginal); // 7FF7E556D158  

        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB10480, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xABBAB9, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB1E2BC, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB25EAA, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xABFDC1, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xAEC76D, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xA9C62C, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xAA1165, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xA9F5C0, (PVOID)GetGoalManagerHook, GoalManagerFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x10975EE, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xAC1C15, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB2096E, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x107B6A5, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB213DC, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xAB51D2, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB21BB8, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xABE6AC, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0xB2247D, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x10988B7, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x107C7B6, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x1096D21, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
        HookInstruction(__int64(GetModuleHandleW(0)) + 0x1097982, (PVOID)GetAIDirectorHook, AIDirectorFuncName, ERelativeOffsets::CALL, FortPlayerControllerAthenaDefault);
    }
    */

    if (Fortnite_Version == 17.30) // Rift Tour stuff
    {
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E07910), (PVOID)GetMeshNetworkNodeTypeHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DED158), (PVOID)ReturnTrueHook, nullptr); // 7FF7E556D158  
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DECFC8), (PVOID)ReturnTrueHook, nullptr); // 7FF7E556CFC8
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DED050), (PVOID)ReturnTrueHook, nullptr); // 7FF7E556D050
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DECF40), (PVOID)ReturnFalseHook, nullptr); // 7FF7E556CF40
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3DE5CE8), (PVOID)ActivatePhaseAtIndexHook, (PVOID*)&ActivatePhaseAtIndexOriginal); // 7FF7E5565CE8
    }
    else if (Fortnite_Version == 17.50) // Sky fire stuff
    {
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + ), (PVOID)GetMeshNetworkNodeTypeHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E54BCC), (PVOID)ReturnTrueHook, nullptr); // 7FF638A04BCC    
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E5496C), (PVOID)ReturnTrueHook, nullptr); // 7FF638A0496C   
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E54A68), (PVOID)ReturnTrueHook, nullptr); // 7FF638A04A68 
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + ), (PVOID)ReturnFalseHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E4D768), (PVOID)ActivatePhaseAtIndexHook, (PVOID*)&ActivatePhaseAtIndexOriginal); // 07FF6389FD768
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x3E4DFA4), (PVOID)AttemptFinishPhaseHook, (PVOID*)&AttemptFinishPhaseOriginal);
    }
    else if (Fortnite_Version == 18.40)
    {
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + ), (PVOID)GetMeshNetworkNodeTypeHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x416AAB8), (PVOID)ReturnTrueHook, nullptr); // 7FF79E3EAAB8  
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x416A840), (PVOID)ReturnTrueHook, nullptr); // 7FF79E3EA840
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x416A93C), (PVOID)ReturnTrueHook, nullptr); // 7FF79E3EA93C
        // Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + ), (PVOID)ReturnFalseHook, nullptr);
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x41624C8), (PVOID)ActivatePhaseAtIndexHook, (PVOID*)&ActivatePhaseAtIndexOriginal); // 7FF79E3E24C8  
    }

    if (std::floor(Fortnite_Version) == 4)
    {
        auto RetrieveCharacterPartsAddr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B 01 0F B6 FA 48 8B D9 FF 90 ? ? ? ? 48 8B C8 E8 ? ? ? ? 84 C0 74 0D 33 C0 48 8B 5C 24 ? 48 83 C4 20 5F", false).Get();
        
        if (!RetrieveCharacterPartsAddr)
            RetrieveCharacterPartsAddr = Memcury::Scanner::FindPattern("40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 90 ? ? ? ? 48 8B C8 E8 ? ? ? ? 84 C0 74 08 33 C0 48 83 C4 20 5B C3 48 8B CB").Get(); // 4.0

        LOG_INFO(LogDev, "RetrieveCharacterPartsAddr: {}", RetrieveCharacterPartsAddr);

        if (RetrieveCharacterPartsAddr)
        {
            for (int i = 0; i < 400; i++)
            {
                if (*(uint8_t*)(RetrieveCharacterPartsAddr + i) == 0x74) // jz
                {
                    DWORD dwProtection;
                    VirtualProtect((PVOID)(RetrieveCharacterPartsAddr + i), 1, PAGE_EXECUTE_READWRITE, &dwProtection);

                    *(uint8_t*)(RetrieveCharacterPartsAddr + i) = 0x75; // jnz

                    DWORD dwTemp;
                    VirtualProtect((PVOID)(RetrieveCharacterPartsAddr + i), 1, dwProtection, &dwTemp);

                    LOG_INFO(LogDev, "Applied RetrieveCharacterParts patch!");
                    break;
                }
            }
        }
    }

    if (Globals::bGoingToPlayEvent)
    {
        if (Fortnite_Version >= 17.30)
        {
            Hooking::MinHook::Hook(FindObject<UObject>("/Script/FortniteGame.Default__FortMissionLibrary"), FindObject<UFunction>(L"/Script/FortniteGame.FortMissionLibrary:TeleportPlayerPawn"), TeleportPlayerPawnHook,
                (PVOID*)&TeleportPlayerPawnOriginal, false, true);
            // Todo hook UBuffetRacePlayerMovementComponent::ServerTeleportPlayer?
        }
        if (Fortnite_Version == 17.50)
        {
            Hooking::MinHook::Hook(FindObject<UObject>("/Script/SpecialEventGameplayRuntime.Default__SpecialEventScript"), FindObject<UFunction>(L"/Script/SpecialEventGameplayRuntime.SpecialEventScript:StartEventAtIndex"), StartEventAtIndexHook,
                (PVOID*)&StartEventAtIndexOriginal, false, true);
        }
    }

    /*

    if (Fortnite_Version == 6.21)
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x191D2E0), (PVOID)CanCreateInCurrentContextHook, (PVOID*)&CanCreateInCurrentContextOriginal);
    else if (Fortnite_Version == 10.40)
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x22A30C0), (PVOID)CanCreateInCurrentContextHook, (PVOID*)&CanCreateInCurrentContextOriginal);
    else if (Fortnite_Version == 12.41)
        Hooking::MinHook::Hook((PVOID)(__int64(GetModuleHandleW(0)) + 0x2DBCBA0), (PVOID)CanCreateInCurrentContextHook, (PVOID*)&CanCreateInCurrentContextOriginal);

    */

    ChangeLevels();

    LOG_INFO(LogDev, "Switch levels.");

    if (Fortnite_Version < 20 && Fortnite_Version != 12)
        ApplyNullAndRetTrues();

    if (Fortnite_Version != 22.4)
    {
        auto matchmaking = Memcury::Scanner::FindPattern("83 BD ? ? ? ? 01 7F 18 49 8D 4D D8 48 8B D6 E8 ? ? ? ? 48", false).Get();

        if (!matchmaking)
            matchmaking = Memcury::Scanner::FindPattern("83 7D 88 01 7F 0D 48 8B CE E8", false).Get();
        if (!matchmaking)
            matchmaking = Memcury::Scanner::FindPattern("83 BD ? ? ? ? ? 7F 18 49 8D 4D D8 48 8B D7 E8").Get(); // 4.20

        bool bMatchmakingSupported = matchmaking && Engine_Version >= 420;
        int idx = 0;

        if (bMatchmakingSupported) // now check if it leads to the right place and where the jg is at
        {
            for (int i = 0; i < 9; i++)
            {
                auto byte = (uint8_t*)(matchmaking + i);

                if (IsBadReadPtr(byte))
                    continue;

                // std::cout << std::format("[{}] 0x{:x}\n", i, (int)*byte);

                if (*byte == 0x7F) // jump if greater
                {
                    bMatchmakingSupported = true;
                    idx = i;
                    break;
                }

                bMatchmakingSupported = false;
            }
        }

        LOG_INFO(LogMatchmaker, "Matchmaking will {}", (bMatchmakingSupported ? "be supported" : "not be supported"));

        if (bMatchmakingSupported)
        {
            std::cout << "idx: " << idx << '\n';

            auto before = (uint8_t*)(matchmaking + idx);

            std::cout << "before byte: " << (int)*before << '\n';

            *before = 0x74; // jump if zero
        }
    }

    // return false;

    // UNetDriver::ReplicationDriverOffset = FindOffsetStruct("/Script/Engine.NetDriver", "ReplicationDriver"); // NetDriver->GetOffset("ReplicationDriver");

    // Globals::bAbilitiesEnabled = Engine_Version < 500;

    InitBotNames();

    if (Engine_Version < 420)
    {
        auto ApplyHomebaseEffectsOnPlayerSetupAddr = Memcury::Scanner::FindPattern("40 55 53 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 00 4C 8B").Get();

        Hooking::MinHook::Hook((PVOID)ApplyHomebaseEffectsOnPlayerSetupAddr, ApplyHomebaseEffectsOnPlayerSetupHook, (PVOID*)&ApplyHomebaseEffectsOnPlayerSetupOriginal);
    }

    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameMode.ReadyToStartMatch"), AFortGameModeAthena::Athena_ReadyToStartMatchHook,
       (PVOID*)&AFortGameModeAthena::Athena_ReadyToStartMatchOriginal, false, false, true);

    if (Fortnite_Version > 3.3) // 0xE9 on 3.3 (assumed every build below)
    {
        Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortGameModeAthena.OnAircraftEnteredDropZone"), AFortGameModeAthena::OnAircraftEnteredDropZoneHook,
            (PVOID*)&AFortGameModeAthena::OnAircraftEnteredDropZoneOriginal, false, false, true, true);
    }

    // Hooking::MinHook::Hook(FindObject<UFortServerBotManagerAthena>(L"/Script/FortniteGame.Default__FortServerBotManagerAthena"), FindObject<UFunction>(L"/Script/FortniteGame.FortServerBotManagerAthena.SpawnBot"),
       // UFortServerBotManagerAthena::SpawnBotHook, (PVOID*)&UFortServerBotManagerAthena::SpawnBotOriginal, false);

    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnFor"),
        AGameModeBase::SpawnDefaultPawnForHook, nullptr, false);
    // Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameModeBase.PlayerCanRestart"),
       // AGameModeBase::PlayerCanRestartHook, (PVOID*)&AGameModeBase::PlayerCanRestartOriginal, false, true);
    Hooking::MinHook::Hook(GameModeDefault, FindObject<UFunction>(L"/Script/Engine.GameModeBase.HandleStartingNewPlayer"), AFortGameModeAthena::Athena_HandleStartingNewPlayerHook,
        (PVOID*)&AFortGameModeAthena::Athena_HandleStartingNewPlayerOriginal, false);

    static auto ControllerServerAttemptInteractFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerAttemptInteract");

    if (ControllerServerAttemptInteractFn)
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, ControllerServerAttemptInteractFn, AFortPlayerController::ServerAttemptInteractHook,
            (PVOID*)&AFortPlayerController::ServerAttemptInteractOriginal, false, true);
    }
    else
    {
        Hooking::MinHook::Hook(FindObject(L"/Script/FortniteGame.Default__FortControllerComponent_Interaction"),
            FindObject<UFunction>(L"/Script/FortniteGame.FortControllerComponent_Interaction.ServerAttemptInteract"),
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

    auto OnRep_EditActorFn = FindObject<UFunction>(L"/Script/FortniteGame.FortWeap_EditingTool.OnRep_EditActor");

    if (OnRep_EditActorFn)
    {
        auto OnRep_EditActorExec = (uint64)OnRep_EditActorFn->GetFunc();
        uint64 OnRep_EditActorOriginal = 0;

        for (int i = 0; i < 400; i++)
        {
            if (*(uint8_t*)(OnRep_EditActorExec + i) == 0xE8 || *(uint8_t*)(OnRep_EditActorExec + i) == 0xE9)
            {
                OnRep_EditActorOriginal = Memcury::Scanner(OnRep_EditActorExec + i).RelativeOffset(1).Get();
                break;
            }
        }

        LOG_INFO(LogDev, "OnRep_EditActor Offset: 0x{:x}", OnRep_EditActorOriginal - __int64(GetModuleHandleW(0)));
        AFortWeap_EditingTool::originalOnRep_EditActor = decltype(AFortWeap_EditingTool::originalOnRep_EditActor)(OnRep_EditActorOriginal);
    }

    static auto ServerReturnToMainMenuFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu");
    static auto ServerReturnToMainMenuIdx = GetFunctionIdxOrPtr(ServerReturnToMainMenuFn) / 8;
    auto FortServerRestartPlayer = FortPlayerControllerDefault->VFTable[ServerReturnToMainMenuIdx];
    VirtualSwap(FortPlayerControllerAthenaDefault->VFTable, ServerReturnToMainMenuIdx, FortServerRestartPlayer);

    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerSuicide"),
        AFortPlayerController::ServerSuicideHook, nullptr, false);

    // HookInstruction(Addresses::UpdateTrackedAttributesLea, (PVOID)AFortPlayerControllerAthena::UpdateTrackedAttributesHook, "/Script/Engine.PlayerController.EnableCheats", ERelativeOffsets::LEA, FortPlayerControllerAthenaDefault);
    // HookInstruction(Addresses::CombinePickupLea, (PVOID)AFortPickup::CombinePickupHook, "/Script/Engine.PlayerController.SetVirtualJoystickVisibility", ERelativeOffsets::LEA, FortPlayerControllerAthenaDefault);
   

    if (Fortnite_Version == 13.40)
    {
        // HookInstruction(__int64(GetModuleHandleW(0)) + 0x1FC835D, (PVOID)UFortAthenaAIBotCustomizationData::ApplyOverrideCharacterCustomizationHook, "/Script/Engine.PlayerController.SetVirtualJoystickVisibility", ERelativeOffsets::CALL, nullptr);
    }

    Hooking::MinHook::Hook(FortWeaponDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortWeapon.ServerReleaseWeaponAbility"),
        AFortWeapon::ServerReleaseWeaponAbilityHook, (PVOID*)&AFortWeapon::ServerReleaseWeaponAbilityOriginal, false, true);

    Hooking::MinHook::Hook(FindObject<UKismetSystemLibrary>(L"/Script/Engine.Default__KismetSystemLibrary"), FindObject<UFunction>(L"/Script/Engine.KismetSystemLibrary.PrintString"),
        UKismetSystemLibrary::PrintStringHook, (PVOID*)&UKismetSystemLibrary::PrintStringOriginal, false, true); // todo FortShippingLog

    Hooking::MinHook::Hook((PVOID)Addresses::GetSquadIdForCurrentPlayer, (PVOID)AFortGameSessionDedicatedAthena::GetSquadIdForCurrentPlayerHook);

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
    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerPlaySprayItem"),
        AFortPlayerController::ServerPlaySprayItemHook, nullptr, false); // S4 explain yourself
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

    auto ServerRequestSeatChangeFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange");

    if (ServerRequestSeatChangeFn)
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerZone.ServerRequestSeatChange"),
            AFortPlayerControllerAthena::ServerRequestSeatChangeHook, (PVOID*)&AFortPlayerControllerAthena::ServerRequestSeatChangeOriginal, false);

    // if (false)
    if (Fortnite_Version > 6.20) // so on 6.20 & below there is a param and our little finder dont work for that so
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerGameplay.StartGhostMode"), // (Milxnor) TODO: This changes to a component in later seasons.
            AFortPlayerControllerAthena::StartGhostModeHook, (PVOID*)&AFortPlayerControllerAthena::StartGhostModeOriginal, false, true); // We can exec hook since it only gets called via blueprint.
        
        auto EndGhostModeFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerGameplay.EndGhostMode");

        if (EndGhostModeFn)
        {
            auto EndGhostModeExec = (uint64)EndGhostModeFn->GetFunc();

            for (int i = 0; i < 400; i++)
            {
                if (*(uint8_t*)(EndGhostModeExec + i) == 0xE9 // thanks 6.21
                    || *(uint8_t*)(EndGhostModeExec + i) == 0xE8)
                {
                    Hooking::MinHook::Hook((PVOID)Memcury::Scanner(EndGhostModeExec + i).RelativeOffset(1).Get(), AFortPlayerControllerAthena::EndGhostModeHook, (PVOID*)&AFortPlayerControllerAthena::EndGhostModeOriginal);
                    break;
                }
            }
        }
    }

    Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem"),
        AFortPlayerControllerAthena::ServerGiveCreativeItemHook, nullptr, true);

    static auto ServerCreativeSetFlightSpeedIndexFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerGameplay.ServerCreativeSetFlightSpeedIndex");

    if (ServerCreativeSetFlightSpeedIndexFn)
        Hooking::MinHook::Hook(FindObject<UObject>("/Script/FortniteGame.Default__FortPlayerControllerGameplay"), ServerCreativeSetFlightSpeedIndexFn,
            AFortPlayerControllerAthena::ServerCreativeSetFlightSpeedIndexHook, nullptr, false, true);

    if (Fortnite_Version < 19) // its all screwed up idk
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerPlaySquadQuickChatMessage"),
            AFortPlayerControllerAthena::ServerPlaySquadQuickChatMessageHook, nullptr, false);
    }

    auto ServerTeleportToPlaygroundIslandFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerTeleportToPlaygroundLobbyIsland");

    if (ServerTeleportToPlaygroundIslandFn)
    {
        Hooking::MinHook::Hook(FortPlayerControllerAthenaDefault, ServerTeleportToPlaygroundIslandFn,
            AFortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIslandHook, nullptr, false);
    }

    // Hooking::MinHook::Hook(FortPlayerStateAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.ServerSetInAircraft"),
        // AFortPlayerStateAthena::ServerSetInAircraftHook, (PVOID*)&AFortPlayerStateAthena::ServerSetInAircraftOriginal, false, true); // We could use second method but eh

    if (false && FortOctopusVehicleDefault) // hooking broken on 19.10 i cant figure it out for the life of me
    {
        static auto ServerUpdateTowhookFn = FindObject<UFunction>(L"/Script/FortniteGame.FortOctopusVehicle.ServerUpdateTowhook");
        Hooking::MinHook::Hook(FortOctopusVehicleDefault, ServerUpdateTowhookFn, AFortOctopusVehicle::ServerUpdateTowhookHook, nullptr, false);
    }

    Hooking::MinHook::Hook(FindObject<AFortWeaponRangedMountedCannon>(L"/Script/FortniteGame.Default__FortWeaponRangedMountedCannon"),
        FindObject<UFunction>(L"/Script/FortniteGame.FortWeaponRangedMountedCannon.ServerFireActorInCannon"), AFortWeaponRangedMountedCannon::ServerFireActorInCannonHook, nullptr, false);

    Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState"),
        AFortPlayerPawn::ServerSendZiplineStateHook, nullptr, false);

    static auto NetMulticast_Athena_BatchedDamageCuesFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.NetMulticast_Athena_BatchedDamageCues") : FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.NetMulticast_Athena_BatchedDamageCues");

    if (Fortnite_Version < 21)
    {
        Hooking::MinHook::Hook((PVOID)GetFunctionIdxOrPtr(FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerOnExitVehicle"), true), AFortPlayerPawn::ServerOnExitVehicleHook, (PVOID*)&AFortPlayerPawn::ServerOnExitVehicleOriginal);
    }
    
    if (Fortnite_Version == 1.11 || Fortnite_Version > 1.8)
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerReviveFromDBNO"),
            AFortPlayerPawn::ServerReviveFromDBNOHook, nullptr, false);
    }

    static auto FortGameplayAbilityAthena_PeriodicItemGrantDefault = FindObject<UFortGameplayAbilityAthena_PeriodicItemGrant>(L"/Script/FortniteGame.Default__FortGameplayAbilityAthena_PeriodicItemGrant");

    if (FortGameplayAbilityAthena_PeriodicItemGrantDefault)
    {
        Hooking::MinHook::Hook(FortGameplayAbilityAthena_PeriodicItemGrantDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant.StopItemAwardTimers"),
            UFortGameplayAbilityAthena_PeriodicItemGrant::StopItemAwardTimersHook, (PVOID*)&UFortGameplayAbilityAthena_PeriodicItemGrant::StopItemAwardTimersOriginal, false, true);
        Hooking::MinHook::Hook(FortGameplayAbilityAthena_PeriodicItemGrantDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant.StartItemAwardTimers"),
            UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersHook, (PVOID*)&UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersOriginal, false, true);
    }

    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_Barrier>(L"/Script/FortniteGame.Default__FortAthenaMutator_Barrier"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_Barrier.OnGamePhaseStepChanged"),
        AFortAthenaMutator_Barrier::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_Barrier::OnGamePhaseStepChangedOriginal, false, true);
    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_Disco>(L"/Script/FortniteGame.Default__FortAthenaMutator_Disco"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_Disco.OnGamePhaseStepChanged"),
        AFortAthenaMutator_Disco::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_Disco::OnGamePhaseStepChangedOriginal, false, true);
    Hooking::MinHook::Hook(FindObject<AFortAthenaMutator_GiveItemsAtGamePhaseStep>(L"/Script/FortniteGame.Default__FortAthenaMutator_GiveItemsAtGamePhaseStep"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_GiveItemsAtGamePhaseStep.OnGamePhaseStepChanged"),
        AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChangedHook, (PVOID*)&AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChangedOriginal, false, true);

    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.GetAIDirector"),
        UFortKismetLibrary::GetAIDirectorHook, (PVOID*)&UFortKismetLibrary::GetAIDirectorOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.GetAIGoalManager"),
        UFortKismetLibrary::GetAIGoalManagerHook, (PVOID*)&UFortKismetLibrary::GetAIGoalManagerOriginal, false, true);
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
    if (Fortnite_Version < 20) // todo
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.OnCapsuleBeginOverlap") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawnAthena.OnCapsuleBeginOverlap") : FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.OnCapsuleBeginOverlap"),
            AFortPlayerPawnAthena::OnCapsuleBeginOverlapHook, (PVOID*)&AFortPlayerPawnAthena::OnCapsuleBeginOverlapOriginal, false, true);
    }


    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_RemoveFortItemFromPlayer"),
        UFortKismetLibrary::K2_RemoveFortItemFromPlayerHook, (PVOID*)&UFortKismetLibrary::K2_RemoveFortItemFromPlayerOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorld"),
        UFortKismetLibrary::K2_SpawnPickupInWorldHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithLootTier"),
        UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierOriginal, false, true);
    Hooking::MinHook::Hook(FortKismetLibraryDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithClass"),
        UFortKismetLibrary::K2_SpawnPickupInWorldWithClassHook, (PVOID*)&UFortKismetLibrary::K2_SpawnPickupInWorldWithClassOriginal, false, true);

    // if (Addresses::FreeArrayOfEntries || Addresses::FreeEntry)
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

    Hooking::MinHook::Hook(FindObject(L"/Script/FortniteGame.Default__FortAthenaVehicleSpawner"), FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaVehicleSpawner.SpawnVehicle"),
        AFortAthenaVehicleSpawner::SpawnVehicleHook, nullptr, false);

    static auto ServerHandlePickupInfoFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupInfo");

    if (ServerHandlePickupInfoFn)
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, ServerHandlePickupInfoFn, AFortPlayerPawn::ServerHandlePickupInfoHook, nullptr, false);
    }
    else
    {
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickup"),
            AFortPlayerPawn::ServerHandlePickupHook, nullptr, false);
        Hooking::MinHook::Hook(FortPlayerPawnAthenaDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerPawn.ServerHandlePickupWithRequestedSwap"),
            AFortPlayerPawn::ServerHandlePickupWithRequestedSwapHook, (PVOID*)&AFortPlayerPawn::ServerHandlePickupWithRequestedSwapOriginal, false, true);
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

    uint64 ServerRemoveInventoryItemFunctionCallBeginFunctionAddr = 0;

    // if (Engine_Version >= 419)
    if (Fortnite_Version < 20)
    {
        std::vector<uint8_t> ServerRemoveInventoryItemCallFunctionStarts = Engine_Version == 416 
            ? std::vector<uint8_t>{ 0x44, 0x88, 0x4C } 
            : Fortnite_Version >= 16 
                ? std::vector<uint8_t>{ 0x48, 0x8B, 0xC4 }
            : std::vector<uint8_t>{ 0x48, 0x89, 0x5C };

        auto ServerRemoveInventoryItemCallFunctionCall = FindFunctionCall(L"ServerRemoveInventoryItem", ServerRemoveInventoryItemCallFunctionStarts);
        auto ServerRemoveInventoryItemFunctionCallRef = Memcury::Scanner::FindPointerRef((PVOID)ServerRemoveInventoryItemCallFunctionCall, 0, true);

        LOG_INFO(LogDev, "ServerRemoveInventoryItemFunctionCallRef: 0x{:x}", ServerRemoveInventoryItemFunctionCallRef.Get() - __int64(GetModuleHandleW(0)));

        for (int i = 0; i < 400; i++)
        {
            // LOG_INFO(LogDev, "[{}] Bugha: 0x{:x}", i, (int)(*(uint8_t*)ServerRemoveInventoryItemFunctionCallRef.Get() - i));
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
    }

    Hooking::MinHook::Hook(Memcury::Scanner(ServerRemoveInventoryItemFunctionCallBeginFunctionAddr).GetAs<PVOID>(), UFortInventoryInterface::RemoveInventoryItemHook);
   
    // if (Fortnite_Version >= 13)
    Hooking::MinHook::Hook((PVOID)Addresses::SetZoneToIndex, (PVOID)SetZoneToIndexHook, (PVOID*)&SetZoneToIndexOriginal);
    Hooking::MinHook::Hook((PVOID)Addresses::EnterAircraft, (PVOID)AFortPlayerControllerAthena::EnterAircraftHook, (PVOID*)&AFortPlayerControllerAthena::EnterAircraftOriginal);

    AddVehicleHook();

    auto ClientOnPawnDiedCallAddr = FindFunctionCall(L"ClientOnPawnDied", Engine_Version == 416 ? std::vector<uint8_t>{ 0x48, 0x89, 0x54 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C });
    LOG_INFO(LogDev, "ClientOnPawnDiedCallAddr: 0x{:x}", ClientOnPawnDiedCallAddr - __int64(GetModuleHandleW(0)));
    Hooking::MinHook::Hook((PVOID)ClientOnPawnDiedCallAddr, AFortPlayerController::ClientOnPawnDiedHook, (PVOID*)&AFortPlayerController::ClientOnPawnDiedOriginal);

    auto OnSafeZoneStateChangeAddr = FindFunctionCall(L"OnSafeZoneStateChange", Engine_Version == 416 ? std::vector<uint8_t>{ 0x48, 0x89, 0x54 } : std::vector<uint8_t>{ 0x48, 0x89, 0x5C });
    LOG_INFO(LogDev, "OnSafeZoneStateChangeAddr: 0x{:x}", OnSafeZoneStateChangeAddr - __int64(GetModuleHandleW(0)));
    Hooking::MinHook::Hook((PVOID)OnSafeZoneStateChangeAddr, AFortSafeZoneIndicator::OnSafeZoneStateChangeHook, (PVOID*)&AFortSafeZoneIndicator::OnSafeZoneStateChangeOriginal);

    LOG_INFO(LogDev, "PredictionKeySize: 0x{:x} {}", PredictionKeySize, PredictionKeySize);

    static auto GameplayEventDataSize = FindObject<UStruct>(L"/Script/GameplayAbilities.GameplayEventData")->GetPropertiesSize();
    LOG_INFO(LogDev, "GameplayEventDataSize: 0x{:x} {}", GameplayEventDataSize, GameplayEventDataSize);

    {
        int increaseOffset = 0x10;

        if (Fortnite_Version > 11.00 && std::floor(Fortnite_Version) < 18) // checked on 11.00, 11.31, 12.41, 14.60, 15.10, 16.40, 17.30 and 18.40
            increaseOffset += 0x8;

        auto MoveSoundStimulusBroadcastIntervalOffset = FindOffsetStruct("/Script/FortniteGame.FortPlayerPawn", "MoveSoundStimulusBroadcastInterval");
        MemberOffsets::FortPlayerPawn::CorrectTags = MoveSoundStimulusBroadcastIntervalOffset + increaseOffset;
        LOG_INFO(LogDev, "CorrectTags: 0x{:x}", MemberOffsets::FortPlayerPawn::CorrectTags);
        MemberOffsets::FortPlayerState::PawnDeathLocation = FindOffsetStruct("/Script/FortniteGame.FortPlayerState", "PawnDeathLocation", false);

        MemberOffsets::FortPlayerPawnAthena::LastFallDistance = FindOffsetStruct("/Script/FortniteGame.FortPlayerPawnAthena", "LastFallDistance", false);

        MemberOffsets::FortPlayerStateAthena::DeathInfo = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "DeathInfo");
        MemberOffsets::FortPlayerStateAthena::KillScore = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "KillScore", false);
        MemberOffsets::FortPlayerStateAthena::TeamKillScore = FindOffsetStruct("/Script/FortniteGame.FortPlayerStateAthena", "TeamKillScore", false);

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

    LOG_INFO(LogHook, "Finished initialization!");

    if (false)
    {
        while (true)
        {
            Sleep(10000);
        }
    }
    else
    {
        Sleep(-1);
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

