#pragma once

#include "Object.h"
#include "UnrealString.h"

#include "World.h"
#include "NetConnection.h"
#include "Array.h"

#include "WeakObjectPtrTemplates.h"
#include "Map.h"
#include "SharedPointer.h"
#include "Level.h"

struct FActorDestructionInfo
{
	TWeakObjectPtr<ULevel>		Level;
	TWeakObjectPtr<UObject>		ObjOuter;
	FVector			            DestroyedPosition;
	int32       	            NetGUID;
	FString			            PathName;
	FName			            StreamingLevelName;
};
struct FNetworkObjectInfo
{
	/** Pointer to the replicated actor. */
	AActor* Actor;

	/** WeakPtr to actor. This is cached here to prevent constantly constructing one when needed for (things like) keys in TMaps/TSets */
	TWeakObjectPtr<AActor> WeakActor;

	/** Next time to consider replicating the actor. Based on FPlatformTime::Seconds(). */
	double NextUpdateTime;

	/** Last absolute time in seconds since actor actually sent something during replication */
	double LastNetReplicateTime;

	/** Optimal delta between replication updates based on how frequently actor properties are actually changing */
	float OptimalNetUpdateDelta;

	/** Last time this actor was updated for replication via NextUpdateTime
	* @warning: internal net driver time, not related to WorldSettings.TimeSeconds */
	float LastNetUpdateTime;

	/** Is this object still pending a full net update due to clients that weren't able to replicate the actor at the time of LastNetUpdateTime */
	uint32 bPendingNetUpdate : 1;

	/** Force this object to be considered relevant for at least one update */
	uint32 bForceRelevantNextUpdate : 1;

	/** List of connections that this actor is dormant on */
	TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;

	/** A list of connections that this actor has recently been dormant on, but the actor doesn't have a channel open yet.
	*  These need to be differentiated from actors that the client doesn't know about, but there's no explicit list for just those actors.
	*  (this list will be very transient, with connections being moved off the DormantConnections list, onto this list, and then off once the actor has a channel again)
	*/
	TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;
};

class FNetworkObjectList
{
public:
	typedef TSet<TSharedPtr<FNetworkObjectInfo>> FNetworkObjectSet;

	FNetworkObjectSet AllNetworkObjects;
	FNetworkObjectSet ActiveNetworkObjects;
	FNetworkObjectSet ObjectsDormantOnAllConnections;

	TMap<TWeakObjectPtr<UNetConnection>, int32> NumDormantObjectsPerConnection;

	void Remove(AActor* const Actor);
};

class UWorld;

struct FURL // idk where this actually goes
{
	FString                                     Protocol;                                                 // 0x0000(0x0010) (ZeroConstructor)
	FString                                     Host;                                                     // 0x0010(0x0010) (ZeroConstructor)
	int                                         Port;                                                     // 0x0020(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                         Valid;                                                    // 0x0024(0x0004) (ZeroConstructor, IsPlainOldData)
	FString                                     Map;                                                      // 0x0028(0x0010) (ZeroConstructor)
	FString                                     RedirectUrl;                                              // 0x0038(0x0010) (ZeroConstructor)
	TArray<FString>                             Op;                                                       // 0x0048(0x0010) (ZeroConstructor)
	FString                                     Portal;                                                   // 0x0058(0x0010) (ZeroConstructor)
};

class UNetDriver : public UObject
{
public:
	// static inline int ReplicationDriverOffset = 0;
	static inline bool (*InitListenOriginal)(UNetDriver* NetDriver, FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
	static inline void (*SetWorldOriginal)(UNetDriver* NetDriver, UWorld* World);
	static inline void (*TickFlushOriginal)(UNetDriver* NetDriver);

	static void TickFlushHook(UNetDriver* NetDriver);

	TArray<UNetConnection*>& GetClientConnections()
	{
		static auto ClientConnectionsOffset = GetOffset("ClientConnections");
		return Get<TArray<UNetConnection*>>(ClientConnectionsOffset);
	}

	void RemoveNetworkActor(AActor* Actor);
	bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) { return InitListenOriginal(this, InNotify, ListenURL, bReuseAddressAndPort, Error); }
	void SetWorld(UWorld* World) { return SetWorldOriginal(this, World); }
	int32 ServerReplicateActors();
	void ServerReplicateActors_BuildConsiderList(std::vector<FNetworkObjectInfo*>& OutConsiderList);
	FNetworkObjectList& GetNetworkObjectList();
};