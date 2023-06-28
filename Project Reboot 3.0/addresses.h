#pragma once

// #include "finder.h"
#include "log.h"
#include "inc.h"

namespace Addresses
{
	extern inline uint64 StaticFindObject = 0;
	extern inline uint64 StaticLoadObject = 0;
	extern inline uint64 ObjectArray = 0;
	extern inline uint64 InitListen = 0;
	extern inline uint64 CreateNetDriver = 0;
	extern inline uint64 SetWorld = 0;
	extern inline uint64 ProcessEvent = 0;
	extern inline uint64 PickupDelay = 0;
	extern inline uint64 GetMaxTickRate = 0;
	extern inline uint64 GetPlayerViewpoint = 0;
	extern inline uint64 InitHost = 0;
	extern inline uint64 PauseBeaconRequests = 0;
	extern inline uint64 SpawnActor = 0;
	extern inline uint64 KickPlayer = 0;
	extern inline uint64 TickFlush = 0;
	extern inline uint64 GetNetMode = 0;
	extern inline uint64 Realloc = 0;
	extern inline uint64 CollectGarbage = 0;
	extern inline uint64 NoMCP = 0;
	extern inline uint64 PickTeam = 0;
	extern inline uint64 InternalTryActivateAbility = 0;
	extern inline uint64 GiveAbility = 0;
	extern inline uint64 CantBuild = 0;
	extern inline uint64 ReplaceBuildingActor = 0;
	extern inline uint64 GiveAbilityAndActivateOnce = 0;
	extern inline uint64 OnDamageServer = 0;
	extern inline uint64 GIsServer = 0;
	extern inline uint64 GIsClient = 0;
	extern inline uint64 ActorGetNetMode = 0;
	extern inline uint64 ChangeGameSessionId = 0;
	extern inline uint64 DispatchRequest = 0;
	extern inline uint64 AddNavigationSystemToWorld = 0;
	extern inline uint64 NavSystemCleanUp = 0;
	extern inline uint64 LoadPlayset = 0;
	extern inline uint64 SetZoneToIndex = 0;
	extern inline uint64 CompletePickupAnimation = 0;
	extern inline uint64 CanActivateAbility = 0;
	extern inline uint64 SpecConstructor = 0;
	extern inline uint64 ReplicateActor = 0;
	extern inline uint64 CallPreReplication = 0;
	extern inline uint64 CreateChannel = 0;
	extern inline uint64 SetChannelActor = 0;
	extern inline uint64 SendClientAdjustment = 0;
	extern inline uint64 FrameStep = 0;
	extern inline uint64 OnRep_ZiplineState = 0;
	extern inline uint64 RemoveFromAlivePlayers = 0;
	extern inline uint64 ActorChannelClose = 0;
	extern inline uint64 FrameStepExplicitProperty = 0;
	extern inline uint64 Free = 0;
	extern inline uint64 ClearAbility = 0;
	extern inline uint64 ApplyGadgetData = 0;
	extern inline uint64 RemoveGadgetData = 0;
	extern inline uint64 ApplyCharacterCustomization = 0;
	extern inline uint64 GetInterfaceAddress = 0;
	extern inline uint64 EnterAircraft = 0;
	extern inline uint64 SetTimer = 0;
	extern inline uint64 PickupInitialize = 0;
	extern inline uint64 FreeEntry = 0;
	extern inline uint64 FreeArrayOfEntries = 0;
	extern inline uint64 UpdateTrackedAttributesLea = 0;
	extern inline uint64 CombinePickupLea = 0;
	extern inline uint64 CreateBuildingActorCallForDeco = 0;
	extern inline uint64 PickSupplyDropLocation = 0;
	extern inline uint64 LoadAsset = 0;
	extern inline uint64 RebootingDelegate = 0;
	extern inline uint64 GetSquadIdForCurrentPlayer = 0;
	extern inline uint64 FinishResurrection = 0;
	extern inline uint64 AddToAlivePlayers = 0;
	extern inline uint64 GameSessionPatch = 0;
	extern inline uint64 GetSessionInterface = 0; // Matchmaking
	extern inline uint64 StartAircraftPhase = 0;

	void SetupVersion(); // Finds Engine Version
	void FindAll();
	void Print();
	void Init();
	
	std::vector<uint64> GetFunctionsToReturnTrue();
	std::vector<uint64> GetFunctionsToNull();
}

namespace Offsets
{
	extern inline uint64 Func = 0;
	extern inline uint64 PropertiesSize = 0;
	extern inline uint64 Children = 0;
	extern inline uint64 SuperStruct = 0;
	extern inline uint64 Offset_Internal = 0;
	extern inline uint64 ServerReplicateActors = 0;
	extern inline uint64 ReplicationFrame = 0;
	extern inline uint64 IsNetRelevantFor = 0;
	extern inline uint64 NetworkObjectList = 0;
	extern inline uint64 ClientWorldPackageName = 0;
	extern inline uint64 Script = 0;
	extern inline uint64 PropertyClass = 0;

	void FindAll();
	void Print();
}