#include "NetDriver.h"

#include "reboot.h"
#include "Actor.h"
#include "NetConnection.h"
#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"
#include "KismetMathLibrary.h"
#include <random>
#include "Package.h"
#include "AssertionMacros.h"
#include "CoreNet.h"
#include "ChildConnection.h"
#include "bots.h"
#include "NetworkingDistanceConstants.h"
#include "gui.h"

void UNetDriver::TickFlushHook(UNetDriver* NetDriver)
{
	if (bShouldDestroyAllPlayerBuilds) // i hate this
	{
		auto AllBuildingSMActors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingSMActor::StaticClass());

		for (int i = 0; i < AllBuildingSMActors.Num(); i++)
		{
			auto CurrentBuildingSMActor = (ABuildingSMActor*)AllBuildingSMActors.at(i);

			if (CurrentBuildingSMActor->IsDestroyed() || CurrentBuildingSMActor->IsActorBeingDestroyed() || !CurrentBuildingSMActor->IsPlayerPlaced()) continue;

			CurrentBuildingSMActor->SilentDie();
			// CurrentBuildingSMActor->K2_DestroyActor();
		}

		AllBuildingSMActors.Free();
		bShouldDestroyAllPlayerBuilds = false;
	}
	
	/* if (bEnableBotTick)
	{
		Bots::Tick();
	} */

	if (Globals::bStartedListening)
	{
		if (!Globals::bShouldUseReplicationGraph)
		{
			NetDriver->ServerReplicateActors();
		}
		else
		{
			static auto ReplicationDriverOffset = NetDriver->GetOffset("ReplicationDriver"/*, false */);

			if (auto ReplicationDriver = NetDriver->Get(ReplicationDriverOffset))
			{
				reinterpret_cast<void(*)(UObject*)>(ReplicationDriver->VFTable[Offsets::ServerReplicateActors])(ReplicationDriver);
			}
			else
			{
				// LOG_INFO(LogDev, "ReplicationDriver is nul!!?1//33/221/4/124/123"); // 3.3 MOMENT
			}
		}
	}

	return TickFlushOriginal(NetDriver);
}

/*

Replication from Unreal Engine 4.19 (using std::vector instead of TArrays).

Can we make it faster? Perhaps removing references from some gets, but it shouldn't do crazy.

*/

FNetworkObjectList& UNetDriver::GetNetworkObjectList()
{
	return *(*(TSharedPtr<FNetworkObjectList>*)(__int64(this) + Offsets::NetworkObjectList));
}

void UNetDriver::RemoveNetworkActor(AActor* Actor) // T(REP)
{
	GetNetworkObjectList().Remove(Actor);

	// RenamedStartupActors.Remove(Actor->GetFName());
}


FActorPriority::FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const std::vector<FNetViewer>& Viewers, bool bLowBandwidth)
	: ActorInfo(InActorInfo), Channel(InChannel), DestructionInfo(NULL)
{
	float Time = Channel ? (InConnection->GetDriver()->GetTime() - Channel->GetLastUpdateTime()) : InConnection->GetDriver()->GetSpawnPrioritySeconds();
	// take the highest priority of the viewers on this connection
	Priority = 0;
	for (int32 i = 0; i < Viewers.size(); i++)
	{
		// Priority = FMath::Max<int32>(Priority, FMath::RoundToInt(65536.0f * 
		//		ActorInfo->Actor->GetNetPriority(Viewers[i].ViewLocation, Viewers[i].ViewDir, Viewers[i].InViewer, V[i].ViewTarget, InChannel, Time, bLowBandwidth)));
	}
}

FActorPriority::FActorPriority(class UNetConnection* InConnection, struct FActorDestructionInfo* Info, const std::vector<FNetViewer>& Viewers)
	: ActorInfo(NULL), Channel(NULL), DestructionInfo(Info)
{
	Priority = 0;

	for (int32 i = 0; i < Viewers.size(); i++)
	{
		float Time = InConnection->GetDriver()->GetSpawnPrioritySeconds();

		FVector Dir = DestructionInfo->DestroyedPosition - Viewers[i].ViewLocation;
		float DistSq = Dir.SizeSquared();

		// adjust priority based on distance and whether actor is in front of viewer
		if ((Viewers[i].ViewDir | Dir) < 0.f)
		{
			if (DistSq > NEARSIGHTTHRESHOLDSQUARED)
				Time *= 0.2f;
			else if (DistSq > CLOSEPROXIMITYSQUARED)
				Time *= 0.4f;
		}
		else if (DistSq > MEDSIGHTTHRESHOLDSQUARED)
			Time *= 0.4f;

		Priority = FMath::Max<int32>(Priority, 65536.0f * Time);
	}
}

