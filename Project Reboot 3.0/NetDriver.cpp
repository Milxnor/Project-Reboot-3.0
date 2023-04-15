#include "NetDriver.h"

#include "reboot.h"
#include "Actor.h"
#include "NetConnection.h"
#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"
#include "KismetMathLibrary.h"
#include <random>
#include "GenericPlatformMath.h"
#include "ActorChannel.h"
#include "KismetSystemLibrary.h"
#include "UnrealMathUtility.h"
#include "FortQuickBars.h"
#include "Sort.h"
#include "Sorting.h"

void UNetDriver::TickFlushHook(UNetDriver* NetDriver)
{
	static auto ReplicationDriverOffset = NetDriver->GetOffset("ReplicationDriver", false);

	if (ReplicationDriverOffset == -1)
	{
		NetDriver->ServerReplicateActors();
	}
	else
	{
		if (auto ReplicationDriver = NetDriver->Get(ReplicationDriverOffset))
			reinterpret_cast<void(*)(UObject*)>(ReplicationDriver->VFTable[Offsets::ServerReplicateActors])(ReplicationDriver);
	}

	return TickFlushOriginal(NetDriver);
}

void Test3(AActor* Actor, const std::string& Str)
{
	if (Actor->IsA(AFortPawn::StaticClass()))
	{
		LOG_INFO(LogDev, "[{}] Pawn", Str)
	}
	if (Actor->IsA(AFortQuickBars::StaticClass()))
	{
		LOG_INFO(LogDev, "[{}] QuickBars", Str)
	}
}

#define NAME_None 0

static FNetViewer ConstructNetViewer(UNetConnection* NetConnection)
{
	FNetViewer newViewer{};
	newViewer.Connection = NetConnection;
	newViewer.InViewer = NetConnection->GetPlayerController() ? NetConnection->GetPlayerController() : NetConnection->GetOwningActor();
	newViewer.ViewTarget = NetConnection->GetViewTarget();

	// if (!NetConnection->GetOwningActor() || !(!NetConnection->GetPlayerController() || (NetConnection->GetPlayerController() == NetConnection->GetOwningActor())))
		// return newViewer;

	APlayerController* ViewingController = NetConnection->GetPlayerController();

	newViewer.ViewLocation = newViewer.ViewTarget->GetActorLocation();

	if (ViewingController)
	{
		static auto ControlRotationOffset = ViewingController->GetOffset("ControlRotation");
		FRotator ViewRotation = ViewingController->Get<FRotator>(ControlRotationOffset); // hmmmm // ViewingController->GetControlRotation();
		// AFortPlayerControllerAthena::GetPlayerViewPointHook(Cast<AFortPlayerControllerAthena>(ViewingController, false), newViewer.ViewLocation, ViewRotation);
		// ViewingController->GetActorEyesViewPoint(&newViewer.ViewLocation, &ViewRotation); // HMMM

		static auto GetActorEyesViewPointOffset = 0x5B0;
		void (*GetActorEyesViewPointOriginal)(AController*, FVector * a2, FRotator * a3) = decltype(GetActorEyesViewPointOriginal)(ViewingController->VFTable[GetActorEyesViewPointOffset / 8]);
		GetActorEyesViewPointOriginal(ViewingController, &newViewer.ViewLocation, &ViewRotation);
		// AFortPlayerControllerAthena::GetPlayerViewPointHook((AFortPlayerControllerAthena*)ViewingController, newViewer.ViewLocation, ViewRotation);
		newViewer.ViewDir = ViewRotation.Vector();
	}

	return newViewer;
}

FNetworkObjectList& UNetDriver::GetNetworkObjectList()
{
	return *(*(TSharedPtr<FNetworkObjectList>*)(__int64(this) + Offsets::NetworkObjectList));
}

struct FPacketIdRange
{
	FPacketIdRange(int32 _First, int32 _Last) : First(_First), Last(_Last) { }
	FPacketIdRange(int32 PacketId) : First(PacketId), Last(PacketId) { }
	// FPacketIdRange() : First(INDEX_NONE), Last(INDEX_NONE) { }
	int32	First;
	int32	Last;

	bool	InRange(int32 PacketId)
	{
		return (First <= PacketId && PacketId <= Last);
	}
};

float GetTimeSecondsForWorld(UWorld* World)
{
	static auto TimeSecondsOffset = 0x900;
	return *(float*)(__int64(World) + TimeSecondsOffset);
}

template <typename T = __int64>
T* AllocForReplication(size_t Size)
{
	return (T*)FMemory::Realloc(nullptr, Size, 0);
	return Alloc<T>(Size);
}

