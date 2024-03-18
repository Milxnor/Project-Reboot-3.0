#include "NetDriver.h"

#include "reboot.h"
#include "Actor.h"
#include "NetConnection.h"
#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"
#include "KismetMathLibrary.h"
#include <random>
#include "Package.h"S
#include "AssertionMacros.h"
#include "bots.h"
#include "gui.h"

FNetworkObjectList& UNetDriver::GetNetworkObjectList()
{
	return *(*(TSharedPtr<FNetworkObjectList>*)(__int64(this) + Offsets::NetworkObjectList));
}

bool ShouldUseNetworkObjectList()
{
	return Fortnite_Version < 20;
}

void UNetDriver::RemoveNetworkActor(AActor* Actor)
{
	GetNetworkObjectList().Remove(Actor);

	// RenamedStartupActors.Remove(Actor->GetFName());
}

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

enum class EChannelCreateFlags : uint32_t
{
	None = (1 << 0),
	OpenedLocally = (1 << 1)
};

int32 ServerReplicateActors_PrepConnections(UNetDriver* NetDriver)
{
	auto& ClientConnections = NetDriver->GetClientConnections();

	int32 NumClientsToTick = ClientConnections.Num();

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		UNetConnection* Connection = ClientConnections.at(ConnIdx);
		if (!Connection) continue;
		// check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		// checkSlow(Connection->GetUChildConnection() == NULL);

		AActor* OwningActor = Connection->GetOwningActor();

		if (OwningActor != NULL) // && /* Connection->State == USOCK_Open && */ (Connection->Driver->Time - Connection->LastReceiveTime < 1.5f))
		{
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
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);
	float random_number = dis(gen);

	return random_number;
}

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

void UNetDriver::ServerReplicateActors_BuildConsiderList(std::vector<FNetworkObjectInfo*>& OutConsiderList, float ServerTickTime)
{
	std::vector<AActor*> ActorsToRemove;

	if (ShouldUseNetworkObjectList())
	{
		auto& ActiveObjects = GetNetworkObjectList().ActiveNetworkObjects;

		auto World = GetWorld();

		for (const TSharedPtr<FNetworkObjectInfo>& ActorInfo : ActiveObjects)
		{
			if (!ActorInfo->bPendingNetUpdate && UGameplayStatics::GetTimeSeconds(GetWorld()) <= ActorInfo->NextUpdateTime)
			{
				continue;
			}

			auto Actor = ActorInfo->Actor;

			if (!Actor)
				continue;

			if (Actor->IsPendingKillPending())
				// if (Actor->IsPendingKill())
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

			static auto NetDormancyOffset = Actor->GetOffset("NetDormancy");

			if (Actor->Get<ENetDormancy>(NetDormancyOffset) == ENetDormancy::DORM_Initial && Actor->IsNetStartupActor()) // IsDormInitialStartupActor
			{
				continue;
			}

			// We should check NeedsLoadForClient here.
			// We should make sure the actor is in the same world here but I don't believe it is needed.

			if (ActorInfo->LastNetReplicateTime == 0)
			{
				ActorInfo->LastNetReplicateTime = UGameplayStatics::GetTimeSeconds(World);
				ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->GetNetUpdateFrequency();
			}

			const float ScaleDownStartTime = 2.0f;
			const float ScaleDownTimeRange = 5.0f;

			const float LastReplicateDelta = UGameplayStatics::GetTimeSeconds(World) - ActorInfo->LastNetReplicateTime;

			if (LastReplicateDelta > ScaleDownStartTime)
			{
				static auto MinNetUpdateFrequencyOffset = Actor->GetOffset("MinNetUpdateFrequency");

				if (Actor->Get<float>(MinNetUpdateFrequencyOffset) == 0.0f)
				{
					Actor->Get<float>(MinNetUpdateFrequencyOffset) = 2.0f;
				}

				const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();									  // Don't go faster than NetUpdateFrequency
				const float MaxOptimalDelta = max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta); // Don't go slower than MinNetUpdateFrequency (or NetUpdateFrequency if it's slower)

				const float Alpha = std::clamp((LastReplicateDelta - ScaleDownStartTime) / ScaleDownTimeRange, 0.0f, 1.0f); // should we use fmath?
				ActorInfo->OptimalNetUpdateDelta = std::lerp(MinOptimalDelta, MaxOptimalDelta, Alpha); // should we use fmath?
			}

			if (!ActorInfo->bPendingNetUpdate)
			{
				constexpr bool bUseAdapativeNetFrequency = false;
				const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->GetNetUpdateFrequency();

				// then set the next update time
				float ServerTickTime = 1.f / 30;
				ActorInfo->NextUpdateTime = UGameplayStatics::GetTimeSeconds(World) + FRand() * ServerTickTime + NextUpdateDelta;
				static auto TimeOffset = GetOffset("Time");
				ActorInfo->LastNetUpdateTime = Get<float>(TimeOffset);
			}

			ActorInfo->bPendingNetUpdate = false;

			OutConsiderList.push_back(ActorInfo.Get());

			static void (*CallPreReplication)(AActor*, UNetDriver*) = decltype(CallPreReplication)(Addresses::CallPreReplication);
			CallPreReplication(Actor, this);
		}
	}
	else
	{
		auto Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass());

		for (int i = 0; i < Actors.Num(); ++i)
		{
			auto Actor = Actors.at(i);

			if (Actor->IsPendingKillPending())
				// if (Actor->IsPendingKill())
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

			static auto NetDormancyOffset = Actor->GetOffset("NetDormancy");

			if (Actor->Get<ENetDormancy>(NetDormancyOffset) == ENetDormancy::DORM_Initial && Actor->IsNetStartupActor()) // IsDormInitialStartupActor
			{
				continue;
			}

			auto ActorInfo = new FNetworkObjectInfo;
			ActorInfo->Actor = Actor;

			OutConsiderList.push_back(ActorInfo);

			static void (*CallPreReplication)(AActor*, UNetDriver*) = decltype(CallPreReplication)(Addresses::CallPreReplication);
			CallPreReplication(Actor, this);
		}

		Actors.Free();
	}

	for (auto Actor : ActorsToRemove)
	{
		if (!Actor)
			continue;

		/* LOG_INFO(LogDev, "Removing actor: {}", Actor ? Actor->GetFullName() : "InvalidObject");
		RemoveNetworkActor(Actor);
		LOG_INFO(LogDev, "Finished removing actor."); */
	}
}