bool UNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
	if (Fortnite_Version >= 2.42) // idk
	{
		return true;
	}

	bool bFirstWorldCheck = Engine_Version == 416
		? (InConnection->GetClientWorldPackageName() == GetWorld()->GetOutermost()->GetFName())
		: (InConnection->GetClientWorldPackageName() == GetWorldPackage()->NamePrivate);

	const bool bCorrectWorld = (bFirstWorldCheck && InConnection->ClientHasInitializedLevelFor(InActor));
	const bool bIsConnectionPC = (InActor == InConnection->GetPlayerController());
	return bCorrectWorld || bIsConnectionPC;
}

FNetViewer::FNetViewer(UNetConnection* InConnection, float DeltaSeconds) :
	Connection(InConnection),
	InViewer(InConnection->GetPlayerController() ? InConnection->GetPlayerController() : InConnection->GetOwningActor()),
	ViewTarget(InConnection->GetViewTarget()),
	ViewLocation(ForceInit),
	ViewDir(ForceInit)
{
	if (!InConnection->GetOwningActor()) return;
	if (InConnection->GetPlayerController() && (InConnection->GetPlayerController() != InConnection->GetOwningActor())) return;

	APlayerController* ViewingController = InConnection->GetPlayerController();

	ViewLocation = ViewTarget->GetActorLocation();
	if (ViewingController)
	{
		FRotator ViewRotation = ViewingController->GetControlRotation();
		ViewingController->GetActorEyesViewPoint(&ViewLocation, &ViewRotation);
		ViewDir = ViewRotation.Vector();
	}
}

static FORCEINLINE bool IsActorRelevantToConnection(const AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers)
{
	for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.size(); ++viewerIdx)
	{
		if (reinterpret_cast<bool(*)(const AActor*, AActor*, AActor*, const FVector&)>(Actor->VFTable[Offsets::IsNetRelevantFor])(
			Actor, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		{
			return true;
		}
	}

	return false;
}

static FORCEINLINE UNetConnection* IsActorOwnedByAndRelevantToConnection(const AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
	const AActor* ActorOwner = Actor->GetNetOwner();

	bOutHasNullViewTarget = false;

	for (int i = 0; i < ConnectionViewers.size(); i++)
	{
		UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

		auto ViewTarget = ViewerConnection->GetViewTarget();

		if (ViewTarget == nullptr)
		{
			bOutHasNullViewTarget = true;
		}

		if (ActorOwner == ViewerConnection->GetPlayerController() ||
			(ViewerConnection->GetPlayerController() && ActorOwner == ViewerConnection->GetPlayerController()->GetPawn()) ||
			(ViewTarget && ViewTarget->IsRelevancyOwnerFor(Actor, ActorOwner, ViewerConnection->GetOwningActor())))
		{
			return ViewerConnection;
		}
	}

	return nullptr;
}

static FORCEINLINE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection)
{
	return ActorInfo->DormantConnections.Contains(Connection);
}