void SetChannelActorForDestroy(UActorChannel* ActorChannel, FActorDestructionInfo* DestructInfo)
{
	static auto ConnectionOffset = ActorChannel->GetOffset("Connection");
	UNetConnection* Connection = ActorChannel->Get<UNetConnection*>(ConnectionOffset);

	auto State = *(uint8_t*)(__int64(Connection) + 0x12C);

	if (!(State - 2 <= 1)) // this will make sure that it is USOCK_Open or USOCK_Pending
		return;

	using FOutBunch = __int64;

	static auto PackageMapOffset = Connection->GetOffset("PackageMap");
	auto PackageMap = Connection->Get(PackageMapOffset);

	FOutBunch* CloseBunch = AllocForReplication(0x200);

	if (!CloseBunch)
		return;

	static FOutBunch(*FOutBunchConstructor)(FOutBunch * a1, UActorChannel * a2, bool bInClose) = decltype(FOutBunchConstructor)(__int64(GetModuleHandleW(0)) + 0x194E800);
	FPacketIdRange Range(0);
	FPacketIdRange* (*SendBunchOriginal)(UActorChannel * Channel, FPacketIdRange * OutRange, FOutBunch * Bunch, bool Merge) = decltype(SendBunchOriginal)(ActorChannel->VFTable[0x288 / 8]);
	bool (*UPackageMap_WriteObjectOriginal)(UObject * PackageMap, FOutBunch * Ar, UObject * InOuter, FNetworkGUID NetGUID, FString ObjectName) = decltype(UPackageMap_WriteObjectOriginal)(PackageMap->VFTable[0x238 / 8]);
	static void (*FArchiveDeconstructor)(FOutBunch* Ar) = decltype(FArchiveDeconstructor)(__int64(GetModuleHandleW(0)) + 0xC36500);

	FOutBunchConstructor(CloseBunch, ActorChannel, true);

	// we could set bDormant but it's set by default to 0.
	SetBitfield((PlaceholderBitfield*)(__int64(CloseBunch) + 0x30), 4, true); // bReliable

	LOG_INFO(LogDev, "UPackageMap_WriteObjectOriginal: 0x{:x}", __int64(UPackageMap_WriteObjectOriginal) - __int64(GetModuleHandleW(0)));
	LOG_INFO(LogDev, "DestructInfo->PathName: {} Num: {} Max: {} Data: {}", DestructInfo->PathName.ToString(), DestructInfo->PathName.Data.Num(), DestructInfo->PathName.Data.ArrayMax, __int64(DestructInfo->PathName.Data.Data));
	// LOG_INFO(LogDev, "DestructInfo->ObjOuter: {}", DestructInfo->ObjOuter.Get()->IsValidLowLevel() ? DestructInfo->ObjOuter.Get()->GetFullName() : "BadRead");

	UPackageMap_WriteObjectOriginal(PackageMap, CloseBunch, DestructInfo->ObjOuter.Get(), DestructInfo->NetGUID, DestructInfo->PathName);
	SendBunchOriginal(ActorChannel, &Range, CloseBunch, false);

	FArchiveDeconstructor(CloseBunch);
}

int CVarSetNetDormancyEnabled = 1; // idk what we supposed to set this to

#define CLOSEPROXIMITY					500.f
#define NEARSIGHTTHRESHOLD				2000.f
#define MEDSIGHTTHRESHOLD				3162.f
#define FARSIGHTTHRESHOLD				8000.f
#define CLOSEPROXIMITYSQUARED			(CLOSEPROXIMITY*CLOSEPROXIMITY)
#define NEARSIGHTTHRESHOLDSQUARED		(NEARSIGHTTHRESHOLD*NEARSIGHTTHRESHOLD)
#define MEDSIGHTTHRESHOLDSQUARED		(MEDSIGHTTHRESHOLD*MEDSIGHTTHRESHOLD)
#define FARSIGHTTHRESHOLDSQUARED		(FARSIGHTTHRESHOLD*FARSIGHTTHRESHOLD)