static UActorChannel* FindChannel(AActor * Actor, UNetConnection * Connection)
{
	static auto OpenChannelsOffset = Connection->GetOffset("OpenChannels");
	auto& OpenChannels = Connection->Get<TArray<UChannel*>>(OpenChannelsOffset);

	static auto ActorChannelClass = FindObject<UClass>(L"/Script/Engine.ActorChannel");

	// LOG_INFO(LogReplication, "OpenChannels.Num(): {}", OpenChannels.Num());

	for (int i = 0; i < OpenChannels.Num(); ++i)
	{
		auto Channel = OpenChannels.at(i);

		if (!Channel)
			continue;

		// LOG_INFO(LogReplication, "[{}] Class {}", i, Channel->ClassPrivate ? Channel->ClassPrivate->GetFullName() : "InvalidObject");

		if (!Channel->IsA(ActorChannelClass)) // (Channel->ClassPrivate == ActorChannelClass)
			continue;

		static auto ActorOffset = Channel->GetOffset("Actor");
		auto ChannelActor = Channel->Get<AActor*>(ActorOffset);

		// LOG_INFO(LogReplication, "[{}] {}", i, ChannelActor->GetFullName());

		if (ChannelActor != Actor)
			continue;

		return (UActorChannel*)Channel;
	}

	// LOG_INFO(LogDev, "Failed to find channel for {}!", Actor->GetName());

	return nullptr;
}

static bool IsActorRelevantToConnection(AActor * Actor, std::vector<FNetViewer>&ConnectionViewers)
{
	for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.size(); viewerIdx++)
	{
		if (!ConnectionViewers[viewerIdx].ViewTarget)
			continue;

		// static bool (*IsNetRelevantFor)(AActor*, AActor*, AActor*, FVector&) = decltype(IsNetRelevantFor)(__int64(GetModuleHandleW(0)) + 0x1ECC700);

		static auto index = Offsets::IsNetRelevantFor;

		// if (Actor->IsNetRelevantFor(ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		// if (IsNetRelevantFor(Actor, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		if (reinterpret_cast<bool(*)(AActor*, AActor*, AActor*, FVector&)>(Actor->VFTable[index])(
			Actor, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
		{
			return true;
		}
	}

	return false;
}