static FORCEINLINE bool ShouldActorGoDormant(AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth)
{
	if (Actor->NetDormancy() <= ENetDormancy::DORM_Awake || !Channel || Channel->IsPendingDormancy() || Channel->IsDormant())
	{
		return false;
	}

	if (Actor->NetDormancy() == ENetDormancy::DORM_DormantPartial)
	{
		for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.size(); viewerIdx++)
		{
			if (!Actor->GetNetDormancy(ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
			{
				return false;
			}
		}
	}

	return true;
}

int32 UNetDriver::ServerReplicateActors_PrepConnections()
{
	int32 NumClientsToTick = GetClientConnections().Num();

	/*
	static bool bForceClientTickingThrottle = false;
	if (bForceClientTickingThrottle 
		// || GetNetMode() == NM_ListenServer
		)
	{
		static float DeltaTimeOverflow = 0.f;

		static bool LanPlay = false; // FParse::Param(FCommandLine::Get(), TEXT("lanplay"));
		float ClientUpdatesThisFrame = GetEngine()->NetClientTicksPerSecond * (DeltaSeconds + DeltaTimeOverflow) * (LanPlay ? 2.f : 1.f);
		NumClientsToTick = FMath::Min<int32>(NumClientsToTick, FMath::TruncToInt(ClientUpdatesThisFrame));
		if (NumClientsToTick == 0)
		{
			DeltaTimeOverflow += DeltaSeconds;
			return 0;
		}
		DeltaTimeOverflow = 0.f;
	}

	if (NetCmds::MaxConnectionsToTickPerServerFrame->GetInt() > 0)
	{
		NumClientsToTick = FMath::Min(ClientConnections.Num(), NetCmds::MaxConnectionsToTickPerServerFrame->GetInt());
	}
	*/

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < GetClientConnections().Num(); ConnIdx++)
	{
		UNetConnection* Connection = GetClientConnections().at(ConnIdx);

		if (!Connection) continue;

		// check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		// checkSlow(Connection->GetUChildConnection() == NULL);

		AActor* OwningActor = Connection->GetOwningActor();
		if (OwningActor != NULL 
			// && Connection->State == USOCK_Open 
			&& (Connection->GetDriver()->GetTime() - Connection->GetLastReceiveTime() < 1.5f)
			)
		{
			// check(World == OwningActor->GetWorld());

			bFoundReadyConnection = true;

			AActor* DesiredViewTarget = OwningActor;
			if (Connection->GetPlayerController())
			{
				if (AActor* ViewTarget = Connection->GetPlayerController()->GetViewTarget())
				{
					if (GetWorld()
						// ViewTarget->GetWorld() // T(REP)
						)
					{
						DesiredViewTarget = ViewTarget;
					}
				}
			}
			Connection->GetViewTarget() = DesiredViewTarget;

			for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ++ChildIdx)
			{
				UNetConnection* Child = Connection->GetChildren().at(ChildIdx);
				APlayerController* ChildPlayerController = Child->GetPlayerController();
				if (ChildPlayerController != NULL)
				{
					Child->GetViewTarget() = ChildPlayerController->GetViewTarget();
				}
				else
				{
					Child->GetViewTarget() = NULL;
				}
			}
		}
		else
		{
			Connection->GetViewTarget() = NULL;
			for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ++ChildIdx)
			{
				Connection->GetChildren().at(ChildIdx)->GetViewTarget() = NULL;
			}
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

void UNetDriver::ServerReplicateActors_BuildConsiderList(std::vector<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime)
{
	int32 NumInitiallyDormant = 0;

	const bool bUseAdapativeNetFrequency = true; // IsAdaptiveNetUpdateFrequencyEnabled(); // T(REP)

	std::vector<AActor*> ActorsToRemove;

	for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList().GetActiveObjects())
	{
		FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();

		if (!ActorInfo->bPendingNetUpdate && World()->GetTimeSeconds() <= ActorInfo->NextUpdateTime)
		{
			continue;
		}

		AActor* Actor = ActorInfo->Actor;

		if (Actor->IsPendingKillPending())
		{
			ActorsToRemove.push_back(Actor);
			continue;
		}

		if (Actor->GetRemoteRole() == ROLE_None)
		{
			ActorsToRemove.push_back(Actor);
			continue;
		}

		if (Actor->GetNetDriverName() != this->GetNetDriverName()) // Milxnor: This really shouldn't happen on Fortnite.
		{
			continue;
		}

		if (!Actor->IsActorInitialized())
		{
			continue;
		}

		ULevel* Level = Actor->GetLevel();
		if (Level->HasVisibilityChangeRequestPending() || Level->IsAssociatingLevel())
		{
			continue;
		}

		if (Actor->NetDormancy() == ENetDormancy::DORM_Initial && Actor->IsNetStartupActor())
		{
			NumInitiallyDormant++;
			ActorsToRemove.push_back(Actor);
			continue;
		}

		// checkSlow(Actor->NeedsLoadForClient()); // We have no business sending this unless the client can load
		// checkSlow(World() == Actor->GetWorld());

		if (ActorInfo->LastNetReplicateTime == 0)
		{
			ActorInfo->LastNetReplicateTime = World()->GetTimeSeconds();
			ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->GetNetUpdateFrequency();
		}

		const float ScaleDownStartTime = 2.0f;
		const float ScaleDownTimeRange = 5.0f;

		const float LastReplicateDelta = World()->GetTimeSeconds() - ActorInfo->LastNetReplicateTime;

		if (LastReplicateDelta > ScaleDownStartTime)
		{
			if (Actor->GetMinNetUpdateFrequency() == 0.0f)
			{
				Actor->GetMinNetUpdateFrequency() = 2.0f;
			}

			const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();									 
			const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);

			const float Alpha = FMath::Clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
			ActorInfo->OptimalNetUpdateDelta = FMath::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
		}

		if (!ActorInfo->bPendingNetUpdate)
		{
			const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->GetNetUpdateFrequency();
			ActorInfo->NextUpdateTime = World()->GetTimeSeconds() + FMath::SRand() * ServerTickTime + NextUpdateDelta;
			ActorInfo->LastNetUpdateTime = GetTime();
		}

		ActorInfo->bPendingNetUpdate = false;

		// ensure(OutConsiderList.Num() < OutConsiderList.Max());
		OutConsiderList.push_back(ActorInfo);

		// Call PreReplication on all actors that will be considered
		AActor::originalCallPreReplication(Actor, this);
	}

	for (AActor* Actor : ActorsToRemove)
	{
		GetNetworkObjectList().Remove(Actor);
	}
}