FActorPriority::FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const std::vector<FNetViewer>& Viewers, bool bLowBandwidth)
	: ActorInfo(InActorInfo), Channel(InChannel), DestructionInfo(NULL)
{
	float Time = Channel ? (InConnection->GetDriver()->GetTime() - Channel->GetLastUpdateTime()) : InConnection->GetDriver()->GetSpawnPrioritySeconds();
	// take the highest priority of the viewers on this connection
	Priority = 0;

	for (int32 i = 0; i < Viewers.size(); i++)
	{
		static auto GetNetPriorityOffset = 0x380;
		float (*GetNetPriorityOriginal)(AActor* Actor, const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
			= decltype(GetNetPriorityOriginal)(ActorInfo->Actor->VFTable[GetNetPriorityOffset / 8]);

		Priority = FMath::Max<int32>(Priority, FMath::RoundToInt(65536.0f * GetNetPriorityOriginal(ActorInfo->Actor, Viewers[i].ViewLocation, Viewers[i].ViewDir, Viewers[i].InViewer, Viewers[i].ViewTarget, InChannel, Time, bLowBandwidth)));
	}
}

FActorPriority::FActorPriority(UNetConnection* InConnection, FActorDestructionInfo* Info, const std::vector<FNetViewer>& Viewers)
	: ActorInfo(NULL), Channel(NULL), DestructionInfo(Info)
{
	Priority = 0;

	for (int32 i = 0; i < Viewers.size(); i++)
	{
		float Time = InConnection->GetDriver()->GetSpawnPrioritySeconds();

		FVector Dir = DestructionInfo->DestroyedPosition - Viewers[i].ViewLocation;
		float DistSq = Dir.SizeSquared();

		// adjust priority based on distance and whether actor is in front of viewer
		if ((Viewers.at(i).ViewDir | Dir) < 0.f)
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

TSet<FNetworkGUID>& GetConnectionDestroyedStartupOrDormantActors(UNetConnection* Connection)
{
	return *(TSet<FNetworkGUID>*)(__int64(Connection) + 0x33678);
}

TMap<FNetworkGUID, FActorDestructionInfo>& GetDriverDestroyedStartupOrDormantActors(UNetDriver* NetDriver)
{
	return *(TMap<FNetworkGUID, FActorDestructionInfo>*)(__int64(NetDriver) + 0x228);
}

void UNetDriver::RemoveNetworkActor(AActor* Actor)
{
	static void (*FNetworkObjectList_Remove)(FNetworkObjectList*, AActor* const a2) = decltype(FNetworkObjectList_Remove)(__int64(GetModuleHandleW(0)) + 0x1AEBB40);
	FNetworkObjectList_Remove(&GetNetworkObjectList(), Actor);
	// GetNetworkObjectList().Remove(Actor);

	// RenamedStartupActors.Remove(Actor->GetFName());
}

int MaxConnectionsToTickPerServerFrame = 25;

int32 ServerReplicateActors_PrepConnections(UNetDriver* NetDriver)
{
	auto& ClientConnections = NetDriver->GetClientConnections();

	int32 NumClientsToTick = ClientConnections.Num();

	if (MaxConnectionsToTickPerServerFrame > 0)
		NumClientsToTick = FMath::Min(ClientConnections.Num(), MaxConnectionsToTickPerServerFrame);

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		UNetConnection* Connection = ClientConnections.at(ConnIdx);
		if (!Connection) continue;
		// check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		// checkSlow(Connection->GetUChildConnection() == NULL);

		AActor* OwningActor = Connection->GetOwningActor();

		if (OwningActor != NULL && /* Connection->State == USOCK_Open && */ (Connection->GetDriver()->GetTime() - Connection->GetLastReceiveTime() < 1.5f))
		{
			// check( World == OwningActor->GetWorld() );

			bFoundReadyConnection = true;

			AActor* DesiredViewTarget = OwningActor;

			if (Connection->GetPlayerController())
			{
				if (AActor* ViewTarget = Connection->GetPlayerController()->GetViewTarget())
				{
					DesiredViewTarget = ViewTarget;
				}
			}

			Connection->GetViewTarget() = DesiredViewTarget;
		}
		else
		{
			Connection->GetViewTarget() = NULL;
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

enum class ENetRole : uint8_t
{
	ROLE_None = 0,
	ROLE_SimulatedProxy = 1,
	ROLE_AutonomousProxy = 2,
	ROLE_Authority = 3,
	ROLE_MAX = 4
};

FORCEINLINE float FRand()
{
	return ReplicationRandStream.FRand();
}

#if 1 // below is "proper"
FORCEINLINE float SRand()
{
	GSRandSeed = (GSRandSeed * 196314165) + 907633515;
	union { float f; int32 i; } Result;
	union { float f; int32 i; } Temp;
	const float SRandTemp = 1.0f;
	Temp.f = SRandTemp;
	Result.i = (Temp.i & 0xff800000) | (GSRandSeed & 0x007fffff);
	auto res = FPlatformMath::Fractional(Result.f);
	// MILXNOR
	// res /= 3;
	return res;
}
#else
FORCEINLINE float SRand()
{
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto value = now_ms.time_since_epoch().count();

	std::default_random_engine generator(value);

	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(generator);
}
#endif

void UNetDriver::ServerReplicateActors_BuildConsiderList(std::vector<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime)
{
	std::vector<AActor*> ActorsToRemove;

	auto World = GetWorld();

	for (const TSharedPtr<FNetworkObjectInfo>& ActorInfo : GetNetworkObjectList().ActiveNetworkObjects)
	{
		if (!ActorInfo->bPendingNetUpdate && GetTimeSecondsForWorld(GetWorld()) <= ActorInfo->NextUpdateTime)
		{
			continue;
		}

		auto Actor = ActorInfo->Actor;

		if (Actor->IsPendingKillPending())
		{
			ActorsToRemove.push_back(Actor);
			continue;
		}

		static auto RemoteRoleOffset = Actor->GetOffset("RemoteRole");

		if (Actor->Get<ENetRole>(RemoteRoleOffset) == ENetRole::ROLE_None)
		{
			ActorsToRemove.push_back(Actor);
			continue;
		}

		// We should add a NetDriverName check but I don't believe it is needed.

		// We should check if the actor is initialized here.

		// We should check the level stuff here.

		if (Actor->GetNetDormancy() == ENetDormancy::DORM_Initial && Actor->IsNetStartupActor())
		{
			ActorsToRemove.push_back(Actor);
			continue;
		}

		static auto NeedsLoadForClientOriginalOffset = 0xD0;
		bool (*NeedsLoadForClientOriginal)(AActor* Actor) = decltype(NeedsLoadForClientOriginal)(Actor->VFTable[NeedsLoadForClientOriginalOffset / 8]);

		if (!NeedsLoadForClientOriginal(Actor)) // Should we remove?
			continue;

		// We should make sure the actor is in the same world here but I don't believe it is needed.

		if (ActorInfo->LastNetReplicateTime == 0)
		{
			ActorInfo->LastNetReplicateTime = GetTimeSecondsForWorld(World);
			ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->GetNetUpdateFrequency();
		}

		const float ScaleDownStartTime = 2.0f;
		const float ScaleDownTimeRange = 5.0f;

		const float LastReplicateDelta = GetTimeSecondsForWorld(World) - ActorInfo->LastNetReplicateTime;

		if (LastReplicateDelta > ScaleDownStartTime)
		{
			if (Actor->GetMinNetUpdateFrequency() == 0.0f)
			{
				Actor->GetMinNetUpdateFrequency() = 2.0f;
			}

			const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();									  // Don't go faster than NetUpdateFrequency
			const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta); // Don't go slower than MinNetUpdateFrequency (or NetUpdateFrequency if it's slower)

			const float Alpha = FMath::Clamp( (LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f);
			ActorInfo->OptimalNetUpdateDelta = FMath::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
		}

		if (!ActorInfo->bPendingNetUpdate)
		{
			constexpr bool bUseAdapativeNetFrequency = false;
			const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->GetNetUpdateFrequency();

			// then set the next update time
			ActorInfo->NextUpdateTime = GetTimeSecondsForWorld(World) + SRand() * ServerTickTime + NextUpdateDelta;
			ActorInfo->LastNetUpdateTime = GetTime();
		}

		ActorInfo->bPendingNetUpdate = false;

		// ensure( OutConsiderList.Num() < OutConsiderList.Max() );
		OutConsiderList.push_back(ActorInfo.Get());

		static void (*CallPreReplication)(AActor*, UNetDriver*) = decltype(CallPreReplication)(Addresses::CallPreReplication);
		CallPreReplication(Actor, this);
	}

	for (auto Actor : ActorsToRemove)
	{
		if (!Actor)
			continue;

		RemoveNetworkActor(Actor);
	}
}

static bool IsActorRelevantToConnection(AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers)
{
	for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.size(); viewerIdx++)
	{
		if (!ConnectionViewers[viewerIdx].ViewTarget)
			continue;

		// static bool (*IsNetRelevantFor)(AActor*, AActor*, AActor*, FVector&) = decltype(IsNetRelevantFor)(__int64(GetModuleHandleW(0)) + 0x1ECC700);

		static auto index = Offsets::IsNetRelevantFor;

		// if (Actor->IsNetRelevantFor(ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		// if (IsNetRelevantFor(Actor, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		if (reinterpret_cast<bool(*)(AActor*, AActor*, AActor*, const FVector&)>(Actor->VFTable[index])(
			Actor, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		{
			return true;
		}
	}

	return false;
}

bool UNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
	const bool bCorrectWorld = (InConnection->GetClientWorldPackageName() == GetWorldPackage()->NamePrivate && InConnection->ClientHasInitializedLevelFor(InActor));
	const bool bIsConnectionPC = (InActor == InConnection->GetPlayerController());
	return bCorrectWorld || bIsConnectionPC;
}

static FORCEINLINE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection)
{
	// If actor is already dormant on this channel, then skip replication entirely
	return ActorInfo->DormantConnections.Contains(Connection);
}

static FORCEINLINE UNetConnection* IsActorOwnedByAndRelevantToConnection(const AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
	const AActor* ActorOwner = Actor->GetNetOwner();

	bOutHasNullViewTarget = false;

	for (int i = 0; i < ConnectionViewers.size(); i++)
	{
		UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

		if (ViewerConnection->GetViewTarget() == nullptr)
		{
			bOutHasNullViewTarget = true;
		}

		if (ActorOwner == ViewerConnection->GetPlayerController() ||
			(ViewerConnection->GetPlayerController() && ActorOwner == ViewerConnection->GetPlayerController()->GetPawn()) ||
			(ViewerConnection->GetViewTarget() && ViewerConnection->GetViewTarget()->IsRelevancyOwnerFor(Actor, ActorOwner, ViewerConnection->GetOwningActor())))
		{
			return ViewerConnection;
		}
	}

	return nullptr;
}

struct FCompareFActorPriority
{
	FORCEINLINE bool operator()(const FActorPriority& A, const FActorPriority& B) const
	{
		return B.Priority < A.Priority;
	}
};

static FORCEINLINE bool ShouldActorGoDormant(AActor* Actor, const std::vector<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth)
{
	using enum ENetDormancy;

	static auto bPendingDormancyOffset = 0x30;
	static auto bPendingDormancyFieldMask = 0x0;
	static auto DormantOffset = 0x30;
	static auto DormantFieldMask = 0x0;

	if (Actor->GetNetDormancy() <= DORM_Awake || !Channel 
		// || ReadBitfield((PlaceholderBitfield*)(__int64(Channel) + bPendingDormancyOffset), bPendingDormancyFieldMask) 
		// || ReadBitfield((PlaceholderBitfield*)(__int64(Channel) + DormantOffset), DormantFieldMask)
		|| Channel->IsPendingDormancy()
		|| Channel->IsDormant()
		)
	{
		// Either shouldn't go dormant, or is already dormant
		return false;
	}

	if (Actor->GetNetDormancy() == DORM_DormantPartial)
	{
		for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.size(); viewerIdx++)
		{
			// if (!Actor->GetNetDormancy(ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
			if (!false) // ^ this just returns false soo (atleast AActor implementation)
			{
				return false;
			}
		}
	}

	return true;
}

UActorChannel* FindChannel(UNetConnection* Connection, FNetworkObjectInfo* ActorInfo)
{
	if (true) 
	{
		auto& ActorChannels = Connection->GetActorChannels();

		for (auto& Pair : ActorChannels)
		{
			if (Pair.First == ActorInfo->WeakActor)
			{
				return Pair.Second;
			}
		}

		return nullptr;
	}

	auto Actor = ActorInfo->Actor;

	if (!Actor)
		return nullptr;

	static auto OpenChannelsOffset = Connection->GetOffset("OpenChannels");
	auto& OpenChannels = Connection->Get<TArray<UChannel*>>(OpenChannelsOffset);

	static auto ActorChannelClass = FindObject<UClass>("/Script/Engine.ActorChannel");

	// LOG_INFO(LogReplication, "OpenChannels.Num(): {}", OpenChannels.Num());

	for (int i = 0; i < OpenChannels.Num(); i++)
	{
		auto Channel = OpenChannels.at(i);

		if (!Channel)
			continue;

		// LOG_INFO(LogReplication, "[{}] Class {}", i, Channel->ClassPrivate ? Channel->ClassPrivate->GetFullName() : "InvalidObject");

		// if (!Channel->IsA(ActorChannelClass))
			// continue;

		if (Channel->ClassPrivate != ActorChannelClass)
			continue;

		static auto ActorOffset = Channel->GetOffset("Actor");

		if (Channel->Get<AActor*>(ActorOffset) != Actor)
			continue;

		return (UActorChannel*)Channel;
	}

	return NULL;
}

int32 UNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, const std::vector<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
	GetNetTag()++;
	Connection->GetTickCount()++;

	for (int32 j = 0; j < Connection->GetSentTemporaries().Num(); j++) 	// Set up to skip all sent temporary actors
	{
		Connection->GetSentTemporaries().at(j)->GetNetTag() = GetNetTag();
	}

	// check( World == Connection->OwningActor->GetWorld() );

	int32 FinalSortedCount = 0;
	int32 DeletedCount = 0;

	// Make weak ptr once for IsActorDormant call
	TWeakObjectPtr<UNetConnection> WeakConnection{};
	WeakConnection.ObjectIndex = Connection->InternalIndex;
	WeakConnection.ObjectSerialNumber = GetItemByIndex(Connection->InternalIndex)->SerialNumber;

	auto& Connection_DestroyedStartupOrDormantActors = GetConnectionDestroyedStartupOrDormantActors(Connection);

	const int32 MaxSortedActors = ConsiderList.size() + Connection_DestroyedStartupOrDormantActors.Num();

	if (MaxSortedActors > 0)
	{
		OutPriorityList = Alloc<FActorPriority>(MaxSortedActors * sizeof(FActorPriority));
			// (FActorPriority*)FMemory::Realloc(nullptr, MaxSortedActors * sizeof(FActorPriority), 0);
		OutPriorityActors = Alloc<FActorPriority*>(MaxSortedActors * sizeof(FActorPriority*));
			// (FActorPriority**)FMemory::Realloc(nullptr, MaxSortedActors * sizeof(FActorPriority*), 0);

		// check( World == Connection->ViewTarget->GetWorld() );

		// AGameNetworkManager* const NetworkManager = World->NetworkManager;
		const bool bLowNetBandwidth = false; // NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false;

		for (int i = 0; i < ConsiderList.size(); i++)
		{
			FNetworkObjectInfo* ActorInfo = ConsiderList.at(i);
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = FindChannel(Connection, ActorInfo);

			if (!Channel)
			{
				if (!IsLevelInitializedForActor(Actor, Connection))
				{
					// If the level this actor belongs to isn't loaded on client, don't bother sending
					continue;
				}

				if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					// If not relevant (and we don't have a channel), skip
					continue;
				}
			}

			// LOG_INFO(LogDev, "Actor->GetNetTag(): {}", Actor->GetNetTag());

			UNetConnection* PriorityConnection = Connection;

			if (Actor->IsOnlyRelevantToOwner())
			{
				// This actor should be owned by a particular connection, see if that connection is the one passed in
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
			else if (CVarSetNetDormancyEnabled != 0)
			{
				// Skip Actor if dormant
				if (IsActorDormant(ActorInfo, WeakConnection))
				{
					continue;
				}

				// See of actor wants to try and go dormant
				if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, GetTime(), bLowNetBandwidth))
				{
					// LOG_INFO(LogDev, "Actor is going dormant!");

					// Channel is marked to go dormant now once all properties have been replicated (but is not dormant yet)
					Channel->StartBecomingDormant();
				}
			}

			// Actor is relevant to this connection, add it to the list
			// NOTE - We use NetTag to make sure SentTemporaries didn't already mark this actor to be skipped

			if (Actor->GetNetTag() != GetNetTag())
			{
				Actor->GetNetTag() = GetNetTag();

				OutPriorityList[FinalSortedCount] = FActorPriority(PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth);
				OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

				FinalSortedCount++;
				// Test3(Actor, "ryo i got added");
			}
		}

		// Add in deleted actors

		for (auto& CurrentGuid : Connection_DestroyedStartupOrDormantActors)
		{
			bool bFound = false;

			FActorDestructionInfo& DInfo = GetDriverDestroyedStartupOrDormantActors(this).Find(CurrentGuid, &bFound);

			if (!bFound)
				continue;

			OutPriorityList[FinalSortedCount] = FActorPriority(Connection, &DInfo, ConnectionViewers);
			OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
			FinalSortedCount++;
			DeletedCount++;
		}

		Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
	}

	return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
{
	static UChannel* (*CreateChannel)(UNetConnection*, int, bool, int32_t) = decltype(CreateChannel)(Addresses::CreateChannel);
	static __int64 (*ReplicateActor)(UActorChannel*) = decltype(ReplicateActor)(Addresses::ReplicateActor);
	static __int64 (*SetChannelActor)(UActorChannel*, AActor*) = decltype(SetChannelActor)(Addresses::SetChannelActor);

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

		// Deletion entry
		if (ActorInfo == NULL && PriorityActors[j]->DestructionInfo)
		{
			// Make sure client has streaming level loaded
			if (PriorityActors[j]->DestructionInfo->StreamingLevelName.ComparisonIndex.Value != NAME_None 
				&& !Connection->GetClientVisibleLevelNames().Contains(PriorityActors[j]->DestructionInfo->StreamingLevelName)
				)
			{
				// This deletion entry is for an actor in a streaming level the connection doesn't have loaded, so skip it
				continue;
			}

			auto& Connection_DestroyedStartupOrDormantActors = GetConnectionDestroyedStartupOrDormantActors(Connection);

			/* bool bFound = false;

			for (auto& aa : Connection_DestroyedStartupOrDormantActors)
			{
				if (aa == PriorityActors[j]->DestructionInfo->NetGUID)
				{
					bFound = true;
					break;
				}
			}

			LOG_INFO(LogDev, "bFound: {}", bFound);

			if (!bFound)
				continue; */

			UActorChannel* Channel = (UActorChannel*)CreateChannel(Connection, 2, true, -1);

			if (Channel)
			{
				FinalRelevantCount++;

				SetChannelActorForDestroy(Channel, PriorityActors[j]->DestructionInfo);						   // Send a close bunch on the new channel
				Connection_DestroyedStartupOrDormantActors.Remove(PriorityActors[j]->DestructionInfo->NetGUID); // Remove from connections to-be-destroyed list (close bunch of reliable, so it will make it there)
				LOG_INFO(LogDev, "Finished!");
			}

			continue;
		}

		// Normal actor replication
		UActorChannel* Channel = PriorityActors[j]->Channel;

		if (!Channel || Channel->GetActor()) //make sure didn't just close this channel
		{
			AActor* Actor = ActorInfo->Actor;
			// Test2(Actor, "Before");

			bool bIsRelevant = false;

			const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

			// only check visibility on already visible actors every 1.0 + 0.5R seconds
			// bTearOff actors should never be checked
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

			// Test3(Actor, std::format("bIsRecentlyRelevant: {} bLevelInitializedForActor: {} Channel: {} bIsRelevant: {}", (int)bIsRecentlyRelevant, bLevelInitializedForActor, __int64(Channel), (int)bIsRelevant));
			// Test2(Actor, std::format("bIsRelevant: {} bLevelInitializedForActor: {} Cond: {}", bIsRelevant, bLevelInitializedForActor, !Actor->IsTearOff() && (!Channel || GetTime() - Channel->GetRelevantTime() > 1.f)));
			// Test2(Actor, std::format("TearOff: {} GetTime(): {}  Channel->GetRelevantTime(): {}", !Actor->IsTearOff(), GetTime(), Channel ? Channel->GetRelevantTime() : 99));

			ActorInfo->bForceRelevantNextUpdate = false;

			if (bIsRecentlyRelevant)
			{
				FinalRelevantCount++;

				if (Channel == NULL 
					/* && GetGuidCache()->SupportsObject(Actor->ClassPrivate)
					&& GetGuidCache()->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype()) */
					)
				{
					if (bLevelInitializedForActor)
					{
						// Create a new channel for this actor.
						UActorChannel* Channel = (UActorChannel*)CreateChannel(Connection, 2, true, -1);

						if (Channel)
						{
							SetChannelActor(Channel, Actor);
						}
					}
					// if we couldn't replicate it for a reason that should be temporary, and this Actor is updated very infrequently, make sure we update it again soon
					else if (Actor->GetNetUpdateFrequency() < 1.0f)
					{
						auto TimeSeconds = GetTimeSecondsForWorld(GetWorld()); // Actor->GetWorld()->TimeSeconds
						ActorInfo->NextUpdateTime = TimeSeconds + 0.2f * FRand();
					}
				}

				if (Channel)
				{
					// if it is relevant then mark the channel as relevant for a short amount of time
					if (bIsRelevant)
					{
						// Channel->GetRelevantTime() = GetTime() - 1.0; // + 0.5f * SRand(); // scufed fn wtf
						Channel->GetRelevantTime() = GetTime() + 0.5f * SRand();
					}

					if (Channel->IsNetReady(0))
					{
						if (ReplicateActor(Channel))
						{
							ActorUpdatesThisConnectionSent++;

							// Calculate min delta (max rate actor will upate), and max delta (slowest rate actor will update)
							const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
							const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);
							const float DeltaBetweenReplications = (GetTimeSecondsForWorld(GetWorld()) - ActorInfo->LastNetReplicateTime);

							// Choose an optimal time, we choose 70% of the actual rate to allow frequency to go up if needed
							ActorInfo->OptimalNetUpdateDelta = FMath::Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
							ActorInfo->LastNetReplicateTime = GetTimeSecondsForWorld(GetWorld());
							// ReplicatedActors.emplace(Actor->GetFullName());
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

	// LOG_INFO(LogDev, "FinalRelevantCount: {} ActorUpdatesThisConnection: {} ActorUpdatesThisConnectionSent: {}", FinalRelevantCount, ActorUpdatesThisConnection, ActorUpdatesThisConnectionSent);

	return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors()
{
	int32 Updated = 0;

	++(*(int*)(__int64(this) + Offsets::ReplicationFrame));

	const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(this);

	if (NumClientsToTick == 0)
	{
		// No connections are ready this frame
		return 0;
	}

	auto World = GetNetDriverWorld();

	AWorldSettings* WorldSettings = World->GetWorldSettings();

	// bool bCPUSaturated = false;
	float ServerTickTime = GetMaxTickRateHook();
	if (ServerTickTime == 0.f)
	{
		// ServerTickTime = DeltaSeconds;
	}
	else
	{
		ServerTickTime = 1.f / ServerTickTime;
		// bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
	}

	std::vector<FNetworkObjectInfo*> ConsiderList;

	ConsiderList.reserve(GetNetworkObjectList().ActiveNetworkObjects.Num());

	// std::cout << "ConsiderList.size(): " << GetNetworkObjectList(NetDriver).ActiveNetworkObjects.Num() << '\n';

	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

	bool bCPUSaturated = false;

	auto& ClientConnections = GetClientConnections();

	for (int32 i = 0; i < ClientConnections.Num(); i++)
	{
		UNetConnection* Connection = ClientConnections.at(i);

		if (!Connection)
			continue;

		// net.DormancyValidate can be set to 2 to validate all dormant actors against last known state before going dormant
		/* if (CVarNetDormancyValidate.GetValueOnAnyThread() == 2)
		{
			for (auto It = Connection->DormantReplicatorMap.CreateIterator(); It; ++It)
			{
				FObjectReplicator& Replicator = It.Value().Get();

				if (Replicator.OwningChannel != nullptr)
				{
					Replicator.ValidateAgainstState(Replicator.OwningChannel->GetActor());
				}
			}
		} */

		// if this client shouldn't be ticked this frame
		if (i >= NumClientsToTick)
		{
			//UE_LOG(LogNet, Log, TEXT("skipping update to %s"),*Connection->GetName());
			// then mark each considered actor as bPendingNetUpdate so that they will be considered again the next frame when the connection is actually ticked
			for (int32 ConsiderIdx = 0; ConsiderIdx < ConsiderList.size(); ConsiderIdx++)
			{
				AActor* Actor = ConsiderList[ConsiderIdx]->Actor;
				// if the actor hasn't already been flagged by another connection,
				if (Actor != NULL && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
				{
					// find the channel

					UActorChannel* Channel = FindChannel(Connection, ConsiderList[ConsiderIdx]);

					// and if the channel last update time doesn't match the last net update time for the actor
					if (Channel != NULL && Channel->GetLastUpdateTime() < ConsiderList[ConsiderIdx]->LastNetUpdateTime)
					{
						ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
					}
				}
			}

			static auto TimeSensitiveOffset = 0x241;
			Connection->Get<bool>(TimeSensitiveOffset) = false;
		}
		else if (Connection->GetViewTarget())
		{
			// Make a list of viewers this connection should consider (this connection and children of this connection)
			// TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

			// ConnectionViewers.Reset();
			std::vector<FNetViewer> ConnectionViewers;
			ConnectionViewers.push_back(ConstructNetViewer(Connection));

			// send ClientAdjustment if necessary
			// we do this here so that we send a maximum of one per packet to that client; there is no value in stacking additional corrections
			if (Connection->GetPlayerController())
			{
				static void (*SendClientAdjustment)(APlayerController*) = decltype(SendClientAdjustment)(Addresses::SendClientAdjustment);
				SendClientAdjustment(Connection->GetPlayerController());
			}

			FActorPriority* PriorityList = NULL;
			FActorPriority** PriorityActors = NULL;

			// LOG_INFO(LogDev, "ConsiderList.size(): {}", ConsiderList.size());

			// Get a sorted list of actors for this connection
			const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

			// Process the sorted list of actors for this connection
			const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

			// LOG_INFO(LogDev, "LastProcessedActor: {} FinalSortedCount: {} NetTag: {}", LastProcessedActor, FinalSortedCount, GetNetTag());

			// relevant actors that could not be processed this frame are marked to be considered for next frame
			for (int32 k = LastProcessedActor; k < FinalSortedCount; k++)
			{
				if (!PriorityActors[k]->ActorInfo)
				{
					// A deletion entry, skip it because we dont have anywhere to store a 'better give higher priority next time'
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
						Channel->GetRelevantTime() = GetTime() + 0.5f * SRand();
					}
				}
			}

			ConnectionViewers.clear();
		}
	}

	// shuffle the list of connections if not all connections were ticked
	/* if (NumClientsToTick < ClientConnections.Num())
	{
		int32 NumConnectionsToMove = NumClientsToTick;
		while (NumConnectionsToMove > 0)
		{
			// move all the ticked connections to the end of the list so that the other connections are considered first for the next frame
			UNetConnection* Connection = ClientConnections.at(0);
			ClientConnections.RemoveAt(0, 1);
			ClientConnections.Add(Connection);
			NumConnectionsToMove--;
		}
	} */

	return Updated;
}