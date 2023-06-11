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
#include "ActorChannel.h"
#include "NetworkGuid.h"

struct FActorDestructionInfo
{
	TWeakObjectPtr<ULevel>		Level;
	TWeakObjectPtr<UObject>		ObjOuter;
	FVector			DestroyedPosition;
	FNetworkGUID	NetGUID;
	FString			PathName;
	FName			StreamingLevelName;
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
	TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;
};

struct FNetViewer
{
	UNetConnection* Connection;                                               // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
	AActor* InViewer;                                                 // 0x0008(0x0008) (ZeroConstructor, IsPlainOldData)
	AActor* ViewTarget;                                               // 0x0010(0x0008) (ZeroConstructor, IsPlainOldData)
	FVector                                     ViewLocation;                                             // 0x0018(0x000C) (IsPlainOldData)
	FVector                                     ViewDir;
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

struct FActorPriority
{
	int32						Priority;	// Update priority, higher = more important.

	FNetworkObjectInfo* ActorInfo;	// Actor info.
	UActorChannel* Channel;	// Actor channel.

	FActorDestructionInfo* DestructionInfo;	// Destroy an actor

	FActorPriority() :
		Priority(0), ActorInfo(NULL), Channel(NULL), DestructionInfo(NULL)
	{}

	FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const std::vector<FNetViewer>& Viewers, bool bLowBandwidth);
	FActorPriority(UNetConnection* InConnection, FActorDestructionInfo* DestructInfo, const std::vector<FNetViewer>& Viewers);
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

struct FNetGUIDCache
{
	bool SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr = nullptr) const
	{
		// 1.11
		bool (*SupportsObjectOriginal)(__int64, const UObject*, const TWeakObjectPtr<UObject>*) = decltype(SupportsObjectOriginal)(__int64(GetModuleHandleW(0)) + 0x1AF01E0);
		return SupportsObjectOriginal(__int64(this), Object, WeakObjectPtr);
	}
};

class UNetDriver : public UObject
{
public:
	// static inline int ReplicationDriverOffset = 0;
	static inline bool (*InitListenOriginal)(UNetDriver* NetDriver, FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
	static inline void (*SetWorldOriginal)(UNetDriver* NetDriver, UWorld* World);
	static inline void (*TickFlushOriginal)(UNetDriver* NetDriver);

	static void TickFlushHook(UNetDriver* NetDriver);

	int& GetMaxInternetClientRate()
	{
		static auto MaxInternetClientRateOffset = GetOffset("MaxInternetClientRate");
		return Get<int>(MaxInternetClientRateOffset);
	}

	int& GetMaxClientRate()
	{
		static auto MaxClientRateOffset = GetOffset("MaxClientRate");
		return Get<int>(MaxClientRateOffset);
	}

	FNetGUIDCache* GetGuidCache()
	{
		static auto GuidCacheOffset = GetOffset("WorldPackage") + 8; // checked for 1.11
		return GetPtr<FNetGUIDCache>(GuidCacheOffset);
	}

	UWorld*& GetNetDriverWorld() const
	{
		static auto WorldOffset = GetOffset("World");
		return Get<UWorld*>(WorldOffset);
	}

	UObject*& GetWorldPackage() const
	{
		static auto WorldPackageOffset = GetOffset("WorldPackage");
		return Get<UObject*>(WorldPackageOffset);
	}

	TArray<UNetConnection*>& GetClientConnections()
	{
		static auto ClientConnectionsOffset = GetOffset("ClientConnections");
		return Get<TArray<UNetConnection*>>(ClientConnectionsOffset);
	}

	float& GetTime()
	{
		static auto TimeOffset = GetOffset("Time");
		return Get<float>(TimeOffset);
	}

	float& GetRelevantTimeout()
	{
		static auto RelevantTimeoutOffset = GetOffset("RelevantTimeout");
		return Get<float>(RelevantTimeoutOffset);
	}

	float& GetSpawnPrioritySeconds()
	{
		static auto SpawnPrioritySecondsOffset = GetOffset("SpawnPrioritySeconds");
		return Get<float>(SpawnPrioritySecondsOffset);
	}

	int32& GetNetTag()
	{
		static auto NetTagOffset = 0x1DC + 4;
		return Get<int32>(NetTagOffset);
	}

	bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const;
	void RemoveNetworkActor(AActor* Actor);
	bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) { return InitListenOriginal(this, InNotify, ListenURL, bReuseAddressAndPort, Error); }
	void SetWorld(UWorld* World) { return SetWorldOriginal(this, World); }
	int32 ServerReplicateActors();
	int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
	void ServerReplicateActors_BuildConsiderList(std::vector<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime);
	int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const std::vector<FNetViewer>& ConnectionViewers, const std::vector<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);
	FNetworkObjectList& GetNetworkObjectList();
};