static FNetViewer ConstructNetViewer(UNetConnection* NetConnection)
{
	FNetViewer newViewer{};
	newViewer.Connection = NetConnection;
	newViewer.InViewer = NetConnection->GetPlayerController() ? NetConnection->GetPlayerController() : NetConnection->GetOwningActor();
	newViewer.ViewTarget = NetConnection->GetViewTarget();

	if (!NetConnection->GetOwningActor() || !(!NetConnection->GetPlayerController() || (NetConnection->GetPlayerController() == NetConnection->GetOwningActor())))
		return newViewer;

	APlayerController* ViewingController = NetConnection->GetPlayerController();

	newViewer.ViewLocation = newViewer.ViewTarget->GetActorLocation();

	if (ViewingController)
	{
		static auto ControlRotationOffset = ViewingController->GetOffset("ControlRotation");
		FRotator ViewRotation = ViewingController->Get<FRotator>(ControlRotationOffset); // hmmmm // ViewingController->GetControlRotation();
		// AFortPlayerControllerAthena::GetPlayerViewPointHook(Cast<AFortPlayerControllerAthena>(ViewingController, false), newViewer.ViewLocation, ViewRotation);
		ViewingController->GetActorEyesViewPoint(&newViewer.ViewLocation, &ViewRotation); // HMMM

		// static auto GetActorEyesViewPointOffset = 0x5B0;
		// void (*GetActorEyesViewPointOriginal)(AController*, FVector * a2, FRotator * a3) = decltype(GetActorEyesViewPointOriginal)(ViewingController->VFTable[GetActorEyesViewPointOffset / 8]);
		// GetActorEyesViewPointOriginal(ViewingController, &newViewer.ViewLocation, &ViewRotation);
		// AFortPlayerControllerAthena::GetPlayerViewPointHook((AFortPlayerControllerAthena*)ViewingController, newViewer.ViewLocation, ViewRotation);
		newViewer.ViewDir = ViewRotation.Vector();
	}

	return newViewer;
}

static FORCEINLINE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection)
{
	// If actor is already dormant on this channel, then skip replication entirely
	return ActorInfo->DormantConnections.Contains(Connection);
}

bool UNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
	if (Fortnite_Version >= 2.42) // idk
	{
		return true;
	}

/* #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // (Milxnor) This is on some ue versions and others not.
	if (!InActor || !InConnection)
		return false;

	// check(World == InActor->GetWorld());
#endif */

	bool bFirstWorldCheck = Engine_Version == 416 
		? (InConnection->GetClientWorldPackageName() == GetWorld()->GetOutermost()->GetFName())
		: (InConnection->GetClientWorldPackageName() == GetWorldPackage()->NamePrivate);

	const bool bCorrectWorld = (bFirstWorldCheck && InConnection->ClientHasInitializedLevelFor(InActor));
	const bool bIsConnectionPC = (InActor == InConnection->GetPlayerController());
	return bCorrectWorld || bIsConnectionPC;
}

TMap<FNetworkGUID, FActorDestructionInfo>* GetDestroyedStartupOrDormantActors(UNetDriver* Driver)
{
	static int off = Fortnite_Version == 1.11 ? 0x228 : 0; // 0x240

	return off == 0 ? nullptr : (TMap<FNetworkGUID, FActorDestructionInfo>*)(__int64(Driver) + off);
}

TSet<FNetworkGUID>* GetDestroyedStartupOrDormantActors(UNetConnection* NetConnection)
{
	static int off = Fortnite_Version == 1.11 ? 0x33678 : 0;

	return off == 0 ? nullptr : (TSet<FNetworkGUID>*)(__int64(NetConnection) + off);
}

using FArchive = void;

bool IsError(FArchive* Ar)
{
	return false;
}

void SerializeChecksum(FArchive* Ar, uint32 x, bool ErrorOK)
{
	/*
	if (Ar->IsLoading())
	{
		uint32 Magic = 0;
		Ar << Magic;
		if ((!ErrorOK || !IsError(Ar)) 
			// && !ensure(Magic == x)
			)
		{
			// UE_LOG(LogCoreNet, Warning, TEXT("%d == %d"), Magic, x);
		}

	}
	else
	{
		uint32 Magic = x;
		Ar << Magic;
	}
	*/
}

#define NET_CHECKSUM(Ser) \
{ \
	SerializeChecksum(Ser,0xE282FA84, false); \
}

struct FPacketIdRange
{
	FPacketIdRange(int32 _First, int32 _Last) : First(_First), Last(_Last) { }
	FPacketIdRange(int32 PacketId) : First(PacketId), Last(PacketId) { }
	FPacketIdRange() : First(INDEX_NONE), Last(INDEX_NONE) { }
	int32 First;
	int32 Last;

	bool InRange(int32 PacketId) const
	{
		return (First <= PacketId && PacketId <= Last);
	}
};

void SetChannelActorForDestroy(UActorChannel* Channel, FActorDestructionInfo* DestructInfo)
{
	auto Connection = Channel->GetConnection();

	if (
		true
		// && !Channel->IsClosing()
			// && (Connection->State == USOCK_Open || Connection->State == USOCK_Pending)
		)
	{

		// Send a close notify, and wait for ack.
		struct FOutBunch
		{
			char pad[0x600]; // idk real size
		};

		FOutBunch CloseBunch{};
		FOutBunch(*ConstructorFOutBunch)(FOutBunch*, UChannel* , bool) = decltype(ConstructorFOutBunch)(__int64(GetModuleHandleW(0)) + 0x194E800);
		ConstructorFOutBunch(&CloseBunch, Channel, 1);
		// check(!CloseBunch.IsError());
		// check(CloseBunch.bClose);

		// https://imgur.com/a/EtKFkrD

		*(bool*)(__int64(&CloseBunch) + 0xE8) = 1;
		*(bool*)(__int64(&CloseBunch) + 0xE6) = 0;

		// Serialize DestructInfo
		// NET_CHECKSUM(CloseBunch); // This is to mirror the Checksum in UPackageMapClient::SerializeNewActor

		using UPackageMap = UObject;

		reinterpret_cast<bool(*)(UPackageMap*, FArchive * Ar, UObject * InOuter,FNetworkGUID NetGUID, FString ObjName)>(Connection->GetPackageMap()->VFTable[0x238 / 8])(Connection->GetPackageMap(), &CloseBunch, DestructInfo->ObjOuter.Get(), DestructInfo->NetGUID, DestructInfo->PathName);

		// UE_LOG(LogNetTraffic, Log, TEXT("SetChannelActorForDestroy: Channel %d. NetGUID <%s> Path: %s. Bits: %d"), ChIndex, *DestructInfo->NetGUID.ToString(), *DestructInfo->PathName, CloseBunch.GetNumBits());
		// UE_LOG(LogNetDormancy, Verbose, TEXT("SetChannelActorForDestroy: Channel %d. NetGUID <%s> Path: %s. Bits: %d"), ChIndex, *DestructInfo->NetGUID.ToString(), *DestructInfo->PathName, CloseBunch.GetNumBits());

		// 0x196E9C0
		reinterpret_cast<FPacketIdRange(*)(UActorChannel*, FOutBunch*, bool)>(Channel->VFTable[0x288 / 8])(Channel, &CloseBunch, false);
	}
}

enum ESetChannelActorFlags
{
	None = 0,
	SkipReplicatorCreation = (1 << 0),
	SkipMarkActive = (1 << 1),
};

