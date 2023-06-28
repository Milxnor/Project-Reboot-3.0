#include "addresses.h"

#include "UObjectGlobals.h"
#include "World.h"
#include "NetDriver.h"
#include "GameSession.h"

#include "NetSerialization.h"

#include "Array.h"
#include "AbilitySystemComponent.h"

#include "finder.h"
#include <regex>

#include "ai.h"
#include "BuildingActor.h"
#include "FortPlaysetItemDefinition.h"
#include "FortGameModeAthena.h"
#include "UObjectArray.h"

void Addresses::SetupVersion()
{
	static FString(*GetEngineVersion)() = decltype(GetEngineVersion)(Memcury::Scanner::FindPattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C8 41 B8 04 ? ? ? 48 8B D3", false).Get());

	std::string FullVersion;
	FString toFree;

	if (!GetEngineVersion)
	{
		auto VerStr = Memcury::Scanner::FindPattern("2B 2B 46 6F 72 74 6E 69 74 65 2B 52 65 6C 65 61 73 65 2D ? ? ? ?").Get();

		// if (!VerStr)

		FullVersion = decltype(FullVersion.c_str())(VerStr);
		Engine_Version = 500;
	}

	else
	{
		toFree = GetEngineVersion();
		FullVersion = toFree.ToString();
	}

	std::string FNVer = FullVersion;
	std::string EngineVer = FullVersion;
	std::string CLStr;

	if (!FullVersion.contains("Live") && !FullVersion.contains(("Next")) && !FullVersion.contains(("Cert")))
	{
		if (GetEngineVersion)
		{
			FNVer.erase(0, FNVer.find_last_of(("-"), FNVer.length() - 1) + 1);
			EngineVer.erase(EngineVer.find_first_of(("-"), FNVer.length() - 1), 40);

			if (EngineVer.find_first_of(".") != EngineVer.find_last_of(".")) // this is for 4.21.0 and itll remove the .0
				EngineVer.erase(EngineVer.find_last_of((".")), 2);

			Engine_Version = std::stod(EngineVer) * 100;
		}

		else
		{
			const std::regex base_regex(("-([0-9.]*)-"));
			std::cmatch base_match;

			std::regex_search(FullVersion.c_str(), base_match, base_regex);

			FNVer = base_match[1];
		}

		Fortnite_Version = std::stod(FNVer);

		if (Fortnite_Version >= 16.00 && Fortnite_Version <= 18.40)
			Engine_Version = 427; // 4.26.1;
	}

	else
	{
		// TODO
		// Engine_Version = FullVersion.contains(("Next")) ? 419 : 416;
		CLStr = FullVersion.substr(FullVersion.find_first_of('-') + 1);
		CLStr = CLStr.substr(0, CLStr.find_first_of('+'));
		Fortnite_CL = std::stoi(CLStr);
		Engine_Version = Fortnite_CL <= 3775276 ? 416 : 419; // std::stoi(FullVersion.substr(0, FullVersion.find_first_of('-')));
		// Fortnite_Version = FullVersion.contains(("Next")) ? 2.4 : 1.8;
	}

	// Fortnite_Season = std::floor(Fortnite_Version);

	FFastArraySerializer::bNewSerializer = Fortnite_Version >= 8.30;

	if (Fortnite_CL == 3807424)
		Fortnite_Version = 1.11;
	if (Fortnite_CL == 3700114)
		Fortnite_Version = 1.72;
	if (Fortnite_CL == 3724489)
		Fortnite_Version = 1.8;
	if (Fortnite_CL == 3757339)
		Fortnite_Version = 1.9;
	if (Fortnite_CL == 3841827)
		Fortnite_Version = 2.2;
	if (Fortnite_CL == 3847564)
		Fortnite_Version = 2.3;
	if (Fortnite_CL == 3858292)
		Fortnite_Version = 2.4;
	if (Fortnite_CL == 3870737)
		Fortnite_Version = 2.42;

	toFree.Free();
}

void Addresses::FindAll()
{
	auto Base = __int64(GetModuleHandleW(0));

	LOG_INFO(LogDev, "Finding ProcessEvent");
	Addresses::ProcessEvent = FindProcessEvent();
	UObject::ProcessEventOriginal = decltype(UObject::ProcessEventOriginal)(ProcessEvent);
	LOG_INFO(LogDev, "Finding StaticFindObject");

	Addresses::StaticFindObject = FindStaticFindObject();
	StaticFindObjectOriginal = decltype(StaticFindObjectOriginal)(StaticFindObject);
	LOG_INFO(LogDev, "StaticFindObject: 0x{:x}", StaticFindObject - Base);
	LOG_INFO(LogDev, "Finding GetPlayerViewpoint");

	Addresses::GetPlayerViewpoint = FindGetPlayerViewpoint();
	LOG_INFO(LogDev, "Finding CreateNetDriver");

	Addresses::CreateNetDriver = FindCreateNetDriver();
	LOG_INFO(LogDev, "Finding InitHost");

	Addresses::InitHost = FindInitHost();
	LOG_INFO(LogDev, "Finding PauseBeaconRequests");

	Addresses::PauseBeaconRequests = FindPauseBeaconRequests();
	LOG_INFO(LogDev, "Finding SpawnActor");

	Addresses::SpawnActor = FindSpawnActor();
	LOG_INFO(LogDev, "Finding InitListen");

	Addresses::InitListen = FindInitListen();
	LOG_INFO(LogDev, "Finding SetWorld");

	Addresses::SetWorld = FindSetWorld();
	LOG_INFO(LogDev, "Finding KickPlayer");

	Addresses::KickPlayer = FindKickPlayer();
	LOG_INFO(LogDev, "Finding TickFlush");

	Addresses::TickFlush = FindTickFlush();
	LOG_INFO(LogDev, "Finding GetNetMode");

	Addresses::GetNetMode = FindGetNetMode();
	LOG_INFO(LogDev, "Finding Realloc");

	Addresses::Realloc = FindRealloc();
	LOG_INFO(LogDev, "Finding CollectGarbage");

	Addresses::CollectGarbage = FindCollectGarbage();
	LOG_INFO(LogDev, "Finding NoMCP");

	Addresses::NoMCP = FindNoMCP();
	LOG_INFO(LogDev, "Finding PickTeam");

	Addresses::PickTeam = FindPickTeam();
	LOG_INFO(LogDev, "Finding InternalTryActivateAbility");

	Addresses::InternalTryActivateAbility = FindInternalTryActivateAbility();
	LOG_INFO(LogDev, "Finding GiveAbility");

	Addresses::GiveAbility = FindGiveAbility();
	LOG_INFO(LogDev, "Finding CantBuild");

	Addresses::CantBuild = FindCantBuild();
	LOG_INFO(LogDev, "Finding ReplaceBuildingActor");

	Addresses::ReplaceBuildingActor = FindReplaceBuildingActor();
	LOG_INFO(LogDev, "Finding GiveAbilityAndActivateOnce");

	Addresses::GiveAbilityAndActivateOnce = FindGiveAbilityAndActivateOnce();
	LOG_INFO(LogDev, "Finding OnDamageServer");

	Addresses::OnDamageServer = FindOnDamageServer();
	LOG_INFO(LogDev, "Finding StaticLoadObject");

	Addresses::StaticLoadObject = FindStaticLoadObject();
	LOG_INFO(LogDev, "Finding ActorGetNetMode");

	Addresses::ActorGetNetMode = FindActorGetNetMode();
	LOG_INFO(LogDev, "Finding ChangeGameSessionId");

	Addresses::ChangeGameSessionId = FindChangeGameSessionId();
	LOG_INFO(LogDev, "Finding DispatchRequest");

	Addresses::DispatchRequest = FindDispatchRequest();
	LOG_INFO(LogDev, "Finding AddNavigationSystemToWorld");

	Addresses::AddNavigationSystemToWorld = FindAddNavigationSystemToWorld();
	LOG_INFO(LogDev, "Finding NavSystemCleanUp");

	Addresses::NavSystemCleanUp = FindNavSystemCleanUp();
	LOG_INFO(LogDev, "Finding LoadPlayset");

	Addresses::LoadPlayset = FindLoadPlayset();
	LOG_INFO(LogDev, "Finding SetZoneToIndex");

	Addresses::SetZoneToIndex = FindSetZoneToIndex();
	LOG_INFO(LogDev, "Finding CompletePickupAnimation");

	Addresses::CompletePickupAnimation = FindCompletePickupAnimation();
	LOG_INFO(LogDev, "Finding CanActivateAbility");

	Addresses::CanActivateAbility = FindCanActivateAbility();
	LOG_INFO(LogDev, "Finding SpecConstructor");

	Addresses::SpecConstructor = FindSpecConstructor();
	LOG_INFO(LogDev, "Finding FrameStep");

	Addresses::FrameStep = FindFrameStep();
	LOG_INFO(LogDev, "Finding ObjectArray");

	Addresses::ObjectArray = FindObjectArray();
	LOG_INFO(LogDev, "Finding ReplicateActor");

	Addresses::ReplicateActor = FindReplicateActor();
	LOG_INFO(LogDev, "Finding SetChannelActor");

	Addresses::SetChannelActor = FindSetChannelActor();
	LOG_INFO(LogDev, "Finding SendClientAdjustment");

	Addresses::SendClientAdjustment = FindSendClientAdjustment();
	LOG_INFO(LogDev, "Finding CreateChannel");

	Addresses::CreateChannel = FindCreateChannel();
	LOG_INFO(LogDev, "Finding CallPreReplication");

	Addresses::CallPreReplication = FindCallPreReplication();
	LOG_INFO(LogDev, "Finding OnRep_ZiplineState");

	Addresses::OnRep_ZiplineState = FindOnRep_ZiplineState();
	LOG_INFO(LogDev, "Finding GetMaxTickRate");

	Addresses::GetMaxTickRate = FindGetMaxTickRate();

	LOG_INFO(LogDev, "Finding RemoveFromAlivePlayers");
	Addresses::RemoveFromAlivePlayers = FindRemoveFromAlivePlayers();

	LOG_INFO(LogDev, "Finding ActorChannelClose");
	Addresses::ActorChannelClose = FindActorChannelClose();

	LOG_INFO(LogDev, "Finding StepExplicitProperty");
	Addresses::FrameStepExplicitProperty = FindStepExplicitProperty();

	LOG_INFO(LogDev, "Finding Free");
	Addresses::Free = FindFree();

	LOG_INFO(LogDev, "Finding ClearAbility");
	Addresses::ClearAbility = FindClearAbility();

	LOG_INFO(LogDev, "Finding ApplyGadgetData");
	Addresses::ApplyGadgetData = FindApplyGadgetData();

	LOG_INFO(LogDev, "Finding RemoveGadgetData");
	Addresses::RemoveGadgetData = FindRemoveGadgetData();

	LOG_INFO(LogDev, "Finding GetInterfaceAddress");
	Addresses::GetInterfaceAddress = FindGetInterfaceAddress();

	LOG_INFO(LogDev, "Finding ApplyCharacterCustomization");
	Addresses::ApplyCharacterCustomization = FindApplyCharacterCustomization();

	LOG_INFO(LogDev, "Finding EnterAircraft");
	Addresses::EnterAircraft = FindEnterAircraft();

	LOG_INFO(LogDev, "Finding SetTimer");
	Addresses::SetTimer = FindSetTimer();

	LOG_INFO(LogDev, "Finding PickupInitialize");
	Addresses::PickupInitialize = FindPickupInitialize();

	LOG_INFO(LogDev, "Finding FreeEntry");
	Addresses::FreeEntry = FindFreeEntry();

	LOG_INFO(LogDev, "Finding FreeArrayOfEntries");
	Addresses::FreeArrayOfEntries = FindFreeArrayOfEntries();

	LOG_INFO(LogDev, "Finding UpdateTrackedAttributesLea");
	Addresses::UpdateTrackedAttributesLea = FindUpdateTrackedAttributesLea();

	LOG_INFO(LogDev, "Finding CombinePickupLea");
	Addresses::CombinePickupLea = FindCombinePickupLea();

	LOG_INFO(LogDev, "Finding CreateBuildingActorCallForDeco");
	Addresses::CreateBuildingActorCallForDeco = FindCreateBuildingActorCallForDeco();

	LOG_INFO(LogDev, "Finding PickSupplyDropLocation");
	Addresses::PickSupplyDropLocation = FindPickSupplyDropLocation();

	LOG_INFO(LogDev, "Finding LoadAsset");
	Addresses::LoadAsset = FindLoadAsset();

	LOG_INFO(LogDev, "Finding RebootingDelegate");
	Addresses::RebootingDelegate = FindRebootingDelegate();

	LOG_INFO(LogDev, "Finding GetSquadIdForCurrentPlayer");
	Addresses::GetSquadIdForCurrentPlayer = FindGetSquadIdForCurrentPlayer();

	LOG_INFO(LogDev, "Finding FinishResurrection");
	Addresses::FinishResurrection = FindFinishResurrection();

	LOG_INFO(LogDev, "Finding AddToAlivePlayers");
	Addresses::AddToAlivePlayers = FindAddToAlivePlayers();

	LOG_INFO(LogDev, "Finding StartAircraftPhase");
	Addresses::StartAircraftPhase = FindStartAircraftPhase();

	// LOG_INFO(LogDev, "Finding GetSessionInterface");
	// Addresses::GetSessionInterface = FindGetSessionInterface();

	LOG_INFO(LogDev, "Applying GameSessionPatch");
	ApplyGameSessionPatch();

	LOG_INFO(LogDev, "Finished finding!");
}

void Addresses::Print()
{
	auto Base = __int64(GetModuleHandleW(0));

	LOG_INFO(LogDev, "Base: 0x{:x}", Base);
	LOG_INFO(LogDev, "ProcessEvent: 0x{:x}", ProcessEvent - Base);
	LOG_INFO(LogDev, "StaticFindObject: 0x{:x}", StaticFindObject - Base);
	LOG_INFO(LogDev, "GetPlayerViewpoint: 0x{:x}", GetPlayerViewpoint - Base);
	LOG_INFO(LogDev, "CreateNetDriver: 0x{:x}", CreateNetDriver - Base);
	LOG_INFO(LogDev, "InitHost: 0x{:x}", InitHost - Base);
	LOG_INFO(LogDev, "PauseBeaconRequests: 0x{:x}", PauseBeaconRequests - Base);
	LOG_INFO(LogDev, "SpawnActor: 0x{:x}", SpawnActor - Base);
	LOG_INFO(LogDev, "InitListen: 0x{:x}", InitListen - Base);
	LOG_INFO(LogDev, "SetWorld: 0x{:x}", SetWorld - Base);
	LOG_INFO(LogDev, "KickPlayer: 0x{:x}", KickPlayer - Base);
	LOG_INFO(LogDev, "TickFlush: 0x{:x}", TickFlush - Base);
	LOG_INFO(LogDev, "GetNetMode: 0x{:x}", GetNetMode - Base);
	LOG_INFO(LogDev, "Realloc: 0x{:x}", Realloc - Base);
	LOG_INFO(LogDev, "CollectGarbage: 0x{:x}", CollectGarbage - Base);
	LOG_INFO(LogDev, "NoMCP: 0x{:x}", NoMCP - Base);
	LOG_INFO(LogDev, "PickTeam: 0x{:x}", PickTeam - Base);
	LOG_INFO(LogDev, "InternalTryActivateAbility: 0x{:x}", InternalTryActivateAbility - Base);
	LOG_INFO(LogDev, "GiveAbility: 0x{:x}", GiveAbility - Base);
	LOG_INFO(LogDev, "CantBuild: 0x{:x}", CantBuild - Base);
	LOG_INFO(LogDev, "ReplaceBuildingActor: 0x{:x}", ReplaceBuildingActor - Base);
	LOG_INFO(LogDev, "GiveAbilityAndActivateOnce: 0x{:x}", GiveAbilityAndActivateOnce - Base);
	LOG_INFO(LogDev, "OnDamageServer: 0x{:x}", OnDamageServer - Base);
	LOG_INFO(LogDev, "StaticLoadObject: 0x{:x}", StaticLoadObject - Base);
	LOG_INFO(LogDev, "ActorGetNetMode: 0x{:x}", ActorGetNetMode - Base);
	LOG_INFO(LogDev, "ChangeGameSessionId: 0x{:x}", ChangeGameSessionId - Base);
	LOG_INFO(LogDev, "DispatchRequest: 0x{:x}", DispatchRequest - Base);
	LOG_INFO(LogDev, "AddNavigationSystemToWorld: 0x{:x}", AddNavigationSystemToWorld - Base);
	LOG_INFO(LogDev, "NavSystemCleanUp: 0x{:x}", NavSystemCleanUp - Base);
	LOG_INFO(LogDev, "LoadPlayset: 0x{:x}", LoadPlayset - Base);
	LOG_INFO(LogDev, "SetZoneToIndex: 0x{:x}", SetZoneToIndex - Base);
	LOG_INFO(LogDev, "CompletePickupAnimation: 0x{:x}", CompletePickupAnimation - Base);
	LOG_INFO(LogDev, "CanActivateAbility: 0x{:x}", CanActivateAbility - Base);
	LOG_INFO(LogDev, "SpecConstructor: 0x{:x}", SpecConstructor - Base);
	LOG_INFO(LogDev, "FrameStep: 0x{:x}", FrameStep - Base);
	LOG_INFO(LogDev, "ObjectArray: 0x{:x}", ObjectArray - Base);
	LOG_INFO(LogDev, "ReplicateActor: 0x{:x}", ReplicateActor - Base);
	LOG_INFO(LogDev, "SetChannelActor: 0x{:x}", SetChannelActor - Base);
	LOG_INFO(LogDev, "SendClientAdjustment: 0x{:x}", SendClientAdjustment - Base);
	LOG_INFO(LogDev, "CreateChannel: 0x{:x}", CreateChannel - Base);
	LOG_INFO(LogDev, "CallPreReplication: 0x{:x}", CallPreReplication - Base);
	LOG_INFO(LogDev, "OnRep_ZiplineState: 0x{:x}", OnRep_ZiplineState - Base);
	LOG_INFO(LogDev, "GetMaxTickRate: 0x{:x}", GetMaxTickRate - Base);
	LOG_INFO(LogDev, "RemoveFromAlivePlayers: 0x{:x}", RemoveFromAlivePlayers - Base);
	LOG_INFO(LogDev, "ActorChannelClose: 0x{:x}", ActorChannelClose - Base);
	LOG_INFO(LogDev, "FrameStepExplicitProperty: 0x{:x}", FrameStepExplicitProperty - Base);
	LOG_INFO(LogDev, "Free: 0x{:x}", Free - Base);
	LOG_INFO(LogDev, "ClearAbility: 0x{:x}", ClearAbility - Base);
	LOG_INFO(LogDev, "ApplyGadgetData: 0x{:x}", ApplyGadgetData - Base);
	LOG_INFO(LogDev, "RemoveGadgetData: 0x{:x}", RemoveGadgetData - Base);
	LOG_INFO(LogDev, "GetInterfaceAddress: 0x{:x}", GetInterfaceAddress - Base);
	LOG_INFO(LogDev, "ApplyCharacterCustomization: 0x{:x}", ApplyCharacterCustomization - Base);
	LOG_INFO(LogDev, "EnterAircraft: 0x{:x}", EnterAircraft - Base);
	LOG_INFO(LogDev, "SetTimer: 0x{:x}", SetTimer - Base);
	LOG_INFO(LogDev, "PickupInitialize: 0x{:x}", PickupInitialize - Base);
	LOG_INFO(LogDev, "FreeEntry: 0x{:x}", FreeEntry - Base);
	LOG_INFO(LogDev, "FreeArrayOfEntries: 0x{:x}", FreeArrayOfEntries - Base);
	LOG_INFO(LogDev, "UpdateTrackedAttributesLea: 0x{:x}", UpdateTrackedAttributesLea - Base);
	LOG_INFO(LogDev, "CombinePickupLea: 0x{:x}", CombinePickupLea - Base);
	LOG_INFO(LogDev, "CreateBuildingActorCallForDeco: 0x{:x}", CreateBuildingActorCallForDeco - Base);
	LOG_INFO(LogDev, "PickSupplyDropLocation: 0x{:x}", PickSupplyDropLocation - Base);
	LOG_INFO(LogDev, "LoadAsset: 0x{:x}", LoadAsset - Base);
	LOG_INFO(LogDev, "RebootingDelegate: 0x{:x}", RebootingDelegate - Base);
	LOG_INFO(LogDev, "GetSquadIdForCurrentPlayer: 0x{:x}", GetSquadIdForCurrentPlayer - Base);
	LOG_INFO(LogDev, "FinishResurrection: 0x{:x}", FinishResurrection - Base);
	LOG_INFO(LogDev, "AddToAlivePlayers: 0x{:x}", AddToAlivePlayers - Base);
	LOG_INFO(LogDev, "GetSessionInterface: 0x{:x}", GetSessionInterface - Base);
	LOG_INFO(LogDev, "StartAircraftPhase: 0x{:x}", StartAircraftPhase - Base);
}

void Offsets::FindAll()
{
	Offsets::Offset_Internal = Fortnite_Version >= 12.10 && std::floor(Fortnite_Version) < 20 ? 0x4C : 0x44;
	Offsets::SuperStruct = Engine_Version >= 422 ? 0x40 : 0x30;
	Offsets::Children = Fortnite_Version >= 12.10 ? 0x50 : Offsets::SuperStruct + 8;
	Offsets::PropertiesSize = Offsets::Children + 8;

	if (Engine_Version >= 416 && Engine_Version <= 421)
		Offsets::Func = 0xB0;
	else if (Engine_Version >= 422 && Engine_Version <= 424)
		Offsets::Func = 0xC0;
	else if (Fortnite_Version >= 12.00 && Fortnite_Version < 12.10)
		Offsets::Func = 0xC8;
	else if (Engine_Version == 425)
		Offsets::Func = 0xF0;
	else if (Engine_Version >= 426)
		Offsets::Func = 0xD8;

	if (Engine_Version == 420)
		Offsets::ServerReplicateActors = 0x53;
	else if (Engine_Version == 421)
		Offsets::ServerReplicateActors = std::floor(Fortnite_Version) == 5 ? 0x54 : 0x56;
	else if (Engine_Version >= 422 && Engine_Version <= 424)
		Offsets::ServerReplicateActors = Fortnite_Version >= 7.40 && Fortnite_Version < 8.40 ? 0x57 :
		Engine_Version == 424 ? (Fortnite_Version >= 11.00 && Fortnite_Version <= 11.10 ? 0x57 :
			(Fortnite_Version == 11.30 || Fortnite_Version == 11.31 ? 0x59 : 0x5A)) : 0x56;

	// ^ I know this makes no sense, 7.40-8.40 is 0x57, other 7-10 is 0x56, 11.00-11.10 = 0x57, 11.30-11.31 = 0x59, other S11 is 0x5A

	else if (std::floor(Fortnite_Version) == 12 || std::floor(Fortnite_Version) == 13)
		Offsets::ServerReplicateActors = 0x5D;
	else if (std::floor(Fortnite_Version) == 14 || Fortnite_Version <= 15.2) // never tested 15.2
		Offsets::ServerReplicateActors = 0x5E;
	else if (Fortnite_Version >= 15.3 && Engine_Version < 500) // 15.3-18 = 0x5F
		Offsets::ServerReplicateActors = 0x5F;
	else if (std::floor(Fortnite_Version) >= 19 && std::floor(Fortnite_Version) <= 20)
		Offsets::ServerReplicateActors = 0x66;
	else if (std::floor(Fortnite_Version) >= 21)
		Offsets::ServerReplicateActors = 0x67; // checked onb 22.30

	if (Engine_Version == 416) // checked on 1.7.2 & 1.8 & 1.9
	{
		Offsets::NetworkObjectList = 0x3F8;
		Offsets::ReplicationFrame = 0x288;
	}
	if (Fortnite_Version == 1.72)
	{
		Offsets::ClientWorldPackageName = 0x336A8;
	}
	if (Fortnite_Version == 1.8 || Fortnite_Version == 1.9)
	{
		Offsets::ClientWorldPackageName = 0x33788;
	}
	if (Fortnite_Version == 1.11)
	{
		Offsets::ClientWorldPackageName = 0x337B8;
	}
	if (Fortnite_Version >= 2.2 && Fortnite_Version <= 2.4) // 2.2 & 2.4
	{
		Offsets::ClientWorldPackageName = 0xA17A8;
	}
	if (Fortnite_Version == 2.42 || Fortnite_Version == 2.5)
	{
		Offsets::ClientWorldPackageName = 0x17F8;
	}
	if (Fortnite_Version >= 2.5 && Fortnite_Version <= 3.1) // checked 2.5, 3.0, 3.1
	{
		Offsets::NetworkObjectList = 0x4F0;
		Offsets::ReplicationFrame = 0x328;
	}
	if (Fortnite_Version == 3.1 || Fortnite_Version == 3.2)
	{
		Offsets::NetworkObjectList = 0x4F8;
		Offsets::ClientWorldPackageName = 0x1818;
	}
	if (Engine_Version == 419) // checked 2.4.2 & 2.2 & 1.11
	{
		Offsets::NetworkObjectList = 0x490;
		Offsets::ReplicationFrame = 0x2C8;
	}
	if (Fortnite_Version >= 20 && Fortnite_Version < 22)
	{
		Offsets::ReplicationFrame = 0x3D8;
	}

	Offsets::IsNetRelevantFor = FindIsNetRelevantForOffset();
	Offsets::Script = Offsets::Children + 8 + 4 + 4;
}

void Offsets::Print()
{
	LOG_INFO(LogDev, "Offset_Internal: 0x{:x}", Offset_Internal);
	LOG_INFO(LogDev, "SuperStruct: 0x{:x}", SuperStruct);
	LOG_INFO(LogDev, "Children: 0x{:x}", Children);
	LOG_INFO(LogDev, "PropertiesSize: 0x{:x}", PropertiesSize);
	LOG_INFO(LogDev, "Func: 0x{:x}", Func);
	LOG_INFO(LogDev, "ServerReplicateActors: 0x{:x}", ServerReplicateActors);
	LOG_INFO(LogDev, "ReplicationFrame: 0x{:x}", ReplicationFrame);
	LOG_INFO(LogDev, "Script: 0x{:x}", Script);
	LOG_INFO(LogDev, "PropertyClass: 0x{:x}", PropertyClass);
}

void Addresses::Init()
{
	// UObject::ProcessEventOriginal = decltype(UObject::ProcessEventOriginal)(ProcessEvent); // we do this in Addresses::FindAll()
	// StaticFindObjectOriginal = decltype(StaticFindObjectOriginal)(StaticFindObject); // we do this in Addresses::FindAll()
	UWorld::SpawnActorOriginal = decltype(UWorld::SpawnActorOriginal)(SpawnActor);
	UNetDriver::InitListenOriginal = decltype(UNetDriver::InitListenOriginal)(InitListen);
	AGameSession::KickPlayerOriginal = decltype(AGameSession::KickPlayerOriginal)(KickPlayer);
	UNetDriver::TickFlushOriginal = decltype(UNetDriver::TickFlushOriginal)(TickFlush);
	FMemory::Realloc = decltype(FMemory::Realloc)(Realloc);
	UAbilitySystemComponent::GiveAbilityOriginal = decltype(UAbilitySystemComponent::GiveAbilityOriginal)(GiveAbility);
	UAbilitySystemComponent::InternalTryActivateAbilityOriginal = decltype(UAbilitySystemComponent::InternalTryActivateAbilityOriginal)(InternalTryActivateAbility);
	UAbilitySystemComponent::InternalTryActivateAbilityOriginal2 = decltype(UAbilitySystemComponent::InternalTryActivateAbilityOriginal2)(InternalTryActivateAbility);
	ABuildingActor::OnDamageServerOriginal = decltype(ABuildingActor::OnDamageServerOriginal)(OnDamageServer);
	StaticLoadObjectOriginal = decltype(StaticLoadObjectOriginal)(StaticLoadObject);

	static auto DefaultNetDriver = FindObject(L"/Script/Engine.Default__NetDriver");
	Addresses::SetWorld = Engine_Version < 426 ? Addresses::SetWorld : __int64(DefaultNetDriver->VFTable[Addresses::SetWorld]);
	UNetDriver::SetWorldOriginal = decltype(UNetDriver::SetWorldOriginal)(SetWorld);

	NavSystemCleanUpOriginal = decltype(NavSystemCleanUpOriginal)(Addresses::NavSystemCleanUp);
	LoadPlaysetOriginal = decltype(LoadPlaysetOriginal)(Addresses::LoadPlayset);
	AFortGameModeAthena::SetZoneToIndexOriginal = decltype(AFortGameModeAthena::SetZoneToIndexOriginal)(Addresses::SetZoneToIndex);

	if (Engine_Version >= 421) ChunkedObjects = decltype(ChunkedObjects)(ObjectArray);
	else UnchunkedObjects = decltype(UnchunkedObjects)(ObjectArray);
}

std::vector<uint64> Addresses::GetFunctionsToReturnTrue()
{
	std::vector<uint64> toReturnTrue;

	if (Fortnite_Version == 1.11 || Fortnite_Version >= 2.2 && Fortnite_Version <= 2.4)
	{
		toReturnTrue.push_back(Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 6C 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B 01 49 8B E9 45 0F B6 F8").Get()); // No Reserve
	}

	if (std::floor(Fortnite_Version) == 17)
	{
		toReturnTrue.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8B EC 48 83 EC 60 4D 8B F9 41 8A F0 4C 8B F2 48 8B F9 45 32 E4").Get()); // No Reserve
	}

	if (Fortnite_Version >= 19)
	{
		// toReturnTrue.push_back(Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 01 49 8B F0 33 DB FF 50 20 48 8B F8").Get()); // funny session thingy
	}

	if (Engine_Version >= 426)
	{
		toReturnTrue.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 60 20 55 41 56 41 57 48 8B EC 48 83 EC 60 49 8B D9 45 8A").Get()); // No reserve
	}

	return toReturnTrue;
}

std::vector<uint64> Addresses::GetFunctionsToNull()
{
	std::vector<uint64> toNull;

	if (Engine_Version == 416)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 89 54 24 ? 48 89 4C 24 ? 55 53 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 8B 41 08 C1 E8 05").Get()); // Widget class
	}

	if (Fortnite_Version > 3.2 && Engine_Version == 420)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 57 48 81 EC ? ? ? ? 4C 8B 82 ? ? ? ? 48 8B F9 0F 29 70 E8 0F 29 78 D8").Get()); // Pawn Overlap
		// toNull.push_back(Memcury::Scanner::FindPattern("E8 ? ? ? ? EB 26 40 38 3D ? ? ? ?").RelativeOffset(1).Get()); // collectgarbage
	}

	if (Fortnite_Version == 4.1)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("4C 8B DC 55 49 8D AB ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 89 5B 10 48 8D 05 ? ? ? ? 48 8B 1D ? ? ? ? 49 89 73 18 33 F6 40").Get()); // grassupdate
	}

	if (Engine_Version == 421)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 70 10 57 48 81 EC ? ? ? ? 48 8B BA ? ? ? ? 48 8B DA 0F 29").Get()); // Pawn Overlap
		toNull.push_back(Memcury::Scanner::FindStringRef(L"Widget Class %s - Running Initialize On Archetype, %s.").ScanFor({ 0x40, 0x55 }, false).Get()); // Widget class
	}

	if (Engine_Version == 422)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 89 5C 24 ? 57 48 83 EC 30 48 8B 41 28 48 8B DA 48 8B F9 48 85 C0 74 34 48 8B 4B 08 48 8D").Get()); // widget class
	}

	if (Engine_Version == 425)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("40 57 41 56 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 0F B6 FA 44 8B F1 74 3A 80 3D ? ? ? ? ? 0F 82").Get()); // collect garbage
	}

	if (Fortnite_Version == 12.41)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 70 48 8B 35").Get()); // random travis crash
		toNull.push_back(Memcury::Scanner::FindPattern("40 57 41 56 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 0F B6 FA 44 8B F1 74 3A 80 3D ? ? ? ? ? 0F").Get()); // collect garbage
	}

	if (Fortnite_Version == 12.61)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 89 5C 24 ? 55 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 20 4C 8B A5").Get()); // idfk
		// toNull.push_back(Memcury::Scanner::FindPattern("48 89 4C 24 ? 55 56 57 41 56 48 81 EC ? ? ? ? 4C 8B B1 ? ? ? ? 33 F6 4C 89 B4 24 ? ? ? ? 48 8B").Get()); // fritter crash
	}

	if (Fortnite_Version == 14.60)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("40 55 57 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 80 3D ? ? ? ? ? 0F B6 FA 44 8B F9 74 3B 80 3D ? ? ? ? ? 0F").Get());
	}

	if (std::floor(Fortnite_Version) == 17)
	{
		toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 70 08 48 89 78 10 55 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC ? ? ? ? 45 33 ED").Get()); // collectgarbage
	}

	if (Engine_Version == 500)
	{
		// toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC ? ? ? ? 45 33 F6 0F 29 70 A8 44 38 35").Get()); // zone
		// toNull.push_back(Memcury::Scanner::FindPattern("48 8B C4 48 89 58 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 A8 48 81 EC ? ? ? ? 45").Get()); // GC
		// toNull.push_back(Memcury::Scanner::FindPattern("40 53 48 83 EC 20 8B D9 E8 ? ? ? ? B2 01 8B CB E8").Get()); // GC Caller 1
		toNull.push_back(Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 55 41 56 48 8B EC 48 83 EC 50 83 65 28 00 40 B6 05 40 38 35 ? ? ? ? 4C").Get()); // InitializeUI
	}

	toNull.push_back(Addresses::ChangeGameSessionId);

	return toNull;
}