int32 UNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, const std::vector<FNetworkObjectInfo*>& ConsiderList, const bool bCPUSaturated, std::vector<FActorPriority>& OutPriorityList, std::vector<FActorPriority*>& OutPriorityActors)
{
	GetNetTag()++;
	Connection->GetTickCount()++;

	for (int32 j = 0; j < Connection->GetSentTemporaries().Num(); ++j)
	{
		Connection->GetSentTemporaries().at(j)->GetNetTag() = GetNetTag();
	}

	// check(World() == Connection->GetOwningActor()->GetWorld());

	int32 FinalSortedCount = 0;
	int32 DeletedCount = 0;

	TWeakObjectPtr<UNetConnection> WeakConnection; // T(REP)
	WeakConnection.ObjectIndex = Connection->InternalIndex;
	WeakConnection.ObjectSerialNumber = GetItemByIndex(Connection->InternalIndex)->SerialNumber;

	const int32 MaxSortedActors = ConsiderList.size() + GetDestroyedStartupOrDormantActors().Num();
	if (MaxSortedActors > 0)
	{
		// OutPriorityList = new (FMemStack::Get(), MaxSortedActors) FActorPriority;
		// OutPriorityActors = new (FMemStack::Get(), MaxSortedActors) FActorPriority*;
		OutPriorityList.reserve(MaxSortedActors);
		OutPriorityActors.reserve(MaxSortedActors);

		// check(World() == Connection->GetViewTarget()->GetWorld());

		// AGameNetworkManager* const NetworkManager = World->NetworkManager;
		const bool bLowNetBandwidth = false; // NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false; // Milxnor: Again, this really shouldn't be needed.

		for (FNetworkObjectInfo* ActorInfo : ConsiderList)
		{
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = Connection->GetActorChannels().FindRef(ActorInfo->WeakActor);

			if (!Channel)
			{
				if (!IsLevelInitializedForActor(Actor, Connection))
				{
					continue;
				}

				if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					continue;
				}
			}

			UNetConnection* PriorityConnection = Connection;

			if (Actor->IsOnlyRelevantToOwner())
			{
				bool bHasNullViewTarget = false;

				PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

				if (PriorityConnection == nullptr)
				{
					if (!bHasNullViewTarget && Channel != NULL && GetTime() - Channel->GetRelevantTime() >= GetRelevantTimeout())
					{
						Channel->Close();
					}

					continue;
				}
			}
			else if (// CVarSetNetDormancyEnabled.GetValueOnGameThread() != 0 // Milxnor: This is always enabled no?
				true)
			{
				if (IsActorDormant(ActorInfo, WeakConnection))
				{
					continue;
				}

				if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, GetTime(), bLowNetBandwidth))
				{
					Channel->StartBecomingDormant();
				}
			}

			if (Actor->GetNetTag() != GetNetTag())
			{
				Actor->GetNetTag() = GetNetTag();

				OutPriorityList[FinalSortedCount] = FActorPriority(PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth);
				OutPriorityActors[FinalSortedCount] = OutPriorityList.data() + FinalSortedCount;

				FinalSortedCount++;
			}
		}

		// Add in deleted actors
		for (auto It = Connection->GetDestroyedStartupOrDormantActors().CreateIterator(); It; ++It)
		{
			FActorDestructionInfo& DInfo = GetDestroyedStartupOrDormantActors().FindChecked(*It);
			OutPriorityList[FinalSortedCount] = FActorPriority(Connection, &DInfo, ConnectionViewers);
			OutPriorityActors[FinalSortedCount] = OutPriorityList.data() + FinalSortedCount;
			FinalSortedCount++;
			DeletedCount++;
		}

		// Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
	}

	return FinalSortedCount;
}