TSet<FName>* GetClientVisibleLevelNames(UNetConnection* NetConnection)
{
	return (TSet<FName>*)(__int64(NetConnection) + 0x336C8);
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

	// AFortWorldSettings* WorldSettings = GetFortWorldSettings(NetDriver->World);

	// bool bCPUSaturated = false;
	float ServerTickTime = GetMaxTickRateHook();
	/* if (ServerTickTime == 0.f)
	{
		ServerTickTime = DeltaSeconds;
	}
	else */
	{
		ServerTickTime = 1.f / ServerTickTime;
		// bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
	}

	std::vector<FNetworkObjectInfo*> ConsiderList;

	if (ShouldUseNetworkObjectList())
		ConsiderList.reserve(GetNetworkObjectList().ActiveNetworkObjects.Num());

	auto World = GetWorld();

	ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

	// LOG_INFO(LogReplication, "Considering {} actors.", ConsiderList.size());

	static UChannel* (*CreateChannel)(UNetConnection*, int, bool, int32_t) = decltype(CreateChannel)(Addresses::CreateChannel);
	static __int64 (*ReplicateActor)(UActorChannel*) = decltype(ReplicateActor)(Addresses::ReplicateActor);
	static UObject* (*CreateChannelByName)(UNetConnection * Connection, FName * ChName, EChannelCreateFlags CreateFlags, int32_t ChannelIndex) = decltype(CreateChannelByName)(Addresses::CreateChannel);
	static __int64 (*SetChannelActor)(UActorChannel*, AActor*) = decltype(SetChannelActor)(Addresses::SetChannelActor);
	static __int64 (*SetChannelActor2)(UActorChannel*, AActor*, ESetChannelActorFlags) = decltype(SetChannelActor2)(Addresses::SetChannelActor);

	for (int32 i = 0; i < this->GetClientConnections().Num(); i++)
	{
		UNetConnection* Connection = this->GetClientConnections().at(i);

		if (!Connection)
			continue;

		if (i >= NumClientsToTick)
			continue;

		if (!Connection->GetViewTarget())
			continue;

		if (Addresses::SendClientAdjustment)
		{
			if (Connection->GetPlayerController())
			{
				static void (*SendClientAdjustment)(APlayerController*) = decltype(SendClientAdjustment)(Addresses::SendClientAdjustment);
				SendClientAdjustment(Connection->GetPlayerController());
			}
		}

		// Make weak ptr once for IsActorDormant call
		TWeakObjectPtr<UNetConnection> WeakConnection{};
		WeakConnection.ObjectIndex = Connection->InternalIndex;
		WeakConnection.ObjectSerialNumber = GetItemByIndex(Connection->InternalIndex)->SerialNumber;

		/* GetNetTag()++;
		Connection->GetTickCount()++;

		for (int32 j = 0; j < Connection->GetSentTemporaries().Num(); j++) 	// Set up to skip all sent temporary actors
		{
			Connection->GetSentTemporaries().at(j)->GetNetTag() = GetNetTag();
		} */

		std::vector<FActorDestructionInfo*> DeletionEntries;

#if 0
		auto ConnectionDestroyedStartupOrDormantActors = GetDestroyedStartupOrDormantActors(Connection);

		if (ConnectionDestroyedStartupOrDormantActors)
		{
			auto DriverDestroyedStartupOrDormantActors = GetDestroyedStartupOrDormantActors(this);

			if (DriverDestroyedStartupOrDormantActors)
			{
				for (FNetworkGUID& ConnectionIt : *ConnectionDestroyedStartupOrDormantActors)
				{
					FActorDestructionInfo* DInfo = nullptr;

					for (TPair<FNetworkGUID, FActorDestructionInfo>& DriverIt : *DriverDestroyedStartupOrDormantActors)
					{
						if (DriverIt.First == ConnectionIt)
						{
							DInfo = &DriverIt.Second;
							break;
						}
					}

					if (!DInfo) continue; // should never happen

					DeletionEntries.push_back(DInfo);
				}
			}
		}

		LOG_INFO(LogDev, "DeletionEntries: {}", DeletionEntries.size());
#endif

		for (FActorDestructionInfo* DeletionEntry : DeletionEntries)
		{
			LOG_INFO(LogDev, "AA: {}", DeletionEntry->PathName.Data.Data ? DeletionEntry->PathName.ToString() : "Null");

			if (DeletionEntry->StreamingLevelName != -1)
			{
				auto ClientVisibleLevelNames = GetClientVisibleLevelNames(Connection);

				bool bFound = false;

				for (FName& ClientVisibleLevelName : *ClientVisibleLevelNames)
				{
					if (ClientVisibleLevelName == DeletionEntry->StreamingLevelName)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					continue;
			}

			UActorChannel* Channel = nullptr;

			if (Engine_Version >= 422)
			{
				FString ActorStr = L"Actor";
				FName ActorName = UKismetStringLibrary::Conv_StringToName(ActorStr);

				int ChannelIndex = -1; // 4294967295
				Channel = (UActorChannel*)CreateChannelByName(Connection, &ActorName, EChannelCreateFlags::OpenedLocally, ChannelIndex);
			}
			else
			{
				Channel = (UActorChannel*)CreateChannel(Connection, 2, true, -1);
			}

			if (Channel)
			{
				// FinalRelevantCount++;

				SetChannelActorForDestroy(Channel, DeletionEntry);						   // Send a close bunch on the new channel
				GetDestroyedStartupOrDormantActors(Connection)->Remove(DeletionEntry->NetGUID); // Remove from connections to-be-destroyed list (close bunch of reliable, so it will make it there)
			}
		}

		for (auto& ActorInfo : ConsiderList)
		{
			if (!ActorInfo || !ActorInfo->Actor)
				continue;

			auto Actor = ActorInfo->Actor;

			auto Channel = FindChannel(Actor, Connection);

			/* if (IsActorDormant(ActorInfo, WeakConnection))
			{
				continue;
			} */

			std::vector<FNetViewer> ConnectionViewers;
			ConnectionViewers.push_back(ConstructNetViewer(Connection));

			const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

			if (!Channel)
			{
				// if (!IsLevelInitializedForActor(Actor, Connection))
				if (!bLevelInitializedForActor)
				{
					// If the level this actor belongs to isn't loaded on client, don't bother sending
					continue;
				}

				/* if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					// If not relevant (and we don't have a channel), skip
					continue;
				} */
			}

			bool bLowNetBandwidth = false;

			// See of actor wants to try and go dormant
			/* if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, GetTime(), bLowNetBandwidth))
			{
				// LOG_INFO(LogDev, "Actor is going dormant!");

				// Channel is marked to go dormant now once all properties have been replicated (but is not dormant yet)
				Channel->StartBecomingDormant();
			} */

			if (Addresses::ActorChannelClose && Offsets::IsNetRelevantFor)
			{
				static void (*ActorChannelClose)(UActorChannel*) = decltype(ActorChannelClose)(Addresses::ActorChannelClose);

				if (!Actor->IsAlwaysRelevant() && !Actor->UsesOwnerRelevancy() && !Actor->IsOnlyRelevantToOwner())
				{
					if (Connection && Connection->GetViewTarget())
					{
						auto Viewer = Connection->GetViewTarget();
						auto Loc = Viewer->GetActorLocation();

						if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
						{
							// LOG_INFO(LogReplication, "Actor is not relevant!");

							if (Channel)
								ActorChannelClose(Channel);

							continue;
						}
					}
				}
			}


			if (!Channel)
			{
				if (Actor->IsA(APlayerController::StaticClass()) && Actor != Connection->GetPlayerController()) // isnetrelevantfor should handle this iirc
					continue;

				if (bLevelInitializedForActor)
				{
					if (Engine_Version >= 422)
					{
						FString ActorStr = L"Actor";
						FName ActorName = UKismetStringLibrary::Conv_StringToName(ActorStr);

						int ChannelIndex = -1; // 4294967295
						Channel = (UActorChannel*)CreateChannelByName(Connection, &ActorName, EChannelCreateFlags::OpenedLocally, ChannelIndex);
					}
					else
					{
						Channel = (UActorChannel*)CreateChannel(Connection, 2, true, -1);
					}

					if (Channel)
					{
						if (Engine_Version >= 500)
							SetChannelActor(Channel, Actor);
						else
							SetChannelActor2(Channel, Actor, ESetChannelActorFlags::None);
					}
				}

				else if (Actor->GetNetUpdateFrequency() < 1.0f)
				{
					ActorInfo->NextUpdateTime = UGameplayStatics::GetTimeSeconds(GetWorld()) + 0.2f * FRand();
				}
			}

			if (Channel)
			{
				if (ReplicateActor(Channel))
				{
					if (ShouldUseNetworkObjectList())
					{
						// LOG_INFO(LogReplication, "Replicated Actor!");
						auto TimeSeconds = UGameplayStatics::GetTimeSeconds(World);
						const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
						const float MaxOptimalDelta = max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);
						const float DeltaBetweenReplications = (TimeSeconds - ActorInfo->LastNetReplicateTime);

						// Choose an optimal time, we choose 70% of the actual rate to allow frequency to go up if needed
						ActorInfo->OptimalNetUpdateDelta = std::clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta); // should we use fmath?
						ActorInfo->LastNetReplicateTime = TimeSeconds;
					}
				}
			}
		}
	}

	// shuffle the list of connections if not all connections were ticked
	/*
	if (NumClientsToTick < NetDriver->ClientConnections.Num())
	{
		int32 NumConnectionsToMove = NumClientsToTick;
		while (NumConnectionsToMove > 0)
		{
			// move all the ticked connections to the end of the list so that the other connections are considered first for the next frame
			UNetConnection* Connection = NetDriver->ClientConnections[0];
			NetDriver->ClientConnections.RemoveAt(0, 1);
			NetDriver->ClientConnections.Add(Connection);
			NumConnectionsToMove--;
		}
	}
	*/

	return Updated;
}