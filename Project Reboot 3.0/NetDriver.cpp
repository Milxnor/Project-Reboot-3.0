#include "NetDriver.h"

#include "reboot.h"
#include "Actor.h"
#include "NetConnection.h"
#include "GameplayStatics.h"

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

int32 ServerReplicateActors_PrepConnections(UNetDriver* NetDriver)
{
	auto& ClientConnections = NetDriver->GetClientConnections();

	int32 NumClientsToTick = ClientConnections.Num();

	bool bFoundReadyConnection = false;

	for (int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++)
	{
		UNetConnection* Connection = ClientConnections.at(ConnIdx);
		// check(Connection);
		// check(Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed);
		// checkSlow(Connection->GetUChildConnection() == NULL);

		AActor* OwningActor = Connection->GetOwningActor();
		if (OwningActor != NULL) // && /* Connection->State == USOCK_Open && */ (Connection->Driver->Time - Connection->LastReceiveTime < 1.5f))
		{
			// check(World == OwningActor->GetWorld());

			bFoundReadyConnection = true;

			// the view target is what the player controller is looking at OR the owning actor itself when using beacons
			Connection->GetViewTarget() = Connection->GetPlayerController() ? Connection->GetPlayerController()->GetViewTarget() : OwningActor;
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

enum class ENetDormancy : uint8_t
{
	DORM_Never = 0,
	DORM_Awake = 1,
	DORM_DormantAll = 2,
	DORM_DormantPartial = 3,
	DORM_Initial = 4,
	DORN_MAX = 5,
	ENetDormancy_MAX = 6
};

struct FNetworkObjectInfo
{
	AActor* Actor;
	/* TWeakObjectPtr<AActor> WeakActor;
	double NextUpdateTime;
	double LastNetReplicateTime;
	float OptimalNetUpdateDelta;
	float LastNetUpdateTime;
	uint32 bPendingNetUpdate : 1;
	uint32 bForceRelevantNextUpdate : 1;
	TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;
	TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections; */
};

static void ServerReplicateActors_BuildConsiderList(UNetDriver* NetDriver, std::vector<FNetworkObjectInfo*>& OutConsiderList)
{
	TArray<AActor*> Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass());

	/* auto& ActiveObjects = GetNetworkObjectList(NetDriver).ActiveNetworkObjects;

	for (int i = 0; i < ActiveObjects.Num(); i++)
	{
		auto ActorInfo = ActiveObjects.GetElements().GetData()[i].ElementData.Value.Get();
		auto Actor = ActorInfo->Actor; */

	for (int i = 0; i < Actors.Num(); i++)
	{
		auto Actor = Actors.at(i);

		if (!Actor)
			continue;

		// if (!Actor->bActorInitialized) continue;

		if (Actor->IsActorBeingDestroyed())
		{
			continue;
		}

		static auto RemoteRoleOffset = Actor->GetOffset("RemoteRole");

		if (Actor->Get<ENetRole>(RemoteRoleOffset) == ENetRole::ROLE_None)
		{
			continue;
		}

		static auto NetDormancyOffset = Actor->GetOffset("NetDormancy");

		if (Actor->Get<ENetDormancy>(NetDormancyOffset) == ENetDormancy::DORM_Initial && Actor->IsNetStartup())
		{
			continue;
		}

		static void (*CallPreReplication)(AActor*, UNetDriver*) = decltype(CallPreReplication)(Addresses::CallPreReplication);
		CallPreReplication(Actor, NetDriver);

		FNetworkObjectInfo* ActorInfo = new FNetworkObjectInfo;
		ActorInfo->Actor = Actor;
		OutConsiderList.push_back(ActorInfo);
	}

	Actors.Free();
}

using UChannel = UObject;
using UActorChannel = UObject;

static UActorChannel* FindChannel(AActor* Actor, UNetConnection* Connection)
{
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

		if (!Channel->IsA(ActorChannelClass)) // (Channel->ClassPrivate == ActorChannelClass)
			continue;

		static auto ActorOffset = Channel->GetOffset("Actor");
		auto ChannelActor = Channel->Get<AActor*>(ActorOffset);

		// LOG_INFO(LogReplication, "[{}] {}", i, ChannelActor->GetFullName());

		if (ChannelActor != Actor)
			continue;
		
		return (UActorChannel*)Channel;
	}

	return NULL;
}

int32 UNetDriver::ServerReplicateActors()
{
	int32 Updated = 0;

	++*(int*)(this + Offsets::ReplicationFrame);

	const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(this);

	if (NumClientsToTick == 0)
	{
		// No connections are ready this frame
		return 0;
	}

	// AFortWorldSettings* WorldSettings = GetFortWorldSettings(NetDriver->World);

	// bool bCPUSaturated = false;
	float ServerTickTime = 30.f; // Globals::MaxTickRate; // GEngine->GetMaxTickRate(DeltaSeconds);
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
	// ConsiderList.reserve(GetNetworkObjectList(NetDriver).ActiveNetworkObjects.Num());

	// std::cout << "ConsiderList.size(): " << GetNetworkObjectList(NetDriver).ActiveNetworkObjects.Num() << '\n';

	ServerReplicateActors_BuildConsiderList(this, ConsiderList);

	for (int32 i = 0; i < this->GetClientConnections().Num(); i++)
	{
		UNetConnection* Connection = this->GetClientConnections().at(i);

		if (!Connection)
			continue;

		if (i >= NumClientsToTick)
			continue;

		if (!Connection->GetViewTarget())
			continue;

		if (Connection->GetPlayerController())
		{
			static void (*SendClientAdjustment)(APlayerController*) = decltype(SendClientAdjustment)(Addresses::SendClientAdjustment);
			SendClientAdjustment(Connection->GetPlayerController());
		}

		for (auto& ActorInfo : ConsiderList)
		{
			if (!ActorInfo || !ActorInfo->Actor)
				continue;

			auto Actor = ActorInfo->Actor;

			auto Channel = FindChannel(Actor, Connection);

			/* std::vector<FNetViewer> ConnectionViewers;
			ConnectionViewers.push_back(ConstructNetViewer(Connection));

			if (!Actor->bAlwaysRelevant && !Actor->bNetUseOwnerRelevancy && !Actor->bOnlyRelevantToOwner)
			{
				if (Connection && Connection->ViewTarget)
				{
					auto Viewer = Connection->ViewTarget;
					auto Loc = Viewer->K2_GetActorLocation();

					if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
					{
						if (Channel)
							CloseChannel(Channel);

						continue;
					}
				}
			} */

			static UChannel* (*CreateChannel)(UNetConnection*, int, bool, int32_t) = decltype(CreateChannel)(Addresses::CreateChannel);
			static __int64 (*ReplicateActor)(UActorChannel*) = decltype(ReplicateActor)(Addresses::ReplicateActor);
			static __int64 (*SetChannelActor)(UActorChannel*, AActor*) = decltype(SetChannelActor)(Addresses::SetChannelActor);

			if (!Channel)
			{
				if (Actor->IsA(APlayerController::StaticClass()) && Actor != Connection->GetPlayerController()) // isnetreelvantfor should handle this iirc
					continue;

				Channel = (UActorChannel*)CreateChannel(Connection, 2, true, -1);

				if (Channel) {
					SetChannelActor(Channel, Actor);
					// Channel->Connection = Connection;
				}

			}

			if (Channel)
				ReplicateActor(Channel);
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