using FArchive = void;

__declspec(noinline) void SetChannelActorForDestroy(UActorChannel* Channel, FActorDestructionInfo* DestructInfo)
{
	auto Connection = Channel->GetConnection();

	// 	check(Connection->Channels[ChIndex]==Channel);

	if (
		true
		// T(REP)
		)
	{

		// You can get size by searching "Making partial bunch from content bunch. bitsThisBunch: %d bitsLeft: %d", there is a new call.
		struct FOutBunch
		{
			char pad[0x110];
		};

		LOG_INFO(LogDev, "SetChannelActorForDestroy PathName: {}", DestructInfo->PathName.ToString());

		static FOutBunch* (*ConstructorFOutBunch)(__int64, UChannel*, bool) = decltype(ConstructorFOutBunch)(__int64(GetModuleHandleW(0)) + 0x194E800);
		FOutBunch* CloseBunch = (FOutBunch*)// FMemory::Realloc(0, sizeof(FOutBunch), 0);
			VirtualAlloc(0, sizeof(FOutBunch), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		ConstructorFOutBunch(__int64(CloseBunch), Channel, 1);

		// check(!CloseBunch.IsError());
		// check(CloseBunch.bClose);

		LOG_INFO(LogDev, "Called Constructor!");

		// https://imgur.com/a/EtKFkrD

		*(bool*)(__int64(&CloseBunch) + 0xE8) = 1; // bReliable
		*(bool*)(__int64(&CloseBunch) + 0xE6) = 0; // bDormant

		// NET_CHECKSUM(CloseBunch); // This is to mirror the Checksum in UPackageMapClient::SerializeNewActor

		using UPackageMap = UObject;

		reinterpret_cast<bool(*)(UPackageMap*, 
			// FArchive& Ar, 
			FOutBunch* Ar,
			UObject* InOuter, FNetworkGUID NetGUID, FString ObjName)>(Connection->GetPackageMap()->VFTable[0x238 / 8])
			(Connection->GetPackageMap(), CloseBunch, DestructInfo->ObjOuter.Get(), DestructInfo->NetGUID, DestructInfo->PathName); // WriteObject

		// 0x196E9C0
		reinterpret_cast<FPacketIdRange(*)(UActorChannel*, FOutBunch*, bool)>(Channel->VFTable[0x288 / 8])(Channel, CloseBunch, false); // SendBunch
		
		// TODO FARCHIVE::~FARHCIVE
	}
}

int32 UNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, std::vector<FActorPriority*>& PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
{
	int32 ActorUpdatesThisConnection = 0;
	int32 ActorUpdatesThisConnectionSent = 0;
	int32 FinalRelevantCount = 0;

	if (!Connection->IsNetReady(0))
	{
		return 0;
	}

	for (int32 j = 0; j < FinalSortedCount; j++)
	{
		FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;

		if (ActorInfo == NULL && PriorityActors[j]->DestructionInfo)
		{
			if (PriorityActors[j]->DestructionInfo->StreamingLevelName != NAME_None
				&& !Connection->GetClientVisibleLevelNames().Contains(PriorityActors[j]->DestructionInfo->StreamingLevelName)
				)
			{
				continue;
			}

			UActorChannel* Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1, EChannelCreateFlags::OpenedLocally);
			if (Channel)
			{
				FinalRelevantCount++;

				SetChannelActorForDestroy(Channel, PriorityActors[j]->DestructionInfo);
				Connection->GetDestroyedStartupOrDormantActors().Remove(PriorityActors[j]->DestructionInfo->NetGUID);
			}

			continue;
		}

		UActorChannel* Channel = PriorityActors[j]->Channel;
		if (!Channel || Channel->GetActor())
		{
			AActor* Actor = ActorInfo->Actor;
			bool bIsRelevant = false;

			const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

			if (bLevelInitializedForActor)
			{
				if (!Actor->IsTearOff() && (!Channel || GetTime() - Channel->GetRelevantTime() > 1.f))
				{
					if (IsActorRelevantToConnection(Actor, ConnectionViewers))
					{
						bIsRelevant = true;
					}
				}
			}

			// if the actor is now relevant or was recently relevant
			const bool bIsRecentlyRelevant = bIsRelevant || (Channel && GetTime() - Channel->GetRelevantTime() < GetRelevantTimeout()) || ActorInfo->bForceRelevantNextUpdate;

			ActorInfo->bForceRelevantNextUpdate = false;

			if (bIsRecentlyRelevant)
			{
				FinalRelevantCount++;

				if (Channel == NULL 
					// T(REP)
					// && GuidCache->SupportsObject(Actor->GetClass()) 
					// && GuidCache->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype())
					)
				{
					if (bLevelInitializedForActor)
					{
						Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1, EChannelCreateFlags::OpenedLocally);
						if (Channel)
						{
							LOG_INFO(LogDev, "Replicating: {}", Actor->GetFullName());
							Channel->SetChannelActor(Actor, ESetChannelActorFlags::None1);
						}
					}
					else if (Actor->GetNetUpdateFrequency() < 1.0f)
					{
						auto ActorWorld = GetWorld(); // Actor->GetWorld() // T(REP)
						ActorInfo->NextUpdateTime = ActorWorld->GetTimeSeconds() + 0.2f * FMath::FRand();
					}
				}

				if (Channel)
				{
					if (bIsRelevant)
					{
						Channel->GetRelevantTime() = GetTime() + 0.5f * FMath::SRand();
					}
					if (Channel->IsNetReady(0))
					{
						if (Channel->ReplicateActor())
						{
							ActorUpdatesThisConnectionSent++;

							const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
							const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);
							const float DeltaBetweenReplications = (World()->GetTimeSeconds() - ActorInfo->LastNetReplicateTime);

							ActorInfo->OptimalNetUpdateDelta = FMath::Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
							ActorInfo->LastNetReplicateTime = World()->GetTimeSeconds();
						}

						ActorUpdatesThisConnection++;
						OutUpdated++;
					}
					else
					{
						Actor->ForceNetUpdate();
					}

					if (!Connection->IsNetReady(0))
					{
						return j;
					}
				}
			}

			if ((!bIsRecentlyRelevant || Actor->IsTearOff()) && Channel != NULL)
			{
				if (!bLevelInitializedForActor || !Actor->IsNetStartupActor())
				{
					Channel->Close();
				}
			}
		}
	}

	return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors()
{
	float DeltaSeconds = 0; // T(REP) Milxnor: This is a parameter but really I cant be asked to implement it.

	/*
	
	if ( ClientConnections.Num() == 0 )
	{
		return 0;
	}

	check( World );

	*/

	int32 Updated = 0;

	// Bump the ReplicationFrame value to invalidate any properties marked as "unchanged" for this frame.
	++(*(int*)(__int64(this) + Offsets::ReplicationFrame));

	const int32 NumClientsToTick = ServerReplicateActors_PrepConnections();

	if (NumClientsToTick == 0)
	{
		// No connections are ready this frame
		return 0;
	}

	AWorldSettings* WorldSettings = World()->GetWorldSettings();

	bool bCPUSaturated = false;
	float ServerTickTime = ServerTickRate; // GetEngine()->GetMaxTickRate(DeltaSeconds); // Milxnor: We hook GetMaxTickRate and just return ServerTickRate.
	if (ServerTickTime == 0.f)
	{
		ServerTickTime = DeltaSeconds;
	}
	else
	{
		ServerTickTime = 1.f / ServerTickTime;
		bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
	}

	std::vector<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.reserve(GetNetworkObjectList().GetActiveObjects().Num());

	// Build the consider list (actors that are ready to replicate)
	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

	for (int32 i = 0; i < GetClientConnections().Num(); ++i)
	{
		UNetConnection* Connection = GetClientConnections().at(i);
		if (!Connection) continue;

		/*
		if (CVarNetDormancyValidate.GetValueOnAnyThread() == 2)
		{
			for (auto It = Connection->DormantReplicatorMap.CreateIterator(); It; ++It)
			{
				FObjectReplicator& Replicator = It.Value().Get();

				if (Replicator.OwningChannel != nullptr)
				{
					Replicator.ValidateAgainstState(Replicator.OwningChannel->GetActor());
				}
			}
		}
		*/

		if (i >= NumClientsToTick)
		{
			for (int32 ConsiderIdx = 0; ConsiderIdx < ConsiderList.size(); ++ConsiderIdx)
			{
				AActor* Actor = ConsiderList[ConsiderIdx]->Actor;

				if (Actor != NULL && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
				{
					UActorChannel* Channel = Connection->GetActorChannels().FindRef(ConsiderList[ConsiderIdx]->WeakActor);

					if (Channel != NULL && Channel->GetLastUpdateTime() < ConsiderList[ConsiderIdx]->LastNetUpdateTime)
					{
						ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
					}
				}
			}

			// Connection->GetTimeSensitive() = false; // T(REP)
		}
		else if (Connection->GetViewTarget())
		{
			// std::vector<FNetViewer>& ConnectionViewers = WorldSettings->GetReplicationViewers(); // Milxnor: Techinally this is proper but they literally just reset it before rep ends.
			std::vector<FNetViewer> ConnectionViewers;
			ConnectionViewers.clear();

			// new(ConnectionViewers)FNetViewer(Connection, DeltaSeconds);
			ConnectionViewers.push_back(FNetViewer(Connection, DeltaSeconds));

			for (int32 ViewerIndex = 0; ViewerIndex < Connection->GetChildren().Num(); ++ViewerIndex)
			{
				if (Connection->GetChildren().at(ViewerIndex)->GetViewTarget() != NULL)
				{
					// new(ConnectionViewers)FNetViewer(Connection->GetChildren().at(ViewerIndex), DeltaSeconds);
					ConnectionViewers.push_back(FNetViewer(Connection->GetChildren().at(ViewerIndex), DeltaSeconds));
				}
			}

			if (Connection->GetPlayerController())
			{
				APlayerController::originalSendClientAdjustment(Connection->GetPlayerController());
			}

			for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ChildIdx++)
			{
				if (Connection->GetChildren().at(ChildIdx)->GetPlayerController() != NULL)
				{
					APlayerController::originalSendClientAdjustment(Connection->GetChildren().at(ChildIdx)->GetPlayerController());
				}
			}

			std::vector<FActorPriority> PriorityList;
			std::vector<FActorPriority*> PriorityActors;

			const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);
			const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

			for (int32 k = LastProcessedActor; k < FinalSortedCount; ++k)
			{
				if (!PriorityActors[k]->ActorInfo)
				{
					continue;
				}

				AActor* Actor = PriorityActors[k]->ActorInfo->Actor;

				UActorChannel* Channel = PriorityActors[k]->Channel;

				if (Channel != NULL && GetTime() - Channel->GetRelevantTime() <= 1.f)
				{
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
				}
				else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
					if (Channel != NULL)
					{
						Channel->GetRelevantTime() = GetTime() + 0.5f * FMath::SRand();
					}
				}
			}

			ConnectionViewers.clear();
		}
	}

	return Updated